#include <stdbool.h>
#include <string.h>
#include "strutils.h"

#define WS "\t\n "

static inline bool is_whitespace (char c)
{
    unsigned int i=0;
    while (WS[i++]!='\0')
        if (c==WS[i])
            return true;

    return false;
}


size_t remove_postfix_whitespace (char *str, size_t len)
{
    int i = len-1;
    unsigned int ws = 0;

    while (i>=0 && is_whitespace(str[i])) {
        --i;
        ++ws;
    }

    str[len-ws] = '\0';

    return len-ws;
}


size_t remove_prefix_whitespace (char *str, size_t len)
{
    unsigned int ws = 0;

    while (ws<len && is_whitespace(str[ws]))
        ++ws;

    if (ws!=0) {
        memmove(str, str+ws, len-ws+1);
        len -= ws;
    }
    return len;
}


/* TODO: check if copies can fit into arrays */
int str_to_key_value_pairs (const char *str, const char separator, char *key,
                            size_t keylen, char *value, size_t valuelen)
{
    int separator_index = -1;

    for (int i=0; str[i] != '\0'; ++i)
        if (str[i] == separator)
            separator_index = i;

    if (separator_index == -1)
        return 0;

    strncpy(key, str, separator_index);
    key[separator_index] = '\0';

    strncpy(value, str+separator_index+1, strlen(str+separator_index+1));
    value[strlen(str+separator_index+1)] = '\0';

    return 1;
}





size_t strip (char *str, size_t len)
{
    len = remove_postfix_whitespace(str, len);
    return remove_postfix_whitespace(str, len);
}

