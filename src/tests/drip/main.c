#include <serial/serial.h>

#include <stdio.h>
#include <unistd.h>

int main()
{
  ref(SeStream) stream = NULL;
  vector(unsigned char) incoming = NULL;

  stream = SeStreamOpen("/dev/cuaU0");
  incoming = vector_new(unsigned char);

  SeStreamWriteCStr(stream, "Hello World!");

  while(1)
  {
    SeStreamRead(stream, incoming);

    if(vector_size(incoming) > 0)
    {
      break;
    }

    sleep(1);
  }

  vector_push_back(incoming, '\0');
  printf("Received: %s\n", &vector_at(incoming, 0));

  SeStreamClose(stream);
  vector_delete(incoming);

  return 0;
}

