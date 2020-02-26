/* This program tries to get a sensor data from Net F/T board
   through CAN-BUS communication.
   @author : Soichiro Uchino
   @version : 2019-11-20
*/

/* ToDo
   1. connect to the ethernet to transfer the data...?
      (probably need more specs about it)
   2. need to accept the data from two boards
   3. verify the calibration of sensors.
*/
#include <Arduino.h>
#include <SPI.h>
#include "mcp_can.h"
#include "util.h"
#include <MsTimer2.h>
#define CUSTOM
#include "profile.h"
#include "EasyCAT.h"

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int BORAD1_BASE = 0x1B0; // base adress of board 1
//const int BOARD2_BASE = 1111111; // base address of board 2

const double FORCE_FACTOR = 35402.0; // devide raw force data by this factor
const double TORQUE_FACTOR = 611.0; // devide raw torque data by this factor
unsigned long time;

MCP_CAN CAN(SPI_CS_PIN); // Set CS pin

unsigned char len = 0;  // data length coming through CAN-bus communication
unsigned char buf[8]; // data buffer for CAN-bus
unsigned int canId; // canId (== base address of each node)

UWORD ContaUp;                      // used for sawthoot test generation
UWORD ContaDown;                    //

unsigned long Millis = 0;
unsigned long PreviousMillis = 0;
unsigned long PreviousSaw = 0;
unsigned long PreviousCycle = 0;

EasyCAT EASYCAT(8); //assign D8 pin since the CAN-bus shield uses D9 pin

const int Ana0 = A0;                // analog input 0
const int Ana1 = A1;                // analog input 1

const int BitOut0 = A2;             // digital output bit 0
const int BitOut1 = A3;             // digital output bit 1
const int BitOut2 = A4;             // digital output bit 2
const int BitOut3 = A5;             // digital output bit 3

const int BitIn0 = 3;               // digital input  bit 0
const int BitIn1 = 5;               // digital input  bit 1
const int BitIn2 = 6;               // digital input  bit 2
const int BitIn3 = 7;               // digital input  bit 3

//---------------prototype declaration--------------------------
// DELETE ALL PROTOTYPE DECLARATION IF YOU USE ARDUINO IDE !!!!!!!

double convertToTorque(unsigned char buf[8]);  //covert buffer torque data(HEX) to DEC

double convertToForce(unsigned char buf[8]);  //convert buffer force data(HEX) to DEC

void storeFT(double& fx, double& fy, double& fz, double& tx, double& ty, double& tz);
// store force/torque to each variable, reading from buffer

void sendToMaster(double fx, double fy, double fz, double tx, double ty, double tz);
// send force and torque data to the master node

void mainFunction();
//-------------------------------------------------------------


void setup() {
  Serial.begin(9600);

  //-----------set up the CAN-bus communication ------
  while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println(" Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");

  unsigned char reset[1] = {0x04}; // 0x04 :: Message to set the current state as zero-loaded
  CAN.sendMsgBuf(BORAD1_BASE, 0, 1, reset); // zeros force and torque at the current value
  //-----------CAN-bus setup done --------------------

  //-----------EasyCAT setup--------------------------
  Serial.print("EasyCAT - Generic EtherCAT slave\n");
  pinMode(BitOut0, OUTPUT);
  pinMode(BitOut1, OUTPUT);
  pinMode(BitOut2, OUTPUT);
  pinMode(BitOut3, OUTPUT);

  pinMode(BitIn0, INPUT_PULLUP);
  pinMode(BitIn1, INPUT_PULLUP);
  pinMode(BitIn2, INPUT_PULLUP);
  pinMode(BitIn3, INPUT_PULLUP);

  ContaUp.Word = 0x0000;
  ContaDown.Word = 0x0000;
  while (EASYCAT.Init() != true) Serial.println("Initializing..");

  if (EASYCAT.Init() == true) {
    Serial.println("EasyCAT initialization completed");
  }
  else {
    Serial.println("EasyCAT initialization failed");
  }

  MsTimer2::set(20, mainFunction);
  MsTimer2::start();
}

void loop() {
  //leave the loop function blank
}

/*
  the main function to execute by a timer function
*/
void mainFunction() {
  unsigned char messageRequest[1] = {1};
  double fx, fy, fz, tx, ty, tz;
  CAN.sendMsgBuf(BORAD1_BASE, 0, 1, messageRequest); // send a message request in a long format
  //sending/receiving data to/from CAN-bus shield is blocking process (probably)

  storeFT(fx, fy, fz, tx, ty, tz);
  storeFT(fx, fy, fz, tx, ty, tz);
  storeFT(fx, fy, fz, tx, ty, tz);
  storeFT(fx, fy, fz, tx, ty, tz);
  // Net F/T board sends back the data 4 times
  // x-axis data, y-axis data, z-axis data, status message
  // These data come in random order, therefore arduino has to read buffer for 4 times

  EASYCAT.MainTask();
  sendToMaster(fx, fy, fz, tx, ty, tz);

}

/* assign the force torque vaues depending on the
   can ID. Make sure that the base address is 432 (HEX:0x1B0)
   base address +1 : x axis (fx, tx)
   base address +2 : y axis (ty, ty)
   base address +3 : z axis (fz, tz)
   @param fx, fy, fz, tx, ty, tz : the force and torque variables to store the data
*/
void storeFT(double& fx, double& fy, double& fz, double& tx, double& ty, double& tz) {
  while(CAN_MSGAVAIL != CAN.checkReceive()); //check if the returning value has arrived
  CAN.readMsgBuf(&len, buf); //read buffer value (|base address|force(32bits)|torque(32bits)|)
  canId = CAN.getCanId(); //get base address (i.e. CAN ID)

  if (canId == BORAD1_BASE + 1) { // if canId is base_address+1 -> returning values are fx, tx
    fx = convertToForce(buf);
    tx = convertToTorque(buf);
  }
  else if (canId == BORAD1_BASE + 2) { // if canId is base_address+2 -> returning values are fy, ty
    fy = convertToForce(buf);
    ty = convertToTorque(buf);
  }
  else if (canId == BORAD1_BASE + 3) { // if canId is base_address+3 -> returning values are fz, tz
    fz = convertToForce(buf);
    tz = convertToTorque(buf);
  }
}

/*
  this function converts the raw string data to
  double floating number force value.
  4th ~ 8th byte of parameter represent the force value

  @param buf[8] : buffer data that contains raw data coming back from NetF/T
  @return : double value of force[N]
*/
double convertToForce(unsigned char buf[8]) {
  String resultStr = "";
  double force;
  for (int i = 3; i >= 0; i--) {
    if (buf[i] <= 15) resultStr += "0" + String(buf[i], HEX);
    else resultStr += String(buf[i], HEX);
  }
  resultStr = hexToBin(resultStr);
  force = binToDec(resultStr)/FORCE_FACTOR;
  return force;
}

/*
  this function converts the raw string data to
  double torque value.

  @param buf[8] : buffer data that contains raw data coming back from NetF/T
  @return : double value of torque[Nm]
*/
double convertToTorque(unsigned char buf[8]) {
  String resultStr = "";
  double torque;
  for (int i = 7; i >= 4; i--) {
    if (buf[i] <= 15) resultStr += "0" + String(buf[i], HEX);
    else resultStr += String(buf[i], HEX);
  }
  resultStr = hexToBin(resultStr);
  torque = binToDec(resultStr)/TORQUE_FACTOR;
  return torque;
}

/*
This function sends force/torque data from Arduino to EtherCAT Master.
@param fx : force about x-axis
@param fy : force about y-axis
@param fz : force about z-axis
@param tx : torque about x-axis
@param fy : torque about y-axis
@param fz : torque about z-axis
*/
void sendToMaster(double fx, double fy, double fz, double tx, double ty, double tz) {

  // EASYCAT.BufferOut.Cust struct contains the data FROM MASTER TO SLAVE
  // read each values and store into fx~tz
  // e.g.) fx_m2s represents "fx master to slave"
  int8_t ref_fx = EASYCAT.BufferOut.Cust.fx_m2s; // reference value of fx, sent from master to slave
  int8_t ref_fy = EASYCAT.BufferOut.Cust.fy_m2s; // reference value of fy
  int8_t ref_fz = EASYCAT.BufferOut.Cust.fz_m2s; // reference value of fz
  int8_t ref_tx = EASYCAT.BufferOut.Cust.tx_m2s; // reference value
  int8_t ref_ty = EASYCAT.BufferOut.Cust.ty_m2s;
  int8_t ref_tz = EASYCAT.BufferOut.Cust.tz_m2s;


  // EASYCAT.BufferIn.Cust struct contains the data FROM SLAVE TO MASTER
  // store each force/torque values measured by sensor into struct member variables
  // e.g.) fx_s2m represents "fx slave to master". This will be read by EasyCAT in every iteration
  fx = fx > 255 ? 255 : fx;
  fy = fy > 255 ? 255 : fy;
  fz = fz > 255 ? 255 : fz;
  tx = tx > 255 ? 255 : tx;
  ty = ty > 255 ? 255 : ty;
  tz = tz > 255 ? 255 : tz;
  EASYCAT.BufferIn.Cust.fx_s2m = (int8_t) fx;
  EASYCAT.BufferIn.Cust.fy_s2m = (int8_t) fy;
  EASYCAT.BufferIn.Cust.fz_s2m = (int8_t) fz;
  EASYCAT.BufferIn.Cust.tx_s2m = (int8_t) tx;
  EASYCAT.BufferIn.Cust.ty_s2m = (int8_t) ty;
  EASYCAT.BufferIn.Cust.tz_s2m = (int8_t) tz;
}
