#include <Arduino.h>

namespace Transceiver {
void init();
}
int generateCRC(long data, int length);   // X^4 + X + 1
bool checkCRC(long data, int length);     // X^4 + X + 1
void sendFrameDAC(long bits, int length); // Send Frame
bool receiveFrameDAC(long *receivedBit, int numBit,
                     unsigned int timeoutMillis); // Receive and Error checking
enum ACKStatus {
  R,  // Received
  NR, // Not Receive
};