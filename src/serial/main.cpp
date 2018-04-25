#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions          */
#include <errno.h>   /* ERROR Number Definitions           */

#include <string>
#include <iostream>

int main()
{
  /* O_RDWR - Open for reading and writing                            */
  /* O_NOCTTY - No terminal will control the process opening the port */
  int fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

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
  settings.c_cc[VTIME] = 0; /* Wait indefinitely */ 

  /* Local settings for raw mode */

  /* Set mode to non-canonical (raw) to communicate with foreign devices */
  settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  /* Input settings for flow control */

  /* Turn off software based flow control (XON/XOFF) */
  settings.c_iflag &= ~(IXON | IXOFF | IXANY);

  /* Apply settings to serial port now */
  tcsetattr(fd, TCSANOW, &settings);

  /* Write buffer */
  char writeBuffer[] = "Abc"; 
  int bytesWritten = write(fd, writeBuffer, sizeof(writeBuffer));

  if(bytesWritten != sizeof(writeBuffer))
  {
    throw std::exception();
  }

  sleep(2);

  /* Read buffer */
  printf("Reading...\n");
  char readBuffer[32];
  int bytesRead = read(fd, &readBuffer, 31);

  if(bytesRead == 0)
  {
    std::cout << "Disconnected" << std::endl;
  }
  else if(bytesRead == -1)
  {
    throw std::exception();
  }

  readBuffer[bytesRead] = '\0';

  printf("Received: %i: %s\n", bytesRead, readBuffer);

  close(fd);

  return 0;
}
