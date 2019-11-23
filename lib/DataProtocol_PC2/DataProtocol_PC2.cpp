#include "DataProtocol_PC2.hpp"

int PC_2::sendFrameCount = 0;
int PC_2::receiveFrameCount = 0;

void PC_2::startSend(long data) {
  ACKStatus status;
  Serial.print("Start Sending Session [");
  Serial.print(data);
  Serial.println("]");
  do {
    // Serial.println("Making Frame...");
    int bits = PC_2::makeFrame(1, sendFrameCount, data);
    // for (int i = 15; i >= 0; --i) {
    //   Serial.print((bits >> i) & 1);
    // }
    Serial.println("Sending...");
    sendFrameDAC(bits, 16);
    // Serial.println("Sent! Waiting for ACK");
    status = PC_2::waitingForACK();
  } while (status != ACKStatus::R);
  PC_2::sendFrameCount = (sendFrameCount + 1) % 2;
  Serial.println("Sending Session Completed");
}

long PC_2::startReceive() {
  long receive = 0;
  bool isReceived = false;
  // Serial.println("Waiting for Frame...");
  while (!isReceived) {
    isReceived = receiveFrameDAC(&receive, 8, 500);
    bool crc = checkCRC(receive, 8);
    if (isReceived && ((receive >> 4) & 1) == receiveFrameCount &&
        ((receive >> 5) & 7) != 0) {
      if (crc) {
        receiveFrameCount = (receiveFrameCount + 1) % 2;
        // Serial.println("Data Received, Sending ACK");
        while (isReceived) {
          PC_2::sendACK();
          long tmp = 0;
          isReceived = receiveFrameDAC(&tmp, 8, 900);
          if (tmp != 0 && tmp == receive)
            isReceived = true;
        }
        // Serial.println("ACK Send + Received");
        return receive;
      } else {
        receive = 0;
        // Serial.println("Incorrect Data, Frame Discarded");
        isReceived = false;
      }
    } else
      isReceived = false;
  }
  return receive;
}

void PC_2::sendACK() {
  long bits = PC_2::makeFrame(0, receiveFrameCount, 0);
  sendFrameDAC(bits, 16);
}

ACKStatus PC_2::waitingForACK() {
  long receive = 0;
  bool isReceived = receiveFrameDAC(&receive, 8, 500);
  if (isReceived && ((receive >> 5) & 111) == 0 &&
      ((receive >> 4) & 1) == (PC_2::sendFrameCount + 1) % 2) {
    if (checkCRC(receive, 8)) {
      Serial.println("ACK Received");
      return ACKStatus::R;
    } else {
      Serial.println("ACK Received, Incorrect Data");
      return ACKStatus::NR;
    }
  }
  Serial.println("ACK Not Received");
  return ACKStatus::NR;
}

long PC_2::makeFrame(int command, int frameNo, long data) {
  /*
  [1] command + [1] frameNo + [9] data + [4] CRC
  */
  long bits = 0;
  bits = command;
  bits <<= 1;
  bits += frameNo;
  bits <<= 9;
  bits += data;
  bits <<= 4;
  bits += generateCRC(bits, 16);
  return bits;
}