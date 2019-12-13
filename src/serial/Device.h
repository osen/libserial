#include "stent.h"

#define SE_MODE_R 1
#define SE_MODE_W 2
#define SE_MODE_RW 3

struct SeDevice
{
  int fd;
};

ref(SeDevice) SeDeviceOpen(char *path);
int SeDeviceReady(ref(SeDevice) ctx, int mode, int timeout);
void SeDeviceRead(ref(SeDevice) ctx, vector(unsigned char) buffer);
void SeDeviceWrite(ref(SeDevice) ctx, vector(unsigned char) buffer);
void SeDeviceClose(ref(SeDevice) ctx);
