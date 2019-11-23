#include "DataProtocol_PC2.hpp"
#include <TEA5767Radio.h>
#include <Wire.h>

TEA5767Radio radio = TEA5767Radio();

using namespace PC_2;

void interpret(long receiveMsg);

void setup() {
  Transceiver::init();
  // Serial.println("<< PC 2 >>");
  radio.setFrequency(99.9);
  pinMode(13, OUTPUT);
}

int cnt = 0;

void loop() {
  digitalWrite(13, LOW);

  // long receive = 0;
  // while (!receiveFrameDAC(&receive, 8, 500)) {
  // }
  // for (int i = 6; i >= 0; i -= 2) {
  //   Serial.print((receive >> (i + 1)) & 1);
  //   Serial.print((receive >> i) & 1);
  //   Serial.print(" ");
  // }
  // Serial.println((char)receive);
  // delay(500);
  // sendFrameDAC(('A' + cnt++%26), 8);
  // Serial.println("<< START RECEIVE >>");
  long receiveMsg = startReceive();
  // Serial.print("Received Msg: ");
  // for (int i = 7; i >= 0; --i) {
  //   Serial.print((receiveMsg >> i) & 1);
  // }
  // Serial.println();
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
      if (Serial.available()) {
        int tmp = Serial.read();
        Serial.write(tmp);
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
      Serial.write('r');
    else if (command == 4)
      Serial.write('c');
    for (int dot = 0; dot < 16; ++dot) {
      Serial.write('q');
      delay(10);
      for (int i = 0; i < 3; ++i) {
        long sendMsg = 0;
        int byteCount = 0;
        // Serial.println("Waiting");
        while (byteCount < 2) {
          if (Serial.available()) {
            sendMsg += Serial.read();
            ++byteCount;
            if (byteCount < 2)
              sendMsg <<= 8;
          }
        }
        startSend(sendMsg);
      }
    }
  }
}