#include "berry.h"
#include "be_mem.h"
#include "be_sys.h"
#include <stdio.h>
#include <string.h>

/* Standard output - on ESP32 Arduino, stdout maps to UART0 (Serial) */

BERRY_API void be_writebuffer(const char *buffer, size_t length)
{
    be_fwrite(stdout, buffer, length);
}

BERRY_API char *be_readstring(char *buffer, size_t size)
{
    return be_fgets(stdin, buffer, (int)size);
}

/* Basic file I/O using standard C - works on ESP32 via VFS */

void *be_fopen(const char *filename, const char *modes)
{
    return fopen(filename, modes);
}

int be_fclose(void *hfile)
{
    return fclose(hfile);
}

size_t be_fwrite(void *hfile, const void *buffer, size_t length)
{
    return fwrite(buffer, 1, length, hfile);
}

size_t be_fread(void *hfile, void *buffer, size_t length)
{
    return fread(buffer, 1, length, hfile);
}

char *be_fgets(void *hfile, void *buffer, int size)
{
    return fgets(buffer, size, hfile);
}

int be_fseek(void *hfile, long offset)
{
    return fseek(hfile, offset, SEEK_SET);
}

long int be_ftell(void *hfile)
{
    return ftell(hfile);
}

long int be_fflush(void *hfile)
{
    return fflush(hfile);
}

size_t be_fsize(void *hfile)
{
    long int size, offset = be_ftell(hfile);
    fseek(hfile, 0L, SEEK_END);
    size = ftell(hfile);
    fseek(hfile, offset, SEEK_SET);
    return size;
}
