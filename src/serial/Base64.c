#include "Base64.h"
#include "base64_impl.h"

void SeBase64Encode(vector(unsigned char) in, vector(unsigned char) out)
{
  unsigned char *tmp = NULL;
  size_t len = 0;

  tmp = base64_encode(&vector_at(in, 0), vector_size(in), &len);
  vector_resize(out, len);
  memcpy(&vector_at(out, 0), tmp, len);
  free(tmp);
}

void SeBase64Decode(vector(unsigned char) in, vector(unsigned char) out)
{
  unsigned char *tmp = NULL;
  size_t len = 0;

  tmp = base64_decode(&vector_at(in, 0), vector_size(in), &len);
  vector_resize(out, len);
  memcpy(&vector_at(out, 0), tmp, len);
  free(tmp);
}
