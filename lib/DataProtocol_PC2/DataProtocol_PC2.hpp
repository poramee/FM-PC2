#include "Transceiver.hpp"

namespace PC_2 {
extern int sendFrameCount;
extern int receiveFrameCount;
void startSend(long data);
long startReceive();
long makeFrame(int command, int frameNo, long data);
void sendACK();
ACKStatus waitingForACK();
}; // namespace PC_2