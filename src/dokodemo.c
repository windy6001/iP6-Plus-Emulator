/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         dokodemo.c                      **/
/**                                                         **/
/** by Windy 2010                                           **/
/*************************************************************/
#pragma warning(disable:4996)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "P6.h"
#include "Z80.h"
#include "mem.h"
#include "device.h"
#include "d88.h"
#include "fdc.h"
#include "disk.h"

#include "dokodemo.h"


extern int P6Version;

static FILE *fp=NULL;


static struct tag {
	char key[MAX_DOKODEMO_KEY];
	char value[ MAX_DOKODEMO_VALUE];
} dokodemo_data[ MAX_DOKODEMO_LINE];

static int max_idx;



int dokodemo_save(char *path)
{
	int ret=0;
	max_idx=0;
	
	fp= fopen(path,"w");
	if( fp != NULL)
	{
		printf("dokodemo saving ...");
		dokodemo_putsection("[iP6 Plus DOKODEMO SAVE FILE]");
		dokodemo_putentry("Version",DOKODEMO_VERSION);
		
		// save status ....
		dokodemo_save_d88();
		dokodemo_save_device();
		dokodemo_save_disk();
		dokodemo_save_fdc();
		dokodemo_save_mem();
		dokodemo_save_z80();
		
		fclose(fp);
		fp=NULL;
		ret=1;
	}
	return ret;
}


int dokodemo_load(char *path)
{
	char key[256];
	char value[ MAX_DOKODEMO_VALUE];
	int ret=0;
	int i;
	int tmp;
	
	fp= fopen(path,"r");
	if( fp != NULL)
		{
		printf("dokodemo file open ...\n");

		for(i=0; i< MAX_DOKODEMO_LINE || !feof(fp) ; i++)
			{
			if( dokodemo_loadentry(key,value)) {
				strcpy( dokodemo_data[i].key ,key);
				strcpy( dokodemo_data[i].value ,value);
				printf("key '%s' value '%s' \n",key,value);
				}
			}
		max_idx =i;

		printf("max_idx %d\n",max_idx);
		if( !dokodemo_getentry( "Version" , value)) return 0;
		printf("dokodemo load  Version '%s'  \n",value);
		if( strcmp(dokodemo_data[1].value ,DOKODEMO_VERSION)==0) 
			{
			if( !dokodemo_getentry_int("P6Version" , &tmp)) return 0;
			printf("dokodemo load  P6Version %d   current P6Version %d \n",tmp, P6Version);
		//	if( tmp==P6Version)
				{
				printf("dokodemo loading ...");

				//  load status....
				if( tmp != P6Version)
					{
					ResetPC(1);
					}

				dokodemo_load_device();
				dokodemo_load_d88();

				dokodemo_load_disk();
				dokodemo_load_fdc();
				dokodemo_load_mem();
				dokodemo_load_z80();

				ret=1;
				}
			}
		fclose(fp);
		}
	return ret;
}

int dokodemo_getentry( char *key , char *value)
{
	int ret=0;
	int i;
	printf("dokodemo_getentry(): searching key '%s'  \n",key);
	for(i=0; i< max_idx; i++)
		{
		//printf("dokodemo_getentry(): key '%s'  value '%s' \n",dokodemo_data[i].key , dokodemo_data[i].value); 
		 if( strcmp( dokodemo_data[i].key ,key)==0)
			{
			strcpy(value , dokodemo_data[i].value);
			printf("FOUND!! \n");
			ret=1;
			break;
			}
		}
	if( !ret) printf("not found \n");
	return ret;
}

int dokodemo_getentry_int( char *key , int *value)
{
	int ret;
	char tmp[ MAX_DOKODEMO_VALUE];
	ret = dokodemo_getentry( key, tmp);
	*value = atoi(tmp);
	return ret;
}

int dokodemo_getentry_byte( char *key , byte *value)
{
	int ret;
	char tmp[ MAX_DOKODEMO_VALUE];
	ret = dokodemo_getentry( key, tmp);
	*value = atoi(tmp);
	return ret;
}

