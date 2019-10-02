
/*-------------------------------------------------------------------------
 *
 * stringinfo.c
 *
 * StringInfo provides an indefinitely-extensible string data type.
 * It can be used to buffer either ordinary C strings (null-terminated text)
 * or arbitrary binary data.  All storage is allocated with malloc().
 *
 * Portions Copyright (c) 1996-2018, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 *-------------------------------------------------------------------------
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define MaxAllocSize	((size_t) 0x3fffffff) /* 1 gigabyte - 1 */

typedef struct StringInfoData
{
    char       *data;
    int        len;
    int        maxlen;
    int        cursor;
}StringInfoData;


enum EXECSTATUS
{
    EXEC_FAIL,
    EXEC_SUCCESS
};

typedef StringInfoData *StringInfo;

extern StringInfo makeStringInfo(void);
extern int initStringInfo(StringInfo str);
extern int resetStringInfo(StringInfo str);
extern int appendStringInfo(StringInfo str, const char *fmt,...);
extern int appendStringInfoVA(StringInfo str, const char *fmt, va_list args);
extern int appendStringInfoString(StringInfo str, const char *s);
extern int appendStringInfoChar(StringInfo str, char ch);
#define appendStringInfoCharMacro(str,ch) \
        (((str)->len + 1 >= (str)->maxlen) ? \
        appendStringInfoChar(str, ch) : \
        (void)((str)->data[(str)->len] = (ch), (str)->data[++(str)->len] = '\0'))
extern int appendStringInfoSpaces(StringInfo str, int count);
extern int appendBinaryStringInfo(StringInfo str,
                                   const char *data, int datalen);
extern int appendBinaryStringInfoNT(StringInfo str,
                                     const char *data, int datalen);
extern int enlargeStringInfo(StringInfo str, int needed);
extern size_t pvsnprintf(char *buf, size_t len, const char *fmt, va_list args);
extern int freeStringInfo(StringInfo str);

StringInfo makeStringInfo(void)
{
    StringInfo    res;

    res = (StringInfo) malloc(sizeof(StringInfoData));

    initStringInfo(res);

    return res;
}

int initStringInfo(StringInfo str)
{
    if (NULL == str)
    {
        return EXEC_FAIL;
    }
    int size = 1024;    /* initial default buffer size */

    str->data = (char *) malloc(size);
    if (NULL == str->data)
    {
        return EXEC_FAIL;
    }
    str->maxlen = size;

    return resetStringInfo(str);
}

int resetStringInfo(StringInfo str)
{
    if (NULL == str || NULL == str->data)
    {
        return EXEC_FAIL;
    }
    str->data[0] = '\0';
    str->len = 0;
    str->cursor = 0;

    return EXEC_SUCCESS;
}

int appendStringInfo(StringInfo str, const char *fmt,...)
{
    for (;;)
    {
        va_list args;
        int     needed;

        /* Try to format the data. */
        va_start(args, fmt);
        needed = appendStringInfoVA(str, fmt, args);
        va_end(args);

        if (needed == 0)
            break;                /* success */

        /* Increase the buffer size and try again. */
        if (enlargeStringInfo(str, needed) != EXEC_SUCCESS)
        {
            return EXEC_FAIL;
        }
    }
}

int appendStringInfoVA(StringInfo str, const char *fmt, va_list args)
{
    int      avail;
    size_t   nprinted;

    if (NULL == str)
    {
        return EXEC_FAIL;
    }
    avail = str->maxlen - str->len;
    if (avail < 16)
        return 32;

    nprinted = pvsnprintf(str->data + str->len, (size_t) avail, fmt, args);

    if (nprinted < (size_t) avail)
    {
        str->len += (int) nprinted;
        return 0;
    }

    str->data[str->len] = '\0';

    return (int) nprinted;
}


int appendStringInfoString(StringInfo str, const char *s)
{
    return appendBinaryStringInfo(str, s, strlen(s));
}

int appendStringInfoChar(StringInfo str, char ch)
{
    if (NULL == str)
    {
        return EXEC_FAIL;
    }
    /* Make more room if needed */
    if (str->len + 1 >= str->maxlen)
    {
        if (enlargeStringInfo(str, 1) != EXEC_SUCCESS)
        {
            return EXEC_FAIL;
        }
    }

    /* OK, append the character */
    str->data[str->len] = ch;
    str->len++;
    str->data[str->len] = '\0';

    return EXEC_SUCCESS;
}


int appendStringInfoSpaces(StringInfo str, int count)
{
    if (count > 0)
    {
        /* Make more room if needed */
        if (enlargeStringInfo(str, count) != EXEC_SUCCESS)
        {
            return EXEC_FAIL;
        }

        /* OK, append the spaces */
        while (--count >= 0)
            str->data[str->len++] = ' ';
        str->data[str->len] = '\0';
    }

    return EXEC_SUCCESS;
}

int appendBinaryStringInfo(StringInfo str, const char *data, int datalen)
{
    if (NULL == str || NULL == data || datalen < 0)
    {
        return EXEC_FAIL;
    }

    /* Make more room if needed */
    if (enlargeStringInfo(str, datalen) != EXEC_SUCCESS)
    {
        return EXEC_FAIL;
    }

    /* OK, append the data */
    memcpy(str->data + str->len, data, datalen);
    str->len += datalen;

    str->data[str->len] = '\0';

    return EXEC_SUCCESS;
}

int appendBinaryStringInfoNT(StringInfo str, const char *data, int datalen)
{
    if (NULL == str || NULL == data || datalen < 0)
    {
        return EXEC_FAIL;
    }

    /* Make more room if needed */
    if (enlargeStringInfo(str, datalen) != EXEC_SUCCESS)
    {
        return EXEC_FAIL;
    }
    /* OK, append the data */
    memcpy(str->data + str->len, data, datalen);
    str->len += datalen;

    return EXEC_SUCCESS;
}

int enlargeStringInfo(StringInfo str, int needed)
{
    int            newlen;

    if (needed < 0)                /* should not happen */
    {
        fprintf(stderr, "invalid string enlargement request size: %d", needed);
        return EXEC_FAIL;
    }
    if (((size_t) needed) >= (MaxAllocSize - (size_t) str->len))
    {
        fprintf(stderr, "Cannot enlarge string buffer containing %d bytes by "
                        "%d more bytes.", str->len, needed);
        return EXEC_FAIL;
    }

    needed += str->len + 1;        /* total space required now */

    if (needed <= str->maxlen)
        return;                    /* got enough space already */

    newlen = 2 * str->maxlen;
    while (needed > newlen)
        newlen = 2 * newlen;

    if (newlen > (int) MaxAllocSize)
        newlen = (int) MaxAllocSize;

    if (NULL == (str->data = (char *) realloc(str->data, newlen)))
    {
        return EXEC_FAIL;
    }

    str->maxlen = newlen;

    return EXEC_SUCCESS;
}

size_t pvsnprintf(char *buf, size_t len, const char *fmt, va_list args)
{
    int            nprinted;

    assert(len > 0);

    errno = 0;

    /*
     * Assert check here is to catch buggy vsnprintf that overruns the
     * specified buffer length.  Solaris 7 in 64-bit mode is an example of a
     * platform with such a bug.
     */
#ifdef USE_ASSERT_CHECKING
    buf[len - 1] = '\0';
#endif

    nprinted = vsnprintf(buf, len, fmt, args);

    assert(buf[len - 1] == '\0');

    /*
     * If vsnprintf reports an error other than ENOMEM, fail.  The possible
     * causes of this are not user-facing errors, so elog should be enough.
     */
    if (nprinted < 0 && errno != 0 && errno != ENOMEM)
    {
        fprintf(stderr, "vsnprintf failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*
     * Note: some versions of vsnprintf return the number of chars actually
     * stored, not the total space needed as C99 specifies.  And at least one
     * returns -1 on failure.  Be conservative about believing whether the
     * print worked.
     */
    if (nprinted >= 0 && (size_t) nprinted < len - 1)
    {
        /* Success.  Note nprinted does not include trailing null. */
        return (size_t) nprinted;
    }

    if (nprinted >= 0 && (size_t) nprinted > len)
    {
        /*
         * This appears to be a C99-compliant vsnprintf, so believe its
         * estimate of the required space.  (If it's wrong, the logic will
         * still work, but we may loop multiple times.)  Note that the space
         * needed should be only nprinted+1 bytes, but we'd better allocate
         * one more than that so that the test above will succeed next time.
         *
         * In the corner case where the required space just barely overflows,
         * fall through so that we'll error out below (possibly after
         * looping).
         */
        if ((size_t) nprinted <= MaxAllocSize - 2)
            return nprinted + 2;
    }

    /*
     * Buffer overrun, and we don't know how much space is needed.  Estimate
     * twice the previous buffer size, but not more than MaxAllocSize; if we
     * are already at MaxAllocSize, choke.  Note we use this palloc-oriented
     * overflow limit even when in frontend.
     */
    if (len >= MaxAllocSize)
    {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }

    if (len >= MaxAllocSize / 2)
        return MaxAllocSize;

    return len * 2;
}

int freeStringInfo(StringInfo str)
{
    if (NULL == str)
    {
        return EXEC_FAIL;
    }

    if (NULL != str->data)
    {
        free(str->data);
    }

    free(str);

    return EXEC_SUCCESS;
}

int main()
{
    StringInfo str = makeStringInfo();
    appendStringInfo(str, "abcedf");

    freeStringInfo(str);

    return 0;
}
