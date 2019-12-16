#include "stent.h"

#define SE_PACKET_TIMEOUT 2

struct SeStream;

ref(SeStream) SeStreamOpen(char *path);
void SeStreamRead(ref(SeStream) ctx, vector(unsigned char) buffer);
void SeStreamWrite(ref(SeStream) ctx, vector(unsigned char) buffer);
void SeStreamClose(ref(SeStream) ctx);
