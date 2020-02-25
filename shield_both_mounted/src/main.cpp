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
#include "profile2.h"
#include "EasyCAT.h"

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int BORAD1_BASE = 0x1B0; // base adress of board 1
//const int BOARD2_BASE = 1111111; // base address of board 2
const double FORCE_FACTOR = 35402.0;
const double TORQUE_FACTOR = 611.0;
unsigned long time;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

unsigned char len = 0;
unsigned char buf[8];
unsigned int canId;

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

double convertToTorque(unsigned char buf[8]);
double convertToForce(unsigned char buf[8]);
void storeFT(double& fx, double& fy, double& fz, double& tx, double& ty, double& tz);
void Application(double fx, double fy, double fz, double tx, double ty, double tz);
void mainFunction();

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

  unsigned char reset[1] = {0x04};
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
  //CAN.sendMsgBuf(BORAD1_BASE, 0, 1, messageRequest); // send a message request in a long format
  //sending/receiving data to/from CAN-bus shield is blocking process (probably)

  //storeFT(fx, fy, fz, tx, ty, tz);
  //storeFT(fx, fy, fz, tx, ty, tz);
  //storeFT(fx, fy, fz, tx, ty, tz);
  //storeFT(fx, fy, fz, tx, ty, tz);

  EASYCAT.MainTask();
  Application(fx, fy, fz, tx, ty, tz);

  //Serial.println(time++); // check if the function executed til the end
}

/* assign the force torque vaues depending on the
   can ID. Make sure that the base address is 432 (HEX:1B0)
   base address +1 : x axis
   base address +2 : y axis
   base address +3 : z axis
   @param fx, fy, fz, tx, ty, tz : the force and torque variables to store the data
*/
void storeFT(double& fx, double& fy, double& fz, double& tx, double& ty, double& tz) {
  while(CAN_MSGAVAIL != CAN.checkReceive());
  CAN.readMsgBuf(&len, buf);
  canId = CAN.getCanId();

  if (canId == BORAD1_BASE + 1) {
    fx = convertToForce(buf);
    tx = convertToTorque(buf);
  }
  else if (canId == BORAD1_BASE + 2) {
    fy = convertToForce(buf);
    ty = convertToTorque(buf);
  }
  else if (canId == BORAD1_BASE + 3) {
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

void Application(double fx, double fy, double fz, double tx, double ty, double tz) {
  uint8_t a = 100, b = 200, c = 254, d = 255, e = 255, f = 255;
  uint8_t ref_fx = EASYCAT.BufferOut.Cust.fx_m2s;
  uint8_t ref_fy = EASYCAT.BufferOut.Cust.fy_m2s;
  uint8_t ref_fz = EASYCAT.BufferOut.Cust.fz_m2s;
  uint8_t ref_tx = EASYCAT.BufferOut.Cust.tx_m2s;
  uint8_t ref_ty = EASYCAT.BufferOut.Cust.ty_m2s;
  uint8_t ref_tz = EASYCAT.BufferOut.Cust.tz_m2s;

  Serial.print(ref_fx); Serial.print(", ");
  Serial.print(ref_fy); Serial.print(", ");
  Serial.print(ref_fz); Serial.print(", ");
  Serial.print(ref_tx); Serial.print(", ");
  Serial.print(ref_ty); Serial.print(", ");
  Serial.println(ref_tz);


  EASYCAT.BufferIn.Cust.fx_s2m = (uint8_t) a;
  EASYCAT.BufferIn.Cust.fy_s2m = (uint8_t) b;
  EASYCAT.BufferIn.Cust.fz_s2m = (uint8_t) c;
  EASYCAT.BufferIn.Cust.tx_s2m = (uint8_t) d;
  EASYCAT.BufferIn.Cust.ty_s2m = (uint8_t) e;
  EASYCAT.BufferIn.Cust.tz_s2m = (uint8_t) f;

  //多分これでスレーブ側のコンフィグは完了したはず
  //SOEM側がどうconfigされるべきなのかは全くわからない
  //SOEMでslaveinfoを見ると変数も見れるのでひょっとするともうconfiguすら必要ない説？
}
