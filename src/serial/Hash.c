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
