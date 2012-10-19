#ifndef STRUTILS_H
#define STRUTILS_H

#include <stddef.h>

size_t remove_postfix_whitespace (char *str, size_t len);
size_t remove_prefix_whitespace (char *str, size_t len);
void removequotes (char *str);
int str_to_key_value_pairs (const char *str, const char separator, char *key,
                            size_t keylen, char *value, size_t valuelen);
size_t strip (char *str, size_t len);

#endif /* STRUTILS_H */

