#include <serial/serial.h>

#include <unistd.h>
#include <stdio.h>

int main()
{
  ref(SeDevice) device = NULL;
  vector(unsigned char) outgoing = NULL;
  vector(unsigned char) incoming = NULL;
  int count = 0;

  device = SeDeviceOpen("/dev/cuaU0");
  outgoing = vector_new(unsigned char);
  incoming = vector_new(unsigned char);

  while(1)
  {
    vector_clear(outgoing);
    vector_push_back(outgoing, 'H');
    vector_push_back(outgoing, 'E');
    vector_push_back(outgoing, 'L');
    vector_push_back(outgoing, 'O');

    SeDeviceWrite(device, outgoing);

    sleep(1);

    if(SeDeviceReady(device, SE_MODE_R, 0))
    {
      vector_resize(incoming, 128);
      SeDeviceRead(device, incoming);

      vector_push_back(incoming, '\0');
      printf("Received: %s\n", &vector_at(incoming, 0));

      count++;
    }

    if(count > 5)
    {
      break;
    }
  }

  SeDeviceClose(device);
  vector_delete(incoming);
  vector_delete(outgoing);

  return 0;
}

