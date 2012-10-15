#ifndef STRUTILS_H
#define STRUTILS_H

#include <stddef.h>

size_t remove_postfix_whitespace (char *str, size_t len);
size_t remove_prefix_whitespace (char *str, size_t len);
size_t strip (char *str, size_t len);

#endif /* STRUTILS_H */

