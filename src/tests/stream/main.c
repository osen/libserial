#include <serial/serial.h>

#include <unistd.h>
#include <stdio.h>

int main()
{
  ref(SeStream) stream = NULL;
  vector(unsigned char) outgoing = NULL;
  vector(unsigned char) incoming = NULL;

  stream = SeStreamOpen("/dev/cuaU0");
  outgoing = vector_new(unsigned char);
  incoming = vector_new(unsigned char);

  while(1)
  {
    vector_clear(outgoing);
    vector_push_back(outgoing, 'H');
    vector_push_back(outgoing, 'E');
    vector_push_back(outgoing, 'L');
    vector_push_back(outgoing, 'O');

    SeStreamWrite(stream, outgoing);

    sleep(1);

    SeStreamRead(stream, incoming);

    if(vector_size(incoming) > 0)
    {
      vector_push_back(incoming, '\0');
      printf("Received: %s\n", &vector_at(incoming, 0));
    }
    else
    {
      printf("No bytes received\n");
    }

    //break;
  }

  SeStreamClose(stream);
  vector_delete(incoming);
  vector_delete(outgoing);

  return 0;
}