int dokodemo_getentry_word( char *key , word *value)
{
	int ret;
	char tmp[ MAX_DOKODEMO_VALUE];
	ret = dokodemo_getentry( key, tmp);
	//*value = atoi(tmp);
	sscanf(tmp,"%04hX",value);		// hex -> word
	return ret;
}

int dokodemo_getentry_buffer( char *key , char *value ,int size)
{
	int i,j;
	int ret;
	char keybuff[ MAX_DOKODEMO_KEY];
	char valuebuff[ MAX_DOKODEMO_VALUE];
	char tmpstr[3];
	int  tmp;
	int  adr=0;
	char *in;
	char *out;
	
	int step= DOKODEMO_STEP;
	
	
	out = value;
	for(i=0; i<size; i+=step)
		{
		in = valuebuff;
		sprintf( keybuff,"%s_%04X",key, adr & 0xffff);
		ret = dokodemo_getentry( keybuff, valuebuff);
		for(j=0; j< step; j++)
			{
			tmpstr[0]=in[0]; tmpstr[1]= in[1]; tmpstr[2]=0;  
			sscanf(tmpstr,"%02X",&tmp);		// hex -> integer
			*out = tmp;
			
			in+=2;
			out++;

			if( i+j+1 >=size) break;
			}
		adr+= step;
		}
	return ret;
}

int dokodemo_putsection( char *value)
{
	int ret=0;
	if(fp !=NULL)
	{
		fprintf(fp ,"%s\n",value);
		max_idx++;
		ret=1;
	}
	return ret;
}

int dokodemo_putentry( char *key , char *value)
{
	int ret=0;
	if(fp !=NULL)
	{
		fprintf(fp ,"%s=%s\n", key ,value);
		max_idx++;
		ret=1;
	}
	return ret;
}


int dokodemo_putentry_int( char *key , int value)
{
	int ret;
	char buff[ MAX_DOKODEMO_VALUE];
	
	sprintf( buff,"%d",value);
	ret = dokodemo_putentry( key ,buff);
	return ret;
}

int dokodemo_putentry_byte( char *key , byte value)
{
	int ret;
	char buff[ MAX_DOKODEMO_VALUE];
	
	sprintf( buff,"%d",value);
	ret = dokodemo_putentry( key ,buff);
	return ret;
}

int dokodemo_putentry_word( char *key , word value)
{
	int ret;
	char buff[ MAX_DOKODEMO_VALUE];
	
	sprintf( buff,"%04X",value);
	ret = dokodemo_putentry( key ,buff);
	return ret;
}


int dokodemo_putentry_buffer(char *key, char *value ,int size)
{
	int  i,j;
	int ret=0;
	int adr=0;
	char keybuff[256];
	char valuebuff[ MAX_DOKODEMO_VALUE];
	char tmp[3];
	char *in;
	
	int  step= DOKODEMO_STEP;
	
	
	in= value;
	
	keybuff[0]=0;
	for(i=0; i< size; i+=step )
		{
		valuebuff[0]=0;
		sprintf(keybuff,"%s_%04X",key ,(adr & 0xffff));
		for(j=0; j< step; j++)
			{
			sprintf(tmp,"%02X", (*in & 0xff));
			strcat(valuebuff, tmp);
			in++;
			if( i+j+1 >=size) break;
			}
		dokodemo_putentry( keybuff, valuebuff);
		adr+=step;
		}
	
	return ret;
}

int dokodemo_loadentry( char *key, char *value)
{
	static char buff[ MAX_DOKODEMO_VALUE+1];
	int ret =0; 
	char *p;
	
	if(fp !=NULL)
	{
		if(fgets( buff, MAX_DOKODEMO_VALUE , fp))
		{
			int len= strlen(buff);
			if( buff[len -1] =='\n') buff[len-1] = 0;	// delete \n
			
			printf("loadentry(): buff '%s' \n",buff);
			p = strchr( buff ,'=');		//separate Cstring by '='
			if( p!=NULL) {*p=0;}
			strncpy(  key  , buff , MAX_DOKODEMO_KEY);			
			if( p!=NULL) {strncpy( value,  p+1  , MAX_DOKODEMO_VALUE);}
			
			printf("loadentry(): key '%s'  value '%s'\n",key,value);
			ret=1;
		}
	}
	return ret;
}
