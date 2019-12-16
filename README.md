# libserial

An ANSI C library for communicating via RS-232 serial port. It comes
with two interfaces **Device** for reading and writing to the raw
device and **Stream** which provides a reliable transmission protocol
in order to eliminate lost or corrupted packets.

The following snippet demonstrates how the **Stream** interface can be used.

    #include <serial/serial.h>

    ref(SeStream) stream = NULL;
    vector(unsigned char) outgoing = NULL;
    vector(unsigned char) incoming = NULL;

    stream = SeStreamOpen("/dev/cuaU0");
    outgoing = vector_new(unsigned char);
    incoming = vector_new(unsigned char);

    vector_push_back(outgoing, 'H');
    vector_push_back(outgoing, 'E');
    vector_push_back(outgoing, 'L');
    vector_push_back(outgoing, 'O');

    SeStreamWrite(stream, outgoing);

    while(1)
    {
      SeStreamRead(stream, incoming);

      if(vector_size(incoming) > 0)
      {
        break;
      }
    }

    vector_push_back(incoming, '\0');
    printf("Received: %s\n", &vector_at(incoming, 0));

    SeStreamClose(stream);
    vector_delete(incoming);
    vector_delete(outgoing);

Note that the incoming buffer will contain the message "HELO". It
is guaranteed to be sent and received, even if the serial is
interrupted.  The message will also be received as a whole. It is
not possible to receive an incomplete message such as "HE", "LO".
