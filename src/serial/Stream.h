#include "stent.h"

#define SE_PING_TIMEOUT 2
#define SE_PACKET_TIMEOUT 1

struct SeStream;

ref(SeStream) SeStreamOpen(char *path);
void SeStreamRead(ref(SeStream) ctx, vector(unsigned char) buffer);
void SeStreamWrite(ref(SeStream) ctx, vector(unsigned char) buffer);
void SeStreamWriteCStr(ref(SeStream) ctx, char *str);
void SeStreamClose(ref(SeStream) ctx);
