// Copy of AvxSynth's windowsPorts.h; it was missing the licensing
// header there, too.

#ifndef __WIN32_STRING_COMPAT_H__
#define __WIN32_STRING_COMPAT_H__

#include <stdarg.h>
#include <limits.h>
#include <time.h>
#include <avs/types.h>

char* _strrev(char *str);
char* _strupr(char *str);
char* _strlwr(char *str);


struct _finddata_t
{
	unsigned    attrib;
    time_t      time_create;    /* -1 for FAT file systems */
    time_t      time_access;    /* -1 for FAT file systems */
    time_t      time_write;
    uint64_t    size;
    char        name[PATH_MAX]; // originally name[260]
};

intptr_t _findfirst(const char *filespec, struct _finddata_t *fileinfo);

#endif // __WIN32_STRING_COMPAT_H__
