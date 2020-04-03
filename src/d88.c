/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           d88.c                         **/
/**                                                         **/
/**                    access to d88 format                 **/
/** by Windy                                                **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "types.h"
#include "P6.h"
#include "mem.h"
#include "Build.h"
#include "Option.h" // for ConfigWrite
#include "d88.h"
#include "message.h"
#include "dokodemo.h"

#include "os.h"


#define MAXTRACK  164
#define MAXSECTOR  16
#define MAXDATA   256


// ****************************************************************************
//           d88 ファイル　ヘッダー
// 0: ドライブ１　　1:ドライブ２　　2:新規ディスク用
// ****************************************************************************
struct _head {
  char diskname[17];
  char reserve[9];
  char writeprotect;
  char disktype;
  unsigned int disksize;
} d88head[3];

unsigned int d88track[3][ MAXTRACK];


// ****************************************************************************
//           d88 セクター　ヘッダー
// ****************************************************************************
struct _sector {
  char  c;
  char  h;
  char  r;
  char  n;
  short sectors;
  char  mitudo;
  char  deleted;
  char  status;
  char  reserve[5];
  short size;
 } d88sector;

// ****************************************************************************
//           Endian Swaper
// ****************************************************************************
#ifdef WORDS_BIGENDIAN
#define SWAPDW(val) ((val & 0x000000ff) << 24 | \
				     (val & 0x0000ff00) << 8  |  \
				     (val & 0x00ff0000) >> 8  |  \
				     (val & 0xff000000)>> 24)

#define SWAPW(val)  ((val & 0x00ff) << 8 | \
				     (val & 0xff00) >> 8)
#else
#define SWAPDW(val) (val)
#define SWAPW(val)  (val)
#endif

// ****************************************************************************
//           internal variable
// ****************************************************************************
static int typetrack[2];	// 1: 1DDitt      2: Ditt 
static int typebad[2];		// 1: bad format  0: normal format
static int issys[2]={-1,-1};	// 1: system disk 0: other disk
static int isprotect[2];   // 1: write protect 0: writable 

void dokodemo_save_d88(void)
{
	dokodemo_putsection("[D88]");
	DOKODEMO_PUTENTRY_INT(typetrack[0]);
	DOKODEMO_PUTENTRY_INT(typetrack[1]);
	DOKODEMO_PUTENTRY_INT(typebad[0]);
	DOKODEMO_PUTENTRY_INT(typebad[1]);
	DOKODEMO_PUTENTRY_INT(issys[0]);
	DOKODEMO_PUTENTRY_INT(issys[1]);
	DOKODEMO_PUTENTRY_INT(isprotect[0]);
	DOKODEMO_PUTENTRY_INT(isprotect[1]);
}

void dokodemo_load_d88(void)
{
	DOKODEMO_GETENTRY_INT(typetrack[0]);
	DOKODEMO_GETENTRY_INT(typetrack[1]);
	DOKODEMO_GETENTRY_INT(typebad[0]);
	DOKODEMO_GETENTRY_INT(typebad[1]);
	DOKODEMO_GETENTRY_INT(issys[0]);
	DOKODEMO_GETENTRY_INT(issys[1]);
	DOKODEMO_GETENTRY_INT(isprotect[0]);
	DOKODEMO_GETENTRY_INT(isprotect[1]);
}

// ****************************************************************************
//           internal function
// ****************************************************************************
void chk_tracktable(int drive);



// ****************************************************************************
//           read_d88head: 
//             d: drive 番号
// ****************************************************************************
int read_d88head(int drive)
{
	char *msg1D  = MSG_1D_FDD; //  MSG_1D_FDD;
	char *msg1DD = MSG_NO_1DD;//  MSG_NO_1DD;


	if(Verbose) printf("Reading  header of D88 file..... ");
	if(DskStream[drive]) 
		{
		rewind( DskStream[drive]);
		if( fread( &d88head[drive] , sizeof(d88head[drive]) ,1,DskStream[drive])!=1) { printf("read error d88head"); return(-1);}
		if( fread( &d88track[drive], sizeof(d88track[drive]),1,DskStream[drive])!=1) { printf("read error d88track");return(-1);}


		{
		int i;
		d88head[drive].disksize  = SWAPDW( d88head[drive].disksize );
		for(i=0; i< MAXTRACK ; i++)
			{
			d88track[drive][i]= SWAPDW( d88track[drive][i] );
			}
		}

#if 0
		// ----------- if PC-6001mk2 and PC-6601 and 1DD then error -------------
		if(( P6Version== PC60M2  || P6Version == PC66 )&& is1DD(drive))
			{ 
			fclose( DskStream[drive]); DskStream[drive]=NULL; *DskName[drive]=0;
			printf( "%s",msg1DD);

			ConfigWrite();
			OSD_MessageBox( msg1DD , PROGRAM_NAME" floppy disk information",OSDR_OK);
                  /* messagebox や、ConfigWrite はここで使うべきか？ */
			}
	/* SRでの、1D format は、1D->1DD コンバートに必要なので、警告だけにしておく */
      	// ----------- if PC-6601SR and  1D then error --------------------------
		if(  P6Version== PC66SR  && !is1DD(drive) && !isSYS(drive)      )
			{
			printf( "%s",msg1D );

			OSD_MessageBox( msg1D , PROGRAM_NAME" floppy disk information",OSDR_OK);
			}
