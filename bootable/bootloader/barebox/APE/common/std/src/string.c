/* string.c
 *
 * public-domain implementation.
 *
 */
 
#include "com_type.h"

/* uppercase */
void uppercase(char *in)
{
	int i = 0;
	while (0x00 != in[i])
	{
		/* Change case for lower case character only */
		if (in[i] >= 97 && in[i] <= 122)
		{
			in[i] = in[i] - 32;
		}
		
		i++;
	}
}

/* strcpy */
char * strcpy(char *s1, const char *s2)
{
     char *dst = s1;
     const char *src = s2;
     /* Do the copying in a loop.  */
     while ((*dst++ = *src++) != '\0')
         ;               /* The body of this loop is left empty. */
     /* Return the destination string.  */
     return s1;
}


/* strcmp */
int strcmp(const char *s1, const char *s2)
{
     unsigned char uc1, uc2;
     /* Move s1 and s2 to the first differing characters 
        in each string, or the ends of the strings if they
        are identical.  */
     while (*s1 != '\0' && *s1 == *s2) {
         s1++;
         s2++;
     }
     /* Compare the characters as unsigned char and
        return the difference.  */
     uc1 = (*(unsigned char *) s1);
     uc2 = (*(unsigned char *) s2);
     return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}


/* strncmp */
int strncmp(const char *s1, const char *s2, size_t n)
{
     unsigned char uc1, uc2;
     /* Nothing to compare?  Return zero.  */
     if (n == 0)
         return 0;
     /* Loop, comparing bytes.  */
     while (n-- > 0 && *s1 == *s2) {
         /* If we've run out of bytes or hit a null, return zero
            since we already know *s1 == *s2.  */
         if (n == 0 || *s1 == '\0')
             return 0;
         s1++;
         s2++;
     }
     uc1 = (*(unsigned char *) s1);
     uc2 = (*(unsigned char *) s2);
     return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}



/* strlen */
size_t strlen(const char *s)
{
     const char *p = s;
     /* Loop over the data in s.  */
     while (*p != '\0')
         p++;
     return (size_t)(p - s);
}


/* memset */
void * memset(void *s, int c, size_t n)
{
     unsigned char *us = s;
     unsigned char uc = c;
     while (n-- != 0)
         *us++ = uc;
     return s;
}


/* memcpy */
void * memcpy(void *s1, const void *s2, size_t n)
{
     char *dst = s1;
     const char *src = s2;
     /* Loop and copy.  */
     while (n-- != 0)
         *dst++ = *src++;
     return s1;
}


/* memmove */
void * memmove(void *s1, const void *s2, size_t n) 
{
    /* note: these don't have to point to unsigned chars */
    char *p1 = s1;
    const char *p2 = s2;
    /* test for overlap that prevents an ascending copy */
    if (p2 < p1 && p1 < p2 + n) {
        /* do a descending copy */
        p2 += n;
        p1 += n;
        while (n-- != 0) 
            *--p1 = *--p2;
    } else 
        while (n-- != 0) 
            *p1++ = *p2++;
    return s1; 
}


/* memcmp */
int memcmp(const void *s1, const void *s2, size_t n)
{
     const unsigned char *us1 = (const unsigned char *) s1;
     const unsigned char *us2 = (const unsigned char *) s2;
     while (n-- != 0) {
         if (*us1 != *us2)
             return (*us1 < *us2) ? -1 : +1;
         us1++;
         us2++;
     }
     return 0;
}

/* memchr */
void * memchr(const void *s, int c, size_t n)
{
     const unsigned char *src = s;
     unsigned char uc = c;
     while (n-- != 0) {
         if (*src == uc)
             return (void *) src;
         src++;
     }
     return NULL;
}

/* end - public-domain implementation */
