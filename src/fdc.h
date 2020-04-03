/*
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
*/

/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           fdc.h                         **/
/**                                                         **/
/*                     Internal disk unit                    */
/*************************************************************/
/* by windy */

void fdc_init(void);
void fdc_push_buffer( int port ,byte dat);
byte fdc_pop_buffer( int port);
byte fdc_inpDC(void);
void fdc_outDD(int Value);
byte fdc_inpDD(void);
void fdc_outDA(byte Value);

void dokodemo_save_fdc(void);
void dokodemo_load_fdc(void);
