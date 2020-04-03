/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                       pngfile.h                         **/
/** modified by Windy 2004                                  **/
/*************************************************************/

#include "Video.h"

//int write_png(char *file_name, unsigned char *image, int Width, int Height,int depth);
int write_png( char * file_name , OSD_Surface *surface_in);
void set_png_palette(int i,byte r, byte g, byte b);
void set_png_maxpalette(int max);
