#include "EasyCAT.h"
#include <SPI.h>
#include <Servo.h>

// EasyCATクラスのインスタンス
EasyCAT easyCAT;

unsigned long Millis = 0;
unsigned long PreviousMillis = 0;

// 実験用のサーボ
Servo servo;

void setup()
{
  // 実験用サーボはポート3に接続
  servo.attach(3);
  servo.write(90);

  Serial.begin(9600);
  Serial.print ("\nEasyCAT - Generic EtherCAT slave\n");

  // EasyCATの初期化
  if (easyCAT.Init() == true)
  {
    Serial.print ("initialized");
  }
  else
  {
    // 初期化に失敗した場合
    Serial.print ("initialization failed");

    pinMode(13, OUTPUT);
    while(1)
    {
      digitalWrite (13, LOW);
      delay(500);
      digitalWrite (13, HIGH);
      delay(500);
    }
  }
  PreviousMillis = millis();
}

void loop()
{
  // EasyCATのメインタスク
  easyCAT.MainTask();
  // ユーザーアプリケーション
  Application();
}

void Application ()
{
  uint16_t masterToSlave = EASYCAT.BufferOut.Cust.output1;
  EASYCAT.BufferIn.Cust.input1 = (uint16_t) 114514;
  // 10msecごとに実行
  Millis = millis();
  if (Millis - PreviousMillis >= 10)
  {
    PreviousMillis = Millis;

    // OUTバッファの0バイトめの値をサーボに出力
    servo.write(easyCAT.BufferOut.Byte[0]);

    // A0ポートのアナログ入力値をINバッファの0～1バイトめに格納
    int vol = analogRead(0);
    byte vol_h = (byte)(vol >> 8);
    byte vol_l = (byte)(vol & 0xFF);
    easyCAT.BufferIn.Byte[0] = vol_h;
    easyCAT.BufferIn.Byte[1] = vol_l;
  }
}
