#ifndef CONFIG_H
#define CONFIG_H

#define _POSIX_C_SOURCE 200809L

#define DEBUG 1

#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)


#endif /* CONFIG_H */

