#include "serial/serial.h"

#include <iostream>

#include <unistd.h>

int main()
{
  serial::Port port;

  port.open("/dev/ttyUSB0");

  while(true)
  {
    //try
    //{
      port.send("Hello");
      usleep(10000);
    //}
    //catch(std::exception&) { }

    bool closed = false;

    try
    {
      std::string data = port.receive();

      if(data == "Hello")
      {
        closed = true;
      }
    }
    catch(std::exception&) { }

    std::cout << closed << std::endl;

    usleep(10000);
  }

  return 0;
}
