/* This program tries to get a sensor data from Net F/T board
   through CAN-BUS communication.
   @author : Soichiro Uchino
   @version : 2019-11-20
*/

#include <SPI.h>
#include "mcp_can.h"


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

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
  unsigned char lenX = 0, lenY = 0, lenZ = 0, lenStatus = 0;
  unsigned char bufX[8], bufY[8], bufZ[8], bufStatus[8];
  unsigned int canIdX, canIdY, canIdZ, canIdStatus;

  CAN.sendMsgBuf(0x1B0, 0, 1, messageRequest); // send a message request in a long format
  //while(CAN_MSGAVAIL != CAN.checkReceive()) ; // wait til the data comes back

  while(CAN_MSGAVAIL != CAN.checkReceive());
  CAN.readMsgBuf(&lenX, bufX);
  canIdX = CAN.getCanId();

  while(CAN_MSGAVAIL != CAN.checkReceive());
  CAN.readMsgBuf(&lenY, bufY);
  canIdY = CAN.getCanId();
  
  while(CAN_MSGAVAIL != CAN.checkReceive());
  CAN.readMsgBuf(&lenZ, bufZ);
  canIdZ = CAN.getCanId();

  while(CAN_MSGAVAIL != CAN.checkReceive());
  CAN.readMsgBuf(&lenStatus, bufStatus);
  canIdStatus = CAN.getCanId();

  Serial.println("--------------------------");

  Serial.print("Data form ID: ");
  Serial.println(canIdX, HEX);

  Serial.print("X datas: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(bufX[i], HEX);
  }
  Serial.println();

  /*-------------------------------*/

  Serial.print("Data form ID: ");
  Serial.println(canIdY, HEX);

  Serial.print("Y datas: ");
  for (int i = 0; i < 8; i++ ) {
    Serial.print(bufY[i], HEX);
  }
  Serial.println();

  /*-------------------------------*/

  Serial.print("Data form ID: ");
  Serial.println(canIdZ, HEX);

  Serial.print("Z datas: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(bufZ[i], HEX);
  }
  Serial.println();

  /*------------------------------*/

  Serial.print("Data form ID: ");
  Serial.println(canIdStatus, HEX);

  Serial.print("Status datas: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(bufStatus[i], HEX);
  }
  Serial.println();
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