#endif
		chk_tracktable(drive);
		}

	if(Verbose && DskStream[drive]) printf("Ok\n");
	return(0);
}




// ****************************************************************************
//           chk_tracktable
// ****************************************************************************
void chk_tracktable(int drive)
{
	int i;

	// ----------- check 1DDitt or Ditt? -----------------
	for(i=0; i< 10; i+=2)
		{
		if( d88track[drive][i+0]!=0 && d88track[drive][i+1]==0)
			{
			typetrack[drive]=2;
			}
		else
			{
			typetrack[drive]=1;
			}
		}
	
	if( typetrack[drive]==2)
		printf("Ok\nThis disk seems to made by Ditt \n");
	else
		printf("Ok\nThis disk seems to made by 1DDitt \n");

 // ----------- bad track table? -----------------

	typebad[drive]=0;
	if( d88track[drive][0]==0x2b0 && d88track[drive][1]==0 && 
     d88track[drive][2]==0x3c0 && d88track[drive][3]==0 && d88track[drive][4]==0x4d0)
		{
		printf("Reparing disk track table ....");
		typebad[drive]=1; typetrack[drive]=2;
		for(i=0; i< 35*2 ; i+=2)	// for 1D disk
			{
			d88track[drive][i+0]= 0x2b0+ (i/2)*(256+16)*16;	// set correct track table
			d88track[drive][i+1]= 0;
			}
//    printf("Done.\n");
	}

#ifdef DEBUG2
	 for(i=0; i< 40*2 ; i++)
 		{
		 printf("%04X ",d88track[drive][i]);
		}
  printf("typetrack =%d typebad=%d  \n",typetrack[drive],typebad[drive]);
#endif
}


// ****************************************************************************
//           seek_d88
// ****************************************************************************
int seek_d88(int drive, int track, int sector)
{
	 int found=0;
	 int i;
	 unsigned int adr;

	// PRINTDEBUG4(DSK_LOG,"[d88][seek_d88]: track=%2d ,sector=%2d  ,adr=%06lX  \n",track,sector,adr);
//	if( track ==35 ) 
//		printf("nya---nn (^^/ ");


	/* -------------- seek   track --------------- */
	 adr= d88track[drive][ track* typetrack[drive]];

	 if(!adr) 
		 {printf("[d88][seek_d88]: ** seek error **  adr=%04X typetrack=%d track=%d sector=%d \n",adr ,typetrack[drive] ,track ,sector);Trace=1; return 0;}
 	 if( fseek(DskStream[drive], adr, SEEK_SET)!=0)
	   	  {
	   	   printf("[d88][seek_d88]: ** Seek error **  invalid adr   track=%d adr=%4X  \n",track ,adr);
	   	   return(0);
	   	  }

	/* -------------- search sector --------------- */
	 i=0;
	 do  {
		 if( fread( &d88sector ,sizeof(d88sector),1, DskStream[drive])!=1)
        	  {
        	   printf("[d88][seek_d88]: ** read error **  C=%d  adr=%4lX \n",track ,adr);
        	   return(0);
        	  }

	  d88sector.sectors = SWAPW ( d88sector.sectors );
	  d88sector.size    = SWAPW ( d88sector.size );

		//printf("c=%d h=%d r=%d n=%d\n",d88sector.c,d88sector.h,d88sector.r,d88sector.n);

		 if( /*track == d88sector.c  && */ sector ==d88sector.r + typebad[drive]) // chrn のcがあてにならないときがあるので外す。
			{
			 fseek(DskStream[drive], 0,SEEK_CUR);	// for fwrite
			 found =1;
		  // printf("[d88][seek_d88]:  sector found! c=%d,h=%d,r=%d,n=%d\n",	d88sector.c ,d88sector.h,d88sector.r, d88sector.n);
			 break;
			}
	 	 if( fseek(DskStream[drive], d88sector.size, SEEK_CUR)!=0)
	   	     {
	   	      printf("[d88][seek_d88]: ** fseek error ** track=%d  adr=%4X\n",track ,adr); 
	   	      return(0);
	   	     }
	   	  i++;
         }
	 while(i< d88sector.sectors);
	 if(!found) printf("[d88][seek_d88]: ** sector not found ** track (C)=%d sector (R)=%d  max=%d\n",track,sector , d88sector.sectors);
	 return( found);
}

