/*
** $Id: rwops.c 10660 2008-08-14 09:30:39Z weiym $
**
** rwops.c: 
**  Functions for reading and writing data from general sources,
**      such as file, memory, etc. 
**
** Copyright (C) 2003 ~ 2008 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** All rights reserved by Feynman Software.
** 
** The idea and most code come from SDL - Simple DirectMedia Layer
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/12/14
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "endianrw.h"

#define MGUI_Error(x)

/* This file provides a general interface for SDL to read and write
   data sources.  It can easily be extended to files, memory, etc.
*/

/* Functions to read/write stdio file pointers */

static int stdio_seek(MG_RWops *context, int offset, int whence)
{
    if ( fseek(context->hidden.stdio.fp, offset, whence) == 0 ) {
        return(ftell(context->hidden.stdio.fp));
    } else {
        MGUI_Error(MGUI_EFSEEK);
        return(-1);
    }
}

static int stdio_read(MG_RWops *context, void *ptr, int size, int maxnum)
{
    size_t nread;

    nread = fread(ptr, size, maxnum, context->hidden.stdio.fp); 
    if ( nread == 0 && ferror(context->hidden.stdio.fp) ) {
        MGUI_Error(MGUI_EFREAD);
    }
    return(nread);
}
static int stdio_write(MG_RWops *context, const void *ptr, int size, int num)
{
    size_t nwrote;

    nwrote = fwrite(ptr, size, num, context->hidden.stdio.fp);
    if ( nwrote == 0 && ferror(context->hidden.stdio.fp) ) {
        MGUI_Error(MGUI_EFWRITE);
    }
    return(nwrote);
}

#ifdef _MGUSE_OWN_STDIO
static int stdio_ungetc(MG_RWops *context, unsigned char c)
{
    return ungetc (c, context->hidden.stdio.fp);
}
#endif

static int stdio_close(MG_RWops *context)
{
    if ( context ) {
        if ( context->hidden.stdio.autoclose ) {
            /* WARNING:  Check the return value here! */
            fclose(context->hidden.stdio.fp);
        }
        free(context);
    }
    return(0);
}
static int stdio_eof(MG_RWops *context)
{
    return feof (context->hidden.stdio.fp);
}

/* Functions to read/write memory pointers */

static int mem_seek(MG_RWops *context, int offset, int whence)
{
    Uint8 *newpos;

    switch (whence) {
        case SEEK_SET:
            newpos = context->hidden.mem.base+offset;
            break;
        case SEEK_CUR:
            newpos = context->hidden.mem.here+offset;
            break;
        case SEEK_END:
            newpos = context->hidden.mem.stop+offset;
            break;
        default:
            _MG_PRINTF ("MISC>RWOps: Unknown value for 'whence'\n");
            return(-1);
    }
    if ( newpos < context->hidden.mem.base ) {
        newpos = context->hidden.mem.base;
    }
    if ( newpos > context->hidden.mem.stop ) {
        newpos = context->hidden.mem.stop;
    }
    context->hidden.mem.here = newpos;
    return(context->hidden.mem.here-context->hidden.mem.base);
}
static int mem_read(MG_RWops *context, void *ptr, int size, int maxnum)
{
    int num;

    num = maxnum;
    if ( (context->hidden.mem.here + (num*size)) > context->hidden.mem.stop ) {
        num = (context->hidden.mem.stop-context->hidden.mem.here)/size;
    }
    memcpy(ptr, context->hidden.mem.here, num*size);
    context->hidden.mem.here += num*size;
    return(num);
}
static int mem_write(MG_RWops *context, const void *ptr, int size, int num)
{
    if ( (context->hidden.mem.here + (num*size)) > context->hidden.mem.stop ) {
        num = (context->hidden.mem.stop-context->hidden.mem.here)/size;
    }
    memcpy(context->hidden.mem.here, ptr, num*size);
    context->hidden.mem.here += num*size;
    return(num);
}
#ifdef _MGUSE_OWN_STDIO
static int mem_ungetc(MG_RWops *context, unsigned char c)
{
    if (context->hidden.mem.here > context->hidden.mem.base) {
        context->hidden.mem.here--;
        *context->hidden.mem.here = c;
        return c;
    }
    
    return EOF;
}
#endif

static int mem_close(MG_RWops *context)
{
    if ( context ) {
        free(context);
    }
    return(0);
}
static int mem_close_none (MG_RWops *context)
{
    return(0);
}
static int mem_eof(MG_RWops *context)
{
    if (context->hidden.mem.here >= context->hidden.mem.stop)
        return 1;
    return 0;
}

/* Functions to create MG_RWops structures from various data sources */

#ifdef macintosh
/*
 * translate unix-style slash-separated filename to mac-style colon-separated
 * name; return malloced string
 */
