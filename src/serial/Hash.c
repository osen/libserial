#include "Hash.h"
#include "sha1.h"

#include <stdio.h>

ref(sstream) SeHash(vector(unsigned char) input)
{
  SHA1_CTX sha = {0};
  uint8_t output[41] = {0};
  char result[21] = {0};
  ref(sstream) rtn = NULL;
  size_t ci = 0;

  SHA1(result, &vector_at(input, 0), vector_size(input));

  for(ci = 0; ci < 20; ci++) {
    sprintf((output + (2 * ci)), "%02x", result[ci]&0xff);
  }

  rtn = sstream_new();
  sstream_str_cstr(rtn, output);

  return rtn;
}

/*


    SHA1_CTX sha; uint8_t results[20]; char *buf; int n;

    buf = "abc"; n = strlen(buf); SHA1Init(&sha); SHA1Update(&sha, (uint8_t *)buf, n); SHA1Final(results, &sha);

    // Print the digest as one long hex value

	printf("0x"); for (n = 0; n < 20; n++)

        printf("%02x", results[n]);

    putchar('n');

Alternately, the helper functions could be used in the following way:

    SHA1_CTX sha; uint8_t output[41]; char *buf = "abc";

    printf("0x%s", SHA1Data(buf, strlen(buf), output));

*/
