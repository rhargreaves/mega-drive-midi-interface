#include "utils.h"

#if defined UNIT_TESTS && defined DEBUG
#include "cmocka_inc.h"
#endif

/* memcmp -- compare two memory regions.
   This function is in the public domain.
@deftypefn Supplemental int memcmp (const void *@var{x}, const void *@var{y}, @
  size_t @var{count})
Compares the first @var{count} bytes of two areas of memory.  Returns
zero if they are the same, a value less than zero if @var{x} is
lexically less than @var{y}, or a value greater than zero if @var{x}
is lexically greater than @var{y}.  Note that lexical order is determined
as if comparing unsigned char arrays.
@end deftypefn
*/
s16 memcmp(const void* str1, const void* str2, size_t count)
{
    register const unsigned char* s1 = (const unsigned char*)str1;
    register const unsigned char* s2 = (const unsigned char*)str2;

    while (count-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}

void debug_message(const char* fmt, ...)
{
#if defined UNIT_TESTS && defined DEBUG
    va_list args;
    va_start(args, fmt);
    vprint_message(fmt, args);
    va_end(args);
#endif
}
