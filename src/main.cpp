#include "DataProtocol_PC2.hpp"
#include <TEA5767Radio.h>
#include <Wire.h>

TEA5767Radio radio = TEA5767Radio();

using namespace PC_2;

void interpret(long receiveMsg);

void setup() {
  Transceiver::init();
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  radio.setFrequency(107.7);
  // radio.setFrequency(107.0);
  // Serial.println("<< PC 2 >>");
}

int cnt = 0;

void loop() {
  // long receiveMsg = 0;
  // if (receiveFrameDAC(&receiveMsg, 8, 500)) {
  //   Serial.print("Received Msg: ");
  //   for (int i = 7; i >= 0; --i) {
  //     Serial.print((receiveMsg >> i) & 1);
  //   }
  //   Serial.print("   ");
  //   Serial.println((char)receiveMsg);
  // }
  long receiveMsg = startReceive();
  delay(300);
  interpret(receiveMsg);
  delay(300);
}

void interpret(long receiveMsg) {
  int command = (receiveMsg >> 5) & 7;
  if (command == 1) {
    Serial.write('s');
    long sendMsg = 0;
    int byteCount = 0;
    // Serial.println("Waiting");
    while (byteCount < 2) {
      long tmp = 0;
      if(receiveFrameDAC(&tmp,8,1000)) sendACK();
      if (Serial.available()) {
        int tmp = Serial.read();
        // Serial.write(tmp);
        sendMsg += tmp;
        ++byteCount;
        if (byteCount < 2)
          sendMsg <<= 8;
      }
    }
    startSend(sendMsg);
  } else {
    if (command == 2)
      Serial.write('l');
    else if (command == 3)
      Serial.write('c');
    else if (command == 4)
      Serial.write('r');
    else return;

    for (int dot = 0; dot < 16; ++dot) {
      Serial.write('q');
      for (int i = 0; i < 3; ++i) {
        long sendMsg = 0;
        int byteCount = 0;

        while (byteCount < 2) {
          long tmp = 0;
          if(receiveFrameDAC(&tmp,8,500)) sendACK();
          if (Serial.available()) {
            sendMsg += Serial.read();
            ++byteCount;
            if (byteCount < 2)
              sendMsg <<= 8;
          }
        }
        startSend(sendMsg);
        // Serial.println("COMPLETE");
      }
    }
  }
}