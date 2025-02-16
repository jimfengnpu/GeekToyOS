#include <lib/const.h>
#include <lib/string.h>

void* memcpy(void* dst, const void* src, size_t n) {
    char *d = dst;
    const char *s = src;
    long* dptr = align((addr_t)d, word_size);
    long* dptr_end = align_down((addr_t)d+n, word_size);
    ssize_t sz = n;
    while (sz > 0 && d < (char*)dptr) {
        *(d++) = *(s++);
        sz--;
    }
    long* sptr = (long*)s;
    if(!((addr_t) sptr & size_mask(word_size))){
        while (sz > 0 && d < (char*)dptr_end) {
            *(dptr++) = *(sptr++);
            sz -= word_size;
        }
        d = (char*)dptr;
        s = (char*)sptr;
    }
    while (sz > 0) {
        *(d++) = *(s++);
        sz--;
    }
    return dst;
}

void* memmove(void* dst, const void* src, size_t n) {
    unsigned long ldst = (unsigned long)dst, lsrc = (unsigned long)src;
    if(ldst - lsrc >= n) 
    {
        memcpy(dst, src, n);
    } else 
    {
        char *d = dst;
        const char *s = src;
        d += n; s += n;
        while (n > 0) {
            *(--d) = *(--s);
            n--;
        }
    }
    return dst;
}

void *memset(void* p_dst, int ch, size_t size) {
    char* p = p_dst;
    while (size > 0) {
        *(p++) = ch;
        size--;
    }
    return p_dst;
}

int memcmp(const void* p1, const void* p2, size_t size) {
    const char *pchar1 = p1, *pchar2 = p2;
    char c1, c2;
    while (size > 0) {
        c1 = *(pchar1++);
        c2 = *(pchar2++);
        if (c1 != c2) { return c1 - c2; }
        size--;
    }
    return 0;
}

char* strcat(char* dst, const char* src) {
    if (dst == 0 || src == 0) return 0;
    char* temp = dst;
    while (*temp != '\0') temp++;
    while ((*temp++ = *src++) != '\0');

    return dst;
}

char* strncat(char* dst, const char* src, size_t n) {
    if (dst == 0 || src == 0) return 0;
    char* temp = dst;
    while (*temp != '\0') temp++;
    while (n > 0 && (*temp++ = *src++) != '\0')
    {
        n--;
    }

    return dst;
}

char* strcpy(char* p_dst, const char* p_src) {
    char c = 0, *p = p_dst;
    do {
        c = *(p_src++);
        *(p++) = c;
    } while (c);
    return p_dst;
}

int strlen(const char* p_str) {
    int cnt = 0;
    while (*(p_str++)) { cnt++; }
    return cnt;
}

int strnlen(const char *s, size_t size)
{
	int n;

	for (n = 0; size > 0 && *s != '\0'; s++, size--)
		n++;
	return n;
}

char* strncpy(char* dest, const char* src, size_t n) {
    if ((dest == 0) || (src == 0)) { /* for robustness */
        return dest;
    }
    size_t i;
    for (i = 0; i < n; i++) {
        dest[i] = src[i];
        if (!src[i]) { break; }
    }
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    if ((s1 == 0) || (s2 == 0)) { /* for robustness */
        return (s1 - s2);
    }

    const char* p1 = s1;
    const char* p2 = s2;

    for (; *p1 && *p2; p1++, p2++) {
        if (*p1 != *p2) { break; }
    }

    return (*p1 - *p2);
}

int strncmp(const char* s1, const char* s2, size_t  n) {
    if (!s1 || !s2) { return s1 - s2; }
    size_t i;
    for (i = 0; i < n; i++) {
        if (s1[i] != s2[i]) { return s1[i] - s2[i]; }
    }
    return 0;
}

char* strchr(const char* s, char c) {
    if (!s) { return 0; }
    const char* p;
    for (p = s; *p; p++) {
        if (*p == c) { break; }
    }
    return (*p == c) ? (char*)p : 0;
}

char* strrchr(const char* s, char c) {
    if (!s) { return 0; }
    const char* r = 0;
    while (*s) {
        if (*s == c) { r = s; }
        ++s;
    }
    return (char*)r;
}