#include "DataProtocol_PC2.hpp"

int PC_2::sendFrameCount = 0;
int PC_2::receiveFrameCount = 0;

void PC_2::startSend(long data) {
  ACKStatus status;
  do {
    long bits = PC_2::makeFrame(1, sendFrameCount, data);
    sendFrameDAC(bits, 16);
    status = PC_2::waitingForACK();
  } while (status != ACKStatus::R);

  PC_2::sendFrameCount = (sendFrameCount + 1) % 2;
  delay(100);
}

long PC_2::startReceive() {
  long receive = 0;
  bool isReceived = false;
  while (!isReceived) {
    receive = 0;
    isReceived = receiveFrameDAC(&receive, 8, 500);
    delayMicroseconds(300000);
    bool crc = checkCRC(receive, 8);
    bool crc2 = crc;
    if (!isReceived) continue;
    while ((isReceived && crc2 &&((receive >> 4) & 1) != receiveFrameCount &&
    ((receive >> 5) & 7) != 0)){
      PC_2::sendACK();
      long tmp = 0;
      isReceived = receiveFrameDAC(&tmp, 8, 1100);
      crc2 = checkCRC(tmp,8);
      if (isReceived && tmp != receive) break; // Another Frame
      if (!isReceived && tmp != 0) isReceived = true; // Partial Bit
    }

    if (isReceived && ((receive >> 4) & 1) == receiveFrameCount &&
        ((receive >> 5) & 7) != 0) {
      if (crc) {
        receiveFrameCount = (receiveFrameCount + 1) % 2;
        while (isReceived) {
          delay(200);
          PC_2::sendACK();
          long tmp = 0;
          isReceived = receiveFrameDAC(&tmp, 8, 1100);
          if (isReceived && tmp != receive) break; // Another Frame
          if (!isReceived && tmp != 0)
            isReceived = true; // Partial Bit
          
        }
        return receive;
      } else {
        receive = 0;
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
  bool isReceived = receiveFrameDAC(&receive, 8, 1050);
  delay(150);
  if (isReceived && ((receive >> 5) & 7) == 0 &&
      ((receive >> 4) & 1) == (PC_2::sendFrameCount + 1) % 2) {
    if (checkCRC(receive, 8)) {
      // Serial.println("ACK Received");
      return ACKStatus::R;
    } else {
      // Serial.println("ACK Received, Incorrect Data");
      return ACKStatus::NR;
    }
  }
  // Serial.println("ACK Not Received");
  return ACKStatus::NR;
}

long PC_2::makeFrame(int command, int frameNo, long data) {
  /*
  [1] command + [1] frameNo + [9] data + [4] CRC
  */
  long bits = 0;
  bits <<= 1;
  bits += command;
  bits <<= 1;
  bits += (frameNo & 1);
  bits <<= 9;
  bits += (data & 511);
  bits <<= 4;
  bits += generateCRC(bits, 16);
  return bits;
}