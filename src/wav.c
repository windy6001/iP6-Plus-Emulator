/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           wav.c                         **/
/**                                                         **/
/** by Windy 2003                                           **/
/*************************************************************/
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "wav.h"


#define ATODW(v)  ((((*(v+3))&0xff)<<24)|(((*(v+2))&0xff)<<16)|(((*(v+1))&0xff)<<8)|(*v&0xff))
#define ATOW(v)   (((*(v+1))&0xff<<8)|(*v&0xff))




int loadWav( char *path ,struct _spec *spec, short **out_buff , int *len)
{
	int  ret =0;
	FILE *fp;
	char buff[5];
	
	fp = fopen( path,"rb");
	if( fp !=NULL)
		{
		fread( buff,4 , 1,fp);
		if( strncmp( buff, "RIFF",4)==0)
			{
			 fread( buff,4 ,1,fp);  // size
			 fread( buff,4 ,1,fp);	 // WAVE header
			 if( strncmp( buff, "WAVE",4)==0)
				{
				 fread( buff,4 ,1,fp);  // fmt chk
				 if( strncmp( buff, "fmt ",4)==0)
					{
					 fread( buff, 4 ,1,fp);		// fmt chk size
					if( ATOW( buff) == 16 )
						{
						 fread( buff, 2 ,1,fp);		// format ID	
						 //if( ATOW( buff) == 1 )
							{
							 fread( buff, 2 ,1,fp); 				// channel
							 spec->channels = ATOW( buff);
							 fread( buff, 4 ,1,fp);				// sample rate
							 spec->freq = ATODW( buff); 
							 fread( buff, 4 ,1,fp);				// data speed
							 fread( buff, 2 ,1,fp);				// block size
							 fread( buff, 2 ,1,fp);				// sample bit
							 spec->samplebit = ATOW( buff); 
							 
							 fread(buff,4 ,1,fp);		// data chk
							 if( strncmp( buff,"data",4)==0)
								{
								 fread(buff,4,1,fp);		// data size
								 *len = ATODW( buff);
								 *out_buff = malloc( *len);
								 fread( *out_buff, *len ,1, fp);	// read wave data
								 ret = 1;
								}
							 
							}

						}
					}
				}
			}
		fclose(fp);
		}
	return ret;
}


void freeWav( char *buff)
{
	free( buff);
}



int getbuff( char *buff, int len ,FILE *fp, char *str)
{
	int ret=0;
	int c;
	do {
		c =fgetc(fp);
		if( c== str[0])
			{
			 buff[0]=c;
			 fgets( buff+1,3,fp);
			 if( strcmp( buff, str)==0)
				break; 
			}
	}while( !feof(fp));
	return ret;
}
