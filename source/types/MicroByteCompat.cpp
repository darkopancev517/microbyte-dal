#include "MicroByteConfig.h"
#include "MicroByteCompat.h"
#include "ErrorNo.h"

int string_reverse(char *s)
{
    //sanity check...
    if(s == NULL)
        return MICROBYTE_INVALID_PARAMETER;

    char *j;
    int c;

    j = s + strlen(s) - 1;

    while(s < j)
    {
        c = *s;
        *s++ = *j;
        *j-- = c;
    }

    return MICROBYTE_OK;
}

int itoa(int n, char *s)
{
    int i = 0;
    int positive = (n >= 0);

    if (s == NULL)
        return MICROBYTE_INVALID_PARAMETER;

    // Record the sign of the number,
    // Ensure our working value is positive.
    if (positive)
        n = -n;

    // Calculate each character, starting with the LSB.
    do {
         s[i++] = abs(n % 10) + '0';
    } while (abs(n /= 10) > 0);

    // Add a negative sign as needed
    if (!positive)
        s[i++] = '-';

    // Terminate the string.
    s[i] = '\0';

    // Flip the order.
    string_reverse(s);

    return MICROBYTE_OK;
}
