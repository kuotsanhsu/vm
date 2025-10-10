## Coroutines

- https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

```c
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

void parser(int c) {
  crBegin;
  while (1) {
    /* first char already in c */
    if (c == EOF)
      break;
    if (isalpha(c)) {
      do {
        add_to_token(c);
        crReturn();
      } while (isalpha(c));
      got_token(WORD);
    }
    add_to_token(c);
    got_token(PUNCT);
    crReturn();
  }
  crFinish;
}
```

## JSON

- https://www.json.org/json-en.html
- https://datatracker.ietf.org/doc/html/rfc8259
- https://ecma-international.org/publications-and-standards/standards/ecma-404/

## UTF-8

- [UTF-8, a transformation format of ISO 10646](https://datatracker.ietf.org/doc/html/rfc3629)
- [8.1.](https://datatracker.ietf.org/doc/html/rfc8259#section-8.1)  Character Encoding
  > JSON text exchanged between systems that are not part of a closed ecosystem MUST be encoded using UTF-8 [RFC3629](https://datatracker.ietf.org/doc/html/rfc3629).
- [Everything You Never Wanted to Know about Unicode Normalization](https://stackoverflow.com/a/7934397/16371358)
- [ICU - International Components for Unicode](https://icu.unicode.org/)

## Hash table

- https://www.reddit.com/r/C_Programming/comments/mdnpv6/how_to_implement_a_hash_table_in_c/
