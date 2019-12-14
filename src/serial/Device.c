#include "Device.h"

#define STENT_IMPLEMENTATION
#include "stent.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>

ref(SeDevice) SeDeviceOpen(char *path)
{
  ref(SeDevice) rtn = NULL;
  struct termios options = {0};

  rtn = allocate(SeDevice);
  _(rtn).fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);

  if(_(rtn).fd == -1)
  {
    release(rtn);

    return NULL;
  }

  cfsetispeed(&options, B9600);
  cfsetospeed(&options, B9600);

  tcgetattr(_(rtn).fd, &options);

  options.c_cflag |= (CLOCAL | CREAD);

  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;

  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_iflag &= ~(IXON | IXOFF | IXANY);
  options.c_oflag &= ~OPOST;

  tcsetattr(_(rtn).fd, TCSAFLUSH, &options);

  return rtn;
}

int SeDeviceReady(ref(SeDevice) ctx, int mode, int timeout)
{
  fd_set readfds = {0};
  fd_set writefds = {0};
  struct timeval tv = {0};
  int rc = 0;
  int bytes = 0;

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = timeout % 1000;

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);

  if(mode == SE_MODE_R || mode == SE_MODE_RW)
  {
    FD_SET(_(ctx).fd, &readfds);
  }

  if(mode == SE_MODE_W || mode == SE_MODE_RW)
  {
    FD_SET(_(ctx).fd, &writefds);
  }

  if(select(_(ctx).fd + 1, &readfds, &writefds, NULL,
    (timeout >= 0 ? &tv : NULL)) == -1)
  {
    printf("Error: Select failed\n");
    abort();
  }

  if(mode == SE_MODE_R || mode == SE_MODE_RW)
  {
    if(FD_ISSET(_(ctx).fd, &readfds))
    {
      rc = ioctl(_(ctx).fd, FIONREAD, &bytes);

      if(rc == -1)
      {
        printf("Error: Failed to enumerate waiting bytes\n");
        abort();
      }

      if(bytes > 0)
      {
        return 1;
      }

      printf("Error: Serial connection closed?\n");
      abort();
    }
  }

  if(mode == SE_MODE_W || mode == SE_MODE_RW)
  {
    if(FD_ISSET(_(ctx).fd, &writefds))
    {
      return 1;
    }
  }

  return 0;
}

void SeDeviceRead(ref(SeDevice) ctx, vector(unsigned char) buffer)
{
  int bytes = 0;

  bytes = read(_(ctx).fd, &vector_at(buffer, 0), vector_size(buffer));

  if(bytes == -1)
  {
    printf("Error: Failed to read\n");
    abort();
  }

  if(bytes == 0)
  {
    printf("Error: Encountered EOF?\n");
    abort();
  }

  if(bytes < vector_size(buffer))
  {
    vector_resize(buffer, bytes);
  }
}

void SeDeviceWrite(ref(SeDevice) ctx, vector(unsigned char) buffer)
{
  int n = 0;

  n = write(_(ctx).fd, &vector_at(buffer, 0), vector_size(buffer));

  if(n < 1)
  {
    printf("Error: Failed to send\n");
    abort();
  }

  vector_erase(buffer, 0, n);
}

void SeDeviceClose(ref(SeDevice) ctx)
{
  close(_(ctx).fd);
  release(ctx);
}

