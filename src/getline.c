#include <stdlib.h>
#include <stdio.h>

#include "getline.h"


int getline_custom (char **line, size_t *size, FILE *file)
{
    int retval = 0;
    char * str = *line;
    int chr;

    if (*size==0 || *line == NULL || file == NULL)
        return -1;

   
    while (1) {
        if (retval == *size) {
            str = (char *) realloc((void*)str, (*size)*2);

            if (str == NULL) {
                str[*size-1] = '\0';
                retval = -2;
                break;
            } else {
                (*size) *= 2;
                line = &str;
            }
        }

        chr = fgetc(file);
        
        if (chr==EOF) {
            /* End of file or error occured */
            str[retval] = '\0';
            break;
        } else {
            str[retval] = (char) chr;

            if (chr=='\n') {
                ++retval;
                
                /* check if need to realloc */
                if (retval == *size) {
                    str = (char *) realloc((void *)str, (*size)+1);
                    
                    if (str==NULL) {
                        str[*size-1] = '\0';
                        retval = -2;
                        break;
                    } else {
                        *size += 1;
                        line = &str;
                    }
                }
                str[retval] = '\0';
                break;
            }
        }

        ++retval;
    }

    return retval;
}

