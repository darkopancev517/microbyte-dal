#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

#define ARRAY_END(array) (&array[ARRAY_LENGTH(array)])

#define ALIGNED_VAR_SIZE(size, align_type) (((size) + (sizeof(align_type) - 1)) / sizeof(align_type))

#define DEFINE_ALIGNED_VAR(name, size, align_type) \
    align_type name[(((size) + (sizeof(align_type) - 1)) / sizeof(align_type))]

#define VERIFY_OR_EXIT(condition, ...) \
    do \
    { \
        if (!(condition)) \
        { \
            __VA_ARGS__; \
            goto exit; \
        } \
    } while (0)

#endif /* UTILS_H */
