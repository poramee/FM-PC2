#include "Transceiver.hpp"
#include <Adafruit_MCP4725.h>
#include <Wire.h>

// Send & Receive Parameters
Adafruit_MCP4725 dac;
const int RxPin = A0;
const int freq[4] = {200, 300, 400, 500};
int delayFreq[4];
const int sample = 4;
int zeta[sample];
uint16_t S_DAC[sample];
const int baudRate = 100;
const int baudTime = 9500;
int cycle[4];

void Transceiver::init() {
  Serial.begin(115200);
  Serial.println("delay     cycle");
  for (int i = 0; i < 4; ++i) {
    delayFreq[i] = ((1000000 / freq[i]) / sample) - 147;
    cycle[i] = freq[i] / baudRate;
    Serial.print(delayFreq[i]);
    Serial.print("  ");
    Serial.println(cycle[i]);
  }
  Serial.println("i   S_DAC");
  for (int i = 0; i < sample; ++i) {
    zeta[i] = ((double)(i) / sample) * 360;
    S_DAC[i] = map(sin(zeta[i] * PI / 180) * 1000, -1000, 1000, 0, 3000);
    Serial.print(i);
    Serial.print("   ");
    Serial.println(S_DAC[i]);
  }
  randomSeed(analogRead(0));
  dac.begin(0x64);
  dac.setVoltage(0, false);
}

bool checkCRC(long data, int length) {
  int bits = 0;
  for (int i = length - 1; i >= 0; --i) {
    int x4 = (bits >> 3) & 1;
    bits <<= 1;
    bits += (data >> i) & 1;
    bits = bits ^ ((x4 << 1) + x4);
    bits &= 15;
  }
  int chk = 0;
  for (int i = 0; i < 4; ++i) {
    chk |= (bits >> i) & 1;
  }
  return !chk;
}
int generateCRC(long data, int length) {
  int bits = 0;
  for (int i = length - 1; i >= 0; --i) {
    int x4 = (bits >> 3) & 1;
    bits <<= 1;
    bits += (data >> i) & 1;
    bits = bits ^ ((x4 << 1) + x4);
    bits &= 15;
  }
  return bits;
}
void sendFrameDAC(long bits, int length) {
  // possible length value are 8 and 22
  // Serial.print("sendFrameDAC:");
  // for (int i = length - 1; i >= 0; --i) {
  //   Serial.print((bits >> i) & 1);
  // }
  // Serial.println();
  dac.setVoltage(0, false);
  for (int i = length - 2; i >= 0; i -= 2) {
    const int level = ((bits >> i) & 3);
    delay(3);
    for (int c = 0; c < cycle[level & 3]; c++) {
      for (int sl = 0; sl < sample; sl++) {
        dac.setVoltage(S_DAC[sl], false);
        delayMicroseconds(delayFreq[level & 3]);
      }
    }
    dac.setVoltage(0, false);
  }
}
bool receiveFrameDAC(long *receivedBit, int numBit,
                     unsigned int timeoutMillis) {
  const int r_slope = 100;
  int bitCount = 0;
  int prevReading = analogRead(RxPin);
  unsigned long startTime = micros();
  unsigned long functionInvokeTimeStamp = millis();
  int cycleCount = 0;
  bool check = false;
  bool startReading = false;
  int max = 0;
  unsigned long lastBitTimeStamp = millis();
  while (bitCount < numBit) {
    const int reading = analogRead(RxPin);
    if (millis() - functionInvokeTimeStamp > timeoutMillis && !startReading) {
      return false;
    }
    if (startReading && millis() - lastBitTimeStamp > 100) {
      *receivedBit = 1;
      return false;
    }
    if (reading - prevReading > r_slope && !check) { // rising signal
      max = 0;
      startReading = true;
      if (cycleCount == 0) {
        startTime = micros();
      }
      check = true;
    }
    if (reading > max)
      max = reading;
    if (max - reading > r_slope && check) { // falling signal
      ++cycleCount;
      check = false;
    }
    if (micros() - startTime > baudTime && cycleCount > 0) {
      check = false;
      *receivedBit <<= 2;
      *receivedBit += (cycleCount - 2);
      cycleCount = 0;
      bitCount += 2;
      lastBitTimeStamp = millis();
    }
    prevReading = reading;
  }
  return true;
}