#include "Port.h"

#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions          */
#include <errno.h>   /* ERROR Number Definitions           */

namespace serial
{

Port::Port() : fd(-1) { }

Port::~Port()
{
  if(fd != -1)
  {
    close();
  }
}

void Port::open(std::string path)
{
  if(fd != -1)
  {
    throw std::exception();
  }

  /* O_RDWR - Open for reading and writing                              */
  /* O_NOCTTY - No terminal will control the process opening the port   */
  /* O_NDELAY - Enable non-blocking for raw mode when VTIME is set to 0 */
  fd = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

  if(fd == -1)
  {
    throw std::exception();
  }

  termios settings;
  tcgetattr(fd, &settings);

  /* Set baud rate for both read and writing to 9600 */
  cfsetispeed(&settings, B9600);
  cfsetospeed(&settings, B9600);

  /* Control settings for 8N1 */

  settings.c_cflag &= ~PARENB; /* No parity bit */
  settings.c_cflag &= ~CSTOPB; /* Stop bits = 1 */

  settings.c_cflag &= ~CSIZE; /* Clears the Mask       */
  settings.c_cflag |=  CS8;   /* Set the data bits = 8 */

  /* Turn off hardware based flow control (RTS/CTS) */
  settings.c_cflag &= ~CRTSCTS;

  /* Turn on the receiver of the serial port (CREAD), so that reading from the serial port will work */
  settings.c_cflag |= CREAD | CLOCAL;

  settings.c_cc[VMIN]  = 10; /* Read 1 characters */
  settings.c_cc[VTIME] = 0;  /* Wait indefinitely */ 

  /* Local settings for raw mode */

  /* Set mode to non-canonical (raw) to communicate with foreign devices */
  settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  /* Input settings for flow control */

  /* Turn off software based flow control (XON/XOFF) */
  settings.c_iflag &= ~(IXON | IXOFF | IXANY);

  /* Apply settings to serial port now */
  tcsetattr(fd, TCSANOW, &settings);
}

void Port::send(std::string message)
{
  std::vector<unsigned char> data;

  for(size_t i = 0; i < message.length(); i++)
  {
    data.push_back(message.at(i));
  }

  send(data);
}

void Port::send(std::vector<unsigned char>& data)
{
  if(fd == -1)
  {
    throw std::exception();
  }

  size_t dataSize = sizeof(unsigned char) * data.size();
  int bytesWritten = write(fd, &data[0], dataSize);

  if(bytesWritten != dataSize)
  {
    throw std::exception();
  }
}

std::string Port::receive()
{
  std::vector<unsigned char> data;

  receive(data);
  std::string rtn;

  for(size_t i = 0; i < data.size(); i++)
  {
    if(data.at(i) == '\0') break;

    rtn = rtn + (char)data.at(i);
  }

  return rtn;
}

void Port::receive(std::vector<unsigned char>& data)
{
  if(fd == -1)
  {
    throw std::exception();
  }

  char readBuffer[32];
  int bytesRead = read(fd, &readBuffer, 31);

  if(bytesRead == 0)
  {
    close();

    return;
  }
  else if(bytesRead == -1)
  {
    throw std::exception();
  }

  data.clear();

  for(size_t i = 0; i < bytesRead; i++)
  {
    data.push_back(readBuffer[i]);
  }
}

void Port::close()
{
  if(fd == -1)
  {
    throw std::exception();
  }

  ::close(fd);
  fd = -1;
}

}