static char *unix_to_mac(const char *file)
{
    int flen = strlen(file);
    char *path = malloc(flen + 2);
    const char *src = file;
    char *dst = path;
    if(*src == '/') {
        /* really depends on filesystem layout, hope for the best */
        src++;
    } else {
        /* Check if this is a MacOS path to begin with */
        if(*src != ':')
            *dst++ = ':';   /* relative paths begin with ':' */
    }
    while(src < file + flen) {
        const char *end = strchr(src, '/');
        int len;
        if(!end)
            end = file + flen; /* last component */
        len = end - src;
        if(len == 0 || (len == 1 && src[0] == '.')) {
            /* remove repeated slashes and . */
        } else {
            if(len == 2 && src[0] == '.' && src[1] == '.') {
                /* replace .. with the empty string */
            } else {
                memcpy(dst, src, len);
                dst += len;
            }
            if(end < file + flen)
                *dst++ = ':';
        }
        src = end + 1;
    }
    *dst++ = '\0';
    return path;
}
#endif /* macintosh */

MG_RWops *MGUI_RWFromFile(const char *file, const char *mode)
{
    FILE *fp;
    MG_RWops *rwops;

    rwops = NULL;

#ifdef macintosh
    {
        char *mpath = unix_to_mac(file);
        fp = fopen(mpath, mode);
        free(mpath);
    }
#else
    fp = fopen(file, (char*)mode);
#endif
    if ( fp == NULL ) {
        _MG_PRINTF ("MISC>RWOps: Couldn't open %s\n", file);
    } else {
        rwops = MGUI_RWFromFP(fp, 1);
    }

    return(rwops);
}

MG_RWops *MGUI_RWFromFP(FILE *fp, int autoclose)
{
    MG_RWops *rwops;

    rwops = MGUI_AllocRW();
    if ( rwops != NULL ) {
        rwops->seek = stdio_seek;
        rwops->read = stdio_read;
        rwops->write = stdio_write;
#ifdef _MGUSE_OWN_STDIO
        rwops->ungetc = stdio_ungetc;
#endif
        rwops->close = stdio_close;
        rwops->eof   = stdio_eof;
        rwops->hidden.stdio.fp = fp;
        rwops->hidden.stdio.autoclose = autoclose;

        rwops->type = RWAREA_TYPE_STDIO;
    }
    return(rwops);
}

MG_RWops *MGUI_RWFromMem(void *mem, int size)
{
    MG_RWops *rwops;

    rwops = MGUI_AllocRW();
    if ( rwops != NULL ) {
        rwops->seek = mem_seek;
        rwops->read = mem_read;
        rwops->write = mem_write;
#ifdef _MGUSE_OWN_STDIO
        rwops->ungetc = mem_ungetc;
#endif
        rwops->close = mem_close;
        rwops->eof   = mem_eof;
        rwops->hidden.mem.base = (Uint8 *)mem;
        rwops->hidden.mem.here = rwops->hidden.mem.base;
        if ((0xFFFFFFFF - (Uint32)mem) < size)
            rwops->hidden.mem.stop = (void*)0xFFFFFFFF;
        else
            rwops->hidden.mem.stop = rwops->hidden.mem.base+size;

        rwops->type = RWAREA_TYPE_MEM;
    }
    return(rwops);
}

void MGUI_InitMemRW (MG_RWops* area, void *mem, int size)
{
    area->seek = mem_seek;
    area->read = mem_read;
    area->write = mem_write;
#ifdef _MGUSE_OWN_STDIO
    area->ungetc = mem_ungetc;
#endif
    area->close = mem_close_none;
    area->eof   = mem_eof;
    area->hidden.mem.base = (Uint8 *)mem;
    area->hidden.mem.here = area->hidden.mem.base;
    if ((0xFFFFFFFF - (Uint32)mem) < size)
        area->hidden.mem.stop = (void*)0xFFFFFFFF;
    else
        area->hidden.mem.stop = area->hidden.mem.base+size;

    area->type = RWAREA_TYPE_MEM;
}

MG_RWops *MGUI_AllocRW(void)
{
    MG_RWops *area;

    area = (MG_RWops *)malloc(sizeof *area);
    if ( area == NULL ) {
        _MG_PRINTF ("MISC>RWOps: Out of memory\n");
    }
    else
        area->type = RWAREA_TYPE_UNKNOWN;

    return(area);
}

void MGUI_FreeRW(MG_RWops *area)
{
    free(area);
}

int MGUI_RWgetc (MG_RWops* area)
{
    unsigned char c;

    if (MGUI_RWread (area, &c, 1, 1) == 0)
        return EOF;

    return c;
}


