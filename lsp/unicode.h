#pragma once
#include <stdio.h>

/* Gets a UTF-8 codepoint from stream.

Returns a codepoint at most U+10FFFF upon success.

Returns EOF (-1) if EOF is encountered before decoding the codepoint.

Returns -2 if EOF is encountered in the middle of decoding the codepoint.

Returns -3 if an encoding error is encountered before encountering EOF.
*/
int utf8_getc(FILE *stream);

/* Gets a UTF-8 codepoint from stream.

Returns a codepoint at most U+10FFFF upon success.

Returns EOF (-1) if EOF is encountered before decoding the codepoint.

Returns -2 if EOF is encountered in the middle of decoding the codepoint.

Returns -3 if an encoding error is encountered before encountering EOF.
*/
int utf16_getc(FILE *stream);
