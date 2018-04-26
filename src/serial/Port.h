#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <string>
#include <vector>

namespace serial
{

class Port
{
public:
  Port();
  ~Port();

  void open(std::string path);
  void close();

  bool hasData();
  void send(std::vector<unsigned char>& data);
  void send(std::string message);
  void receive(std::vector<unsigned char>& data);
  std::string receive();

private:
  int fd;

  Port(const Port&);
  Port& operator=(const Port&);

};

}

#endif