// ****************************************************************************
//           read_d88sector:
// ****************************************************************************
// input
//   len: input length
byte* read_d88sector(int len, int drive ,int track ,int sector)
{
		
    int i=0;
    int sectors;
    byte *p;
    static byte buff[ 20*256];

	int  size=0;

    memset(buff,0,sizeof(buff));
    if( DskStream[drive])
       {
        p=buff;
        if( !seek_d88(drive,track,sector+i+1))
           	   {
           	    printf("read_d88sector(): seek error track=%d,sector=%d\n",track,sector+i+1); 
           	    return(buff);
           	    }
        
        sectors= len/ (d88sector.size / 0x100);	// 読み込みセクター数 求める
		// printf("len=%d, size=%d sectors=%d\n",len ,d88sector.size, sectors);
		for(i=0; i<sectors; i++)
		 {
		// printf("** seek_d88 c=%d h=%d r=%d\n",track,0,sector+i+1);
           if( !seek_d88(drive,track,sector+i+1))
           	   {
           	    printf("read_d88sector(): seek error track=%d,sector=%d\n",track,sector); 
           	    break;
           	    }
		// printf("read data\n");
           if( fread( p , d88sector.size ,1,DskStream[drive])!=1)
               { 
               printf("read_d88sector(): Read error c=%d \n",track);
               break;
               }
           p   += d88sector.size;
		   size+= d88sector.size;
		   if( size > 19*256) break;	// against buffer-overflow! 2003/1/5
          }
       }

    //printf("read data=");
    //for(int i=0; i<19*256; i++)
    //{
    //    PRINTDEBUG1(DSK_LOG,"%02X ",buff[i]);
    //}

   return( buff);
//       dump( buff ,sizeof(sectordata[c][r]));
}


// ****************************************************************************
//           write_d88sector:
// ****************************************************************************
// write Data * kai
int write_d88sector(int len, int drive ,int track ,int sector, byte *buff)
{
    int i=0;
    int sectors;
    byte *p;
//    long adr;

    if( DskStream[drive]) 
		{
        p=buff;
            
        if( !seek_d88(drive,track,sector+i+1))
           	   {
           	    printf("write_d88sector(): seek error track=%d,sector=%d\n",track,sector); 
           	    return(0);
           	    }
        
        sectors= len/ (d88sector.size / 0x100);	// 書き込みセクター数 求める
	//  printf("len=%d, size=%d sectors=%d\n",len ,d88sector.size, sectors);
		for(i=0; i<sectors; i++)
			{
		// printf("** seek_d88 c=%d h=%d r=%d\n",track,0,sector+i+1);
           if( !seek_d88(drive,track,sector+i+1))
           	   {
           	    printf("[d88.c][write_d88sector]: seek error track=%d,sector=%d\n",track,sector); 
           	    break;
           	    }
		// printf("write data\n");
           if( fwrite( p , d88sector.size ,1,DskStream[drive])!=1)
               { 
               printf("[d88.c][write_d88sector]: Write error c=%d \n",track);
               break;
               }
           p+= d88sector.size;
         }
            
            
#if 0
            PRINTDEBUG4(" (WRITE: track=%2d ,sector=%2d ,adr=%06lX ) \n",track,sector+1,adr);
           for(i=0; i< len ; i++)		// kaisuu kurikaesu
             {
              if( !seek_d88(drive,track,sector+i+1))
              	   { printf("write_d88sector(): seek failed \n"); return(-1);}
              if( fwrite( p , d88sector.size,1,DskStream[0])!=1)
                   { printf("write_d88sector(): Write error c=%d \n",track); return(-1);}
              p+= d88sector.size;
           }
#endif
        }
    return(0);
}


