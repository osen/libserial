#include "serial/serial.h"

#include <iostream>

#include <unistd.h>

int main()
{
  serial::Port port;

  port.open("/dev/ttyUSB0");
  port.send("Hello");

  usleep(10000);

  std::string data = port.receive();

  std::cout << "Received: " << data << std::endl;

  return 0;
}
