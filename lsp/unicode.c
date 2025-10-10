/* rfc3629: UTF-8, a transformation format of ISO 10646
   Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
   --------------------+---------------------------------------------
   0000 0000-0000 007F | 0xxxxxxx
   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
   0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

                Scalar Value    UTF-16
            xxxxxxxxxxxxxxxx    xxxxxxxxxxxxxxxx
    000uuuuuxxxxxxxxxxxxxxxx    110110wwwwxxxxxx 110111xxxxxxxxxx
Note: wwww = uuuuu - 1
*/
#include "unicode.h"

int utf8_getc(FILE *stream) {
  int c = getc(stream);
  if (c == EOF) {
    return EOF;
  }
  if (c & 0x80) {
    int mask = 0x40;
    if (c & mask) {
      c ^= 0x80;
      do {
        int d = getc(stream);
        if (d == EOF) {
          return -2;
        }
        if ((d ^= 0x80) >> 6) {
          return -3;
        }
        c = (c ^ mask) << 6 ^ d;
      } while (c & (mask <<= 7));
    } else {
      return -3;
    }
  }
  return c;
}

int utf16_getc(FILE *stream) {
  int c = getc(stream);
  if (c == EOF) {
    return EOF;
  }
  if (c >> 10 == 0xD8) {
    int d = getc(stream);
    if (d == EOF) {
      return -2;
    }
    if ((d ^= 0xDC00) >> 10) {
      return -3;
    }
    c = (c ^ 0xD8) << 10 ^ (d + 1);
  }
  return c;
}