// ****************************************************************************
//           is1DD: ディスクが、1DDかどうか
// ****************************************************************************
int is1DD(int drive)
{
 int ret;

 ret=0;
 switch ( d88head[drive].disktype )
	{
	case 0x00: ret=0; break; // 2D -->  1D  (by Ditt)
    case 0x10: ret=1; break; // 2DD--> 1DD  (by Ditt)
	case 0x30: ret=0; break; // 1D          (by 1DDitt)
    case 0x40: ret=1; break; // 1DD         (by 1DDitt)
    default:   ret=-1; printf("d88.c : Invalid disk type =%d\n",d88head[drive].disktype); break;
	}
 return( ret);    // 1: 1DD    0: 1D
}

// ****************************************************************************
//           isSYS: ディスクの先頭 (Track 0  Sector 1)に、SYS が書かれているか？
// ****************************************************************************
int isSYS(int drive)
{
	int len=1;
	int track =0;
	int sector =0;
 // ------------- system disk? -----------------------
 if(issys[drive]==-1)
	 issys[drive]= !strncmp( (char*)read_d88sector(len,drive,track,sector),"SYS",3);	

 return(issys[drive]);
}

// ****************************************************************************
//           setDiskprotect
//    1: write protected   0: normal		// 2003/1/21
// ****************************************************************************
void setDiskProtect(int drive, int Value)
{
 isprotect[drive]= Value;
}

// ****************************************************************************
//           getDiskprotect
//     1: write protected   0: normal
// ****************************************************************************
int getDiskProtect(int drive)
{
 return( isprotect[drive]);
}



// ****************************************************************************
//           create a new d88 file
// In: type:  0  1D
//            1  1DD
// ****************************************************************************
int createNewD88(char *path , int type)
{
	int  drive=2;
    FILE *fp;
    int  maxtrack;
    int  maxsector=  16;
    int  sectorsize=256;
    int  filldata;
    int  reservetrack;
	int  disktype;
    int  c,h,r,n;
    int  i;

    switch( type)
        {
         case 0: maxtrack=35; reservetrack=18; disktype=0x00; break;   // 1D
         case 1: maxtrack=80; reservetrack=37; disktype=0x10; break;   // 1DD
         default: return(0);
        }

	if(Verbose) printf("Creating a new file (%s) ...",path);
    fp= fopen( path ,"w+b");
    if( fp==NULL) {if(Verbose) printf("FAILED"); return(0);}
	if(Verbose) printf("OK\nFormatting %s ...",path);

    memset( &d88head[drive], 0,sizeof(d88head[drive] ));
    d88head[drive].disktype= disktype;
    d88head[drive].disksize= SWAPDW( 0x2b0+ (maxtrack)*(256+16)*16 );

    memset( &d88track[drive] , 0,sizeof(d88track));
    for(i=0; i< maxtrack*2 ; i+=2)
 		{
		 d88track[drive][i+0]= SWAPDW( 0x2b0+ (i/2)*(256+16)*16 );	// set correct track table
		 d88track[drive][i+1]= 0;
		}

    fwrite( &d88head[drive]  , sizeof(d88head[drive]) ,1,fp);
    fwrite( &d88track[drive] , sizeof(d88track[drive]),1,fp);
    
    h=0; n=1;
    memset( &d88sector, 0,sizeof(d88sector));
    for( c=0; c< maxtrack; c++)
        for( r=1; r<=16; r++)
            {
             d88sector.c=c; d88sector.h=h; d88sector.r=r; d88sector.n=n;
             d88sector.sectors= SWAPW(maxsector );
             d88sector.size   = SWAPW(sectorsize);
             fwrite( &d88sector , sizeof(d88sector),1,fp);

             
             for(i=0; i< sectorsize; i++)
                {
                 filldata= 0xff;
                 if( c== reservetrack)  // Reserve track?  (DIRECTORY / ID/ FAT)
                   {
                    if( r==13) {        // ID の作成
                         filldata=0x00;
                        }
                    else if(r>=14) {    // FAT の作成
                         if((i==reservetrack*2) || (i==reservetrack*2+1)) filldata=0xFE;
                        }
                   }
                 fputc( filldata ,fp);
                }
            }

    fclose(fp);
	if(Verbose) printf("OK\n");
	return(1);
}


#if 0
// ****************************************************************************
//           dump2
// ****************************************************************************
void dump2(char *buff,int length)
{
 int i,j;
 printf("------------------------------------------------------\n");
 for( i=0; i< length; i+=16)
   {
    printf("%04X :",i & 0xffff);
    for( j=0; j<16; j++)
      {
       char c= *(buff+i+j);
       printf("%02X ",c & 0xff);
      }
    for( j=0; j<16; j++)
      {
       char c= *(buff+i+j);
       printf("%c",c <0x20 ? 0x2e: c);
      }
    printf("\n");
//    if( i ==0x80|| i==0xf8) { getch();}
   }
}
#endif
