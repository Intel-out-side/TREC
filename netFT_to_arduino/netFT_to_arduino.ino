/* This program tries to get a sensor data from Net F/T board
   through CAN-BUS communication.
   @author : Soichiro Uchino
   @version : 2019-11-20
*/

/* ToDo
   1. need some calibration
   2. need to accept the data from two boards
   scaling factor can be found on the page B-83
*/

#include <SPI.h>
#include "mcp_can.h"
#include "util.h"

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int BORAD1_BASE = 0x1B0; // base adress of board 1
//const int BOARD2_BASE = 1111111; // base address of board 2

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

unsigned char len = 0;
unsigned char buf[8];
unsigned int canId;

void setup()
{
  Serial.begin(115200);

  while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println(" Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");
  
  CAN.sendMsgBuf(BORAD1_BASE, 0, 1, 0x04); // zeros force and torque at the current value
}

/* To communicate with Net F/T board to get a sensor data from mini45
   1. send a message request 0x01 from CAN ID:432 (1B0 in HEX)
   2. read a message buffer returning back in little endian format

   In long data format, data comes back in 8bytes.
   First 4bytes contain force data, the other 4bytes
   contain torque data
   e.g.) 11 22 33 44 11 22 33 44
*/

void loop()
{
  unsigned char messageRequest[1] = {1};
  unsigned int canIdX, canIdY, canIdZ, canIdStatus;
  long fx, fy, fz, tx, ty, tz;

  CAN.sendMsgBuf(BORAD1_BASE, 0, 1, messageRequest); // send a message request in a long format

  storeFT(fx, fy, fz, tx, ty, tz);
  storeFT(fx, fy, fz, tx, ty, tz);
  storeFT(fx, fy, fz, tx, ty, tz);
  storeFT(fx, fy, fz, tx, ty, tz);

  Serial.println("----------------");
}

/* assign the force torque vaues depending on the
   can ID. Make sure that the base address is 432 (HEX:1B0)
   base address +1 : x axis
   base address +2 : y axis
   base address +3 : z axis
   @param fx, fy, fz, tx, ty, tz : the force and torque variables to store the data
*/
void storeFT(long& fx, long& fy, long& fz, long& tx, long& ty, long& tz) {
  while(CAN_MSGAVAIL != CAN.checkReceive());
  CAN.readMsgBuf(&len, buf);
  canId = CAN.getCanId();

  if (canId == BORAD1_BASE + 1) {
    fx = convertToForce(buf);
    tx = convertToTorque(buf);
    Serial.print("from: "); Serial.println(canId);
    Serial.print("fx: "); Serial.println(fx);
    Serial.print("tx: "); Serial.println(tx);
  }
  else if (canId == BORAD1_BASE + 2) {
    fy = convertToForce(buf);
    ty = convertToTorque(buf);
    Serial.print("from: "); Serial.println(canId);
    Serial.print("fy: "); Serial.println(fy);
    Serial.print("ty: "); Serial.println(ty);
  }
  else if (canId == BORAD1_BASE + 3) {
    fz = convertToForce(buf);
    tz = convertToTorque(buf);
    Serial.print("from: "); Serial.println(canId);
    Serial.print("fz: "); Serial.println(fz);
    Serial.print("tz: "); Serial.println(tz);
  }
  else {
    Serial.println("StatusMessage received");
  }
  
}

/*
  this function converts the raw string data to
  long integer force value.
  4th ~ 8th byte of parameter represent the force value

  @param buf[8] : buffer data that contains raw data coming back from NetF/T
  @return : long integer value of force
*/
long convertToForce(unsigned char buf[8]) {
  String resultStr = "";
  for (int i = 7; i >= 4; i--) {
    if (buf[i] <= 15) resultStr += "0" + String(buf[i], HEX);
    else resultStr += String(buf[i], HEX);
  }
  resultStr = hexToBin(resultStr);
  return binToDec(resultStr);
}

/*
  this function converts the raw string data to
  integer torque value.

  @param buf[8] : buffer data that contains raw data coming back from NetF/T
  @return : integer value of torque
*/
long convertToTorque(unsigned char buf[8]) {
  String resultStr = "";
  for (int i = 3; i >= 0; i--) {
    if (buf[i] <= 15) resultStr += "0" + String(buf[i], HEX);
    else resultStr += String(buf[i], HEX);
  }
  resultStr = hexToBin(resultStr);
  return binToDec(resultStr);
}

//------------------------test functions below-----------------------
/*  test method for convertToForce
    @postcondition : result should match -35991793 for given data
*/
void testConvertToForce() {
  unsigned char buf[8] = {0x00, 0x00, 0x00, 0x00, 0x0f, 0xcf, 0xda, 0xfd};

  long result = convertToForce(buf);

  if (result == -35991793) Serial.println("test passed");
}

/* test method for convertToTorque
   @postcondition : result shold match -35991793 for given data
*/
void testConvertToTorque() {
  unsigned char buf[8] = {0x0f, 0xcf, 0xda, 0xfd, 0x00, 0x00, 0x00, 0x00};

  long result = convertToTorque(buf);

  if (result == -35991793) Serial.println("test passed");
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
