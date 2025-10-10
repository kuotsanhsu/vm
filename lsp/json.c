#include "unicode.h"

#define crBegin                                                                \
  static int state = 0;                                                        \
  switch (state) {                                                             \
  case 0:
#define crReturn(x)                                                            \
  do {                                                                         \
    state = __LINE__;                                                          \
    return x;                                                                  \
  case __LINE__:;                                                              \
  } while (0)
#define crFinish }

/*
JSON-text = ws value ws

   These are the six structural characters:

      begin-array     = ws %x5B ws  ; [ left square bracket

      begin-object    = ws %x7B ws  ; { left curly bracket

      end-array       = ws %x5D ws  ; ] right square bracket

      end-object      = ws %x7D ws  ; } right curly bracket

      name-separator  = ws %x3A ws  ; : colon

      value-separator = ws %x2C ws  ; , comma

   Insignificant whitespace is allowed before or after any of the six
   structural characters.

*/
enum json_lexeme : char {
  JSON_ws = ' ',
  JSON_begin_array = '[',
  JSON_begin_object = '{',
  JSON_end_array = ']',
  JSON_end_object = '}',
  JSON_name_separator = ':',
  JSON_value_separator = ',',

  JSON_decimal_point = '.',
  JSON_digit19 = '1',
  JSON_e = 'e', // e E
  JSON_minus = '-',
  JSON_plus = '+',
  JSON_zero = '0',

  JSON_quotation_mark = '"',
  JSON_escape = '\\',
};

enum json_lexeme json_lexer(FILE *stream) {
  while (1) {
    const int c = utf8_getc(stream);
    switch (c) {
    case JSON_begin_array:
    case JSON_begin_object:
    case JSON_end_array:
    case JSON_end_object:
    case JSON_name_separator:
    case JSON_value_separator:
    case JSON_decimal_point:
    case JSON_minus:
    case JSON_plus:
    case JSON_zero:
      return c;
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      while (1) {
        const int c = utf8_getc(stream);
        switch (c) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
          break;
        default:
          ungetc(c, stream);
          return JSON_ws;
        }
      }
    case 'e':
    case 'E':
      return JSON_e;
    default:
      if ('1' <= c && c <= '9') {
        return JSON_digit19;
      }
    }
  }
}

struct json_value {
  enum {
    JSON_false,
    JSON_null,
    JSON_true,
    JSON_object,
    JSON_array,
    JSON_number,
    JSON_string,
  } type;
  union {
    struct {
      char *name;
      struct json_value *value;
    } object[];
    struct json_value *array[];
    double number;
    char *string;
  };
};

/*
object = begin-object [ member *( value-separator member ) ]
               end-object

      member = string name-separator value

*/

/*
```
string = quotation-mark *char quotation-mark

char = unescaped /
    escape (
        %x22 /          ; "    quotation mark  U+0022
        %x5C /          ; \    reverse solidus U+005C
        %x2F /          ; /    solidus         U+002F
        %x62 /          ; b    backspace       U+0008
        %x66 /          ; f    form feed       U+000C
        %x6E /          ; n    line feed       U+000A
        %x72 /          ; r    carriage return U+000D
        %x74 /          ; t    tab             U+0009
        %x75 4HEXDIG )  ; uXXXX                U+XXXX

escape = %x5C              ; \

quotation-mark = %x22      ; "

unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
```
*/
static ssize_t parse_string(FILE *stream, char *restrict buf, size_t buf_size) {
  const char *const buf_begin = buf;
  const char *const buf_end = buf_begin + buf_size;
  for (; buf != buf_end; ++buf) {
    const int c = utf8_getc(stream);
    if (c < 0) {
      // TODO: EOF or UTF-8 error
    }
    switch (c) {
    case '"':
      *buf = '\0';
      return buf - buf_begin;
    case '\\': {
      const int c = utf8_getc(stream);
      switch (c) {
      case '"':
      case '\\':
      case '/':
        *buf = c;
        break;
      case 'b':
        *buf = '\b';
        break;
      case 'f':
        *buf = '\f';
        break;
      case 'n':
        *buf = '\n';
        break;
      case 'r':
        *buf = '\r';
        break;
      case 't':
        *buf = '\t';
        break;
      case 'u': {
        int hex4 = 0;
        for (int i = 0; i != 4; ++i) {
          int c = utf8_getc(stream);
          if ('0' <= c && c <= '9') {
            c -= '0';
          } else if ('a' <= c && c <= 'f') {
            c = c - 'a' + 10;
          } else if ('A' <= c && c <= 'F') {
            c = c - 'A' + 10;
          } else {
            return -1;
          }
          hex4 = (hex4 << 4) ^ c;
        }
        // TODO: decode UTF-16
      } break;
      default:
        return -1;
      }
    } break;
    default:
      // TODO: decode UTF-8
      break;
    }
  }
  return -1;
}

/*
number = [ minus ] int [ frac ] [ exp ]

exp = e [ minus / plus ] 1*DIGIT

frac = decimal-point 1*DIGIT

int = zero / ( digit1-9 *DIGIT )
*/
static double parse_number(FILE *stream, int c0) {
  if (c0 != 0) {
    if (c0 == -1) {
      c0 = 0;
    }
    // TODO: parse rest of int
  }
  // TODO: parse frac and exp
  return 0;
}

void parser(FILE *stream) {
  // const enum json_lexeme lexeme = json_lexer(stream);
  const int c = utf8_getc(stream);
  if ('0' <= c && c <= '9') {
    parse_number(stream, c - '0');
  } else {
    switch (c) {
    case '-':
      -parse_number(stream, -1);
      break;
    case '"':
      parse_string(stream);
      break;
    case JSON_begin_array:
      break;
    case JSON_begin_object:
      while (1) {
        const enum json_lexeme lexeme = json_lexer(stream);
        switch (lexeme) {
        case JSON_end_object:
          break; // finalize
        }
      }
      break;
    }
  }
}
