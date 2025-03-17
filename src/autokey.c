/*
name is autokey.c

Copyright (c) 2020 Windy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

based on https://github.com/windy6001/ROMAJI/
*/


#include <string.h>
#include "buffer.h"

static RINGBUFFER* autokeybuffer;
static int _isAutoKey;          // TRUE: autokey doing

int initAutokey(void)
{
    autokeybuffer = ringbuffer_Open(10);
    _isAutoKey = FALSE;
}

int putAutokey(unsigned char data)
{
    ringbuffer_Put(autokeybuffer, data);
    _isAutoKey = TRUE;
}

int getAutokey(unsigned char *data)
{
    int ret = FALSE;
    ret = ringbuffer_Get(autokeybuffer, data);
    if (!ret || autokeybuffer->datas ==0) {
        _isAutoKey = FALSE;
    }
    return ret;
}


int putAutokeyMessage(unsigned char* data)
{
    for (size_t i = 0; i < strlen(data); i++) {
        ringbuffer_Put(autokeybuffer, data[i]);
    }

    _isAutoKey = TRUE;
}

void exitAutokey(void)
{
    ringbuffer_Close(autokeybuffer);
    _isAutoKey = 0;
}

int isAutokey(void)
{
    return _isAutoKey;
}
