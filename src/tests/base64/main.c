#include <serial/Base64.h>

#include <stdio.h>

int main()
{
  vector(unsigned char) outgoing = NULL;
  vector(unsigned char) incoming = NULL;

  outgoing = vector_new(unsigned char);
  incoming = vector_new(unsigned char);

  vector_push_back(outgoing, 'H');
  vector_push_back(outgoing, 'E');
  vector_push_back(outgoing, 'L');
  vector_push_back(outgoing, 'O');

  SeBase64Encode(outgoing, incoming);
  SeBase64Decode(incoming, incoming);

  vector_push_back(incoming, '\0');

  printf("Current: [%s]\n", &vector_at(incoming, 0));

  vector_delete(incoming);
  vector_delete(outgoing);

  return 0;
}

