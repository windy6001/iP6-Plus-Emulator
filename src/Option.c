/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Option.c                      **/
/** modified by Windy 2002                                  **/
/** This code is based on ISHIOKA Hiroshi 1998-2000         **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/
/* TO DO :  Win32 Ç©ÇÁíEãpÇ∑ÇÈÅB*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#if HAVE_DIRECT_H
#include <direct.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif


#include "os.h"

#include "P6.h"
#include "Sound.h"
#include "fm.h"
#include "Refresh.h"
#include "Help.h"
#include "Option.h"
#include "mem.h"
#include "Debug.h"
#include "device.h"

extern int SaveCPU;

#ifdef MITSHM
extern int UseSHM;
#endif

extern int isFullScr;

extern char *Title;


#define MAX_CONFIG_LINE 50

// ****************************************************************************
//          Internal variable
// ****************************************************************************
static FILE *configStream;
static char ConfigPath[PATH_MAX];	// config file path


// ****************************************************************************
//          CONFIG FILE OPTIONS
// ****************************************************************************
enum Config_defs {
D_CONFIG  ,  D_P6VERSION  , D_USESOUND , D_DISK_NUM , D_CPUCLOCK , D_SCALE , D_INTLAC,
D_CASPATH,   D_DSKPATH    , D_CASNAME   , D_DSKNAME , D_SCR4COL    , D_DRAWWAIT, D_FASTTAPE, 
D_UPERIOD ,  D_EXTKANJIROM, D_EXTRAM64 , D_SRLINE  ,
D_IMGPATH ,  D_MEMPATH    , D_EXTROMPATH   ,D_EXT1NAME      , D_EXT2NAME  , D_DUMMY,
D_CASPATHSAVE,  D_CASNAMESAVE ,
D_USESAVETAPEMENU , D_USESTATUSBAR, 
D_USEDISKLAMP ,
D_SOUND_RATE, D_PSG_VOL, D_FM_VOL,
D_ROMPATH ,
D_EXTRAM32,
D_DSKPATH2,
D_DSKNAME2,
D_ROMAJI_MODE,
D_USE_COMPATIBLEROM,
D_DEBUGWORKPATH,
D_PRNMODE,
};

char *ConfigOptions[]=
{ 
"[CONFIG]",  "P6Version=" , "UseSound=", "disk_num=", "CPUclock=", "scale=", "intLac=",
"CasPath=",  "DskPath=",    "CasName=",  "DskName=",   "scr4col=","drawWait=","FastTape=",
"UPeriod=",  "extkanjirom=","extram64=", "srline=",
"ImgPath=",  "MemPath=",    "ExtRomPath=" ,"Ext1Name=","Ext2Name=","dummy=",
"CasPath(save)=","CasName(save)=",
"UseSaveTapeMenu=","UseStatusBar=",
"UseDiskLamp=",
"sound_rate=","psg_volume=","fm_volume=",
"RomPath=",
"extram32=",
"DskPath2=",
"DskName2=",
"RomajiMode=",
"Use_CompatibleROM=",
"DebugWorkPath=",
"PrnMode=",
NULL
};




// ****************************************************************************
//          ConfigInit
// ****************************************************************************
void ConfigInit(void)
{
 char separator[10];
 ConfigPath[0]=0;

// if( _getcwd(ConfigPath, PATH_MAX)==NULL)
 if( OSD_GetModulePath(ConfigPath, PATH_MAX)==0)
	{
	// messagebox("_getcwd failed","");
    // printf("_getcwd failed\n");
	}
 if( ConfigPath[0] =='/')
     strcpy( separator, "/");    // unix style
 else
     strcpy( separator, "\\");   // msdos style 
 strcat(ConfigPath,separator);
 strcat(ConfigPath,"ip6kai.ini");
}


int ConfigDelete(void)
{
 int ret;
 printf(" Removing Config file '%s'.... ",ConfigPath);
 ret = remove( ConfigPath );
 if( ret==0)
 	printf(" OK \n");
 else
 	printf(" FAILED \n"); 
 return ret;
}


// ****************************************************************************
//          ConfigWrite
// ****************************************************************************
int ConfigWrite(void)
{
//#ifdef WIN32
// char tmpCasName[PATH_MAX],tmpDskName[ PATH_MAX];

 configStream= (FILE*)fopen( ConfigPath,"w");
 if(configStream==NULL) { OSD_MessageBox( "Cannot write config file ","" , OSDR_OK );return(0);}
    
 if(!WaitFlag) 	sw_nowait_mode(0);// NOWAIT???E?c?A?e?U?Anormal speed?E?s?ÅE?B2003/8/24   Thanks  Bernie

 fprintf(configStream,"%s\n",ConfigOptions[D_CONFIG]);
 fprintf(configStream,"%s%d\n",ConfigOptions[D_P6VERSION]  ,newP6Version);
#ifdef SOUND
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_USESOUND] ,newUseSound);
#else
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_USESOUND]  ,0);
#endif
 fprintf(configStream,"%s%d\n"  ,ConfigOptions[D_DISK_NUM]  ,new_disk_num);

 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_CPUCLOCK] ,CPUclock);
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_SCALE]        ,scale);
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_INTLAC]      ,IntLac);
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_CASPATH]   ,CasPath[0]);
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_DSKPATH]   ,DskPath[0]);
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_DSKPATH2]  ,DskPath[1]);
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_CASNAME]  ,CasName[0]);
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_DSKNAME]  ,DskName[0]);
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_DSKNAME2] ,DskName[1]);
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_SCR4COL]  ,scr4col);
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_DRAWWAIT] ,drawwait );
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_FASTTAPE]  ,FastTape );
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_UPERIOD]   ,UPeriod );
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_EXTKANJIROM] ,new_extkanjirom );
 fprintf(configStream,"%s%d\n", ConfigOptions[D_ROMAJI_MODE], romaji_mode);
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_EXTRAM64]   ,new_use_extram64 );
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_EXTRAM32]   ,new_use_extram32 );
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_SRLINE  ]    ,srline );
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_IMGPATH]  ,ImgPath );
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_MEMPATH]  ,MemPath );
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_EXTROMPATH]  ,ExtRomPath );
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_EXT1NAME] ,Ext1Name );
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_EXT2NAME] ,Ext2Name );


 fprintf(configStream,"%s%s\n"  ,ConfigOptions[D_CASPATHSAVE] ,CasPath[1]);
 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_CASNAMESAVE] ,CasName[1]);
 fprintf(configStream,"%s%d\n"  ,ConfigOptions[D_USESAVETAPEMENU] ,UseSaveTapeMenu);
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_USESTATUSBAR]      ,UseStatusBar);

 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_USEDISKLAMP ]     ,UseDiskLamp);

 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_SOUND_RATE]     ,sound_rate);
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_PSG_VOL]     ,psg_vol);
 fprintf(configStream,"%s%d\n" ,ConfigOptions[D_FM_VOL]      ,fm_vol);

 fprintf(configStream,"%s%s\n" ,ConfigOptions[D_ROMPATH]         ,RomPath);
 fprintf(configStream,"%s%d\n", ConfigOptions[D_USE_COMPATIBLEROM], new_Use_CompatibleROM);
 fprintf(configStream,"%s%s\n", ConfigOptions[D_DEBUGWORKPATH]    , debugWorkPath);
 fprintf(configStream,"%s%d\n", ConfigOptions[D_PRNMODE], PrnMode);

 fclose( configStream);

	// NOWAIT???E?c?ANOWAIT ?o?O?E?s?ÅE?B2003/8/24   Thanks Bernie
 if(!WaitFlag) sw_nowait_mode(1);
//#endif
 return(1);
}



// ****************************************************************************
//          ConfigRead
// ****************************************************************************
int ConfigRead(void)
{
//#ifdef WIN32
// char *p;

 char msg[256];
 int  i,J,N,K;
 int  max;
 static char tmp[ MAX_CONFIG_LINE ][256];
 
 printf(" Reading Config file (%s)...",ConfigPath);
 configStream= (FILE*)fopen( ConfigPath,"r");
 if(configStream==NULL) { printf("FAILED\n"); return(0);}
 
 for(i=0; i< MAX_CONFIG_LINE ; i++)
	{
	if(fgets(tmp[i],255,configStream)==NULL)
		break;
	 K = strlen( tmp[i])-1;
	 if( tmp[i][K]=='\n')
		 tmp[i][K] ='\0';	// chop
	}
 max= i;
 fclose( configStream);

 for(N=0; N< max ; N++)
	{
         for(J=0;  ConfigOptions[J] ; J++)
	   {
	    K= strlen( ConfigOptions[J]);
	    if(!strncmp(tmp[N] ,ConfigOptions[J],K))
			break;
	   }

	 switch( J)
	   {
	     case D_CONFIG:break;
	     case D_P6VERSION :
	     		P6Version= atoi( &tmp[N][K] ); 
			    P6Version = (P6Version <=4)? P6Version: 0; 
			    newP6Version= P6Version;  break;
	     case D_USESOUND:
#ifdef SOUND
		 		UseSound = atoi( &tmp[N][K] );
			    newUseSound = UseSound; 
#endif
				break;
	     case D_DISK_NUM: 
	     		disk_num = atoi( &tmp[N][K] ); 
		    	new_disk_num= disk_num; 
		    	break;
	     case D_CPUCLOCK:
	     		CPUclock = atoi( &tmp[N][K] ); 
	     		if( CPUclock==0) CPUclock=4; 
	     		break;
	     case D_SCALE:
	     		scale    = atoi( &tmp[N][K] ); 
	     	    if(scale<=0|| scale >=3) scale=2;
	     	    new_scale= scale; 
	     	    break;
	     case D_INTLAC:
	     		IntLac   = atoi( &tmp[N][K] ); break;
	     case D_CASPATH: 
	     		my_strncpy(CasPath[0], &tmp[N][K],PATH_MAX); break;
	     case D_DSKPATH:
	     		 my_strncpy(DskPath[0], &tmp[N][K],PATH_MAX); break;
		 case D_DSKPATH2:
			    my_strncpy(DskPath[1], &tmp[N][K],PATH_MAX); break;
	     case D_CASNAME: 
	     		my_strncpy(CasName[0], &tmp[N][K],PATH_MAX); break;
		 case D_DSKNAME:
			   my_strncpy(DskName[0], &tmp[N][K],PATH_MAX); break;
		 case D_DSKNAME2:
			   my_strncpy(DskName[1], &tmp[N][K],PATH_MAX); break;
	     case D_SCR4COL:
	     		scr4col = atoi( &tmp[N][K] ); break;
	     case D_DRAWWAIT:
	     		 drawwait= atoi( &tmp[N][K] ); 
	     		 if(drawwait<0) drawwait=0;
	     		 if(drawwait>262) drawwait=262;
	     		 break;
	     case D_FASTTAPE:
	     		 FastTape= atoi( &tmp[N][K] ); break;
	     case D_UPERIOD:
	     		 UPeriod = atoi( &tmp[N][K] ); 
				 if(UPeriod>2 )  UPeriod=2; 
				 UPeriod_bak = UPeriod;		// bug fixed: Ç±ÇÍÇ™Ç»Ç¢ÇÃÇ≈ÅAèÌÇ…30fps Ç…Ç»Ç¡ÇƒÇ¢ÇΩ 2005/7/17
	     		 break;
	     case D_EXTKANJIROM:
	     		 extkanjirom = atoi( &tmp[N][K] );
		 		 new_extkanjirom = extkanjirom;
	     		 break;
	     case D_EXTRAM64:
	     		 new_use_extram64 = use_extram64  = atoi( &tmp[N][K] );
	     		 break;
	     case D_EXTRAM32:
	     		 new_use_extram32 = use_extram32  = atoi( &tmp[N][K] );
	     		 break;
	     case D_SRLINE:
	     		 srline      = atoi( &tmp[N][K] );
		 		 if( srline >190) srline=190;
				 if( srline <0)  srline=0x10;
	     		 break;
	     case D_IMGPATH: 
	     		 my_strncpy(ImgPath, &tmp[N][K],PATH_MAX); break;
	     case D_MEMPATH:
	     		 my_strncpy(MemPath, &tmp[N][K],PATH_MAX); break;
	     case D_EXTROMPATH:
	     		 my_strncpy(ExtRomPath, &tmp[N][K],PATH_MAX); break;
	     case D_EXT1NAME:
	     		 my_strncpy(Ext1Name, &tmp[N][K],PATH_MAX); break;
	     case D_EXT2NAME:
	     		 my_strncpy(Ext2Name, &tmp[N][K],PATH_MAX); break;
		 case D_DUMMY: break;
	     case D_CASPATHSAVE: 
	     		my_strncpy(CasPath[1], &tmp[N][K],PATH_MAX); break;	/* tape (save)*/
	     case D_CASNAMESAVE: 
	     		my_strncpy(CasName[1], &tmp[N][K],PATH_MAX); break;
	     case D_USESAVETAPEMENU: 
	     		UseSaveTapeMenu    = atoi( &tmp[N][K] ); break;
	     case D_USESTATUSBAR: 
	     		UseStatusBar       = atoi( &tmp[N][K] ); break;
	     case D_USEDISKLAMP:
	     		 UseDiskLamp        = atoi( &tmp[N][K] ); break;
		 case D_ROMAJI_MODE:
				romaji_mode         = atoi(&tmp[N][K]); break;
		 case D_SOUND_RATE:
	     		 sound_rate         = atoi( &tmp[N][K] ); 
         		  if( sound_rate != 11025 && sound_rate != 22050 && sound_rate !=44100)
                  	  sound_rate = SOUND_RATE;
         		  break;
		 case D_PSG_VOL:
				 psg_vol = atoi( &tmp[N][K]);
				 if(psg_vol <0)  psg_vol = 0;
				 if(psg_vol >MAX_VOL) psg_vol = MAX_VOL;
				 ym2203_SetVolumePSG( psg_vol);
				 break;
		 case D_FM_VOL:
				 fm_vol = atoi( &tmp[N][K]);
				 if(fm_vol  <0)   fm_vol = 0;
				 if(fm_vol  >MAX_VOL)  fm_vol = MAX_VOL;
				 ym2203_SetVolumeFM( fm_vol);
				 break;
	     case D_ROMPATH: 
	     			if( tmp[N][K] )
				      my_strncpy(RomPath ,&tmp[N][K], PATH_MAX);
				  else
						{
					  	//strncpy(RomPath , getmodulepath(),PATH_MAX-10);	/* RomPath */
						OSD_GetModulePath( RomPath , PATH_MAX-10);        /* RomPath */
						strcat(RomPath, "\\rom\\");
						}
				  sprintf(msg,"rompath='%s' \n",RomPath);
				  break;
		 case D_USE_COMPATIBLEROM:
				Use_CompatibleROM = atoi(&tmp[N][K]);
				if (Use_CompatibleROM < 0) Use_CompatibleROM = 0;
				if (Use_CompatibleROM > 1) Use_CompatibleROM = 1;
				new_Use_CompatibleROM = Use_CompatibleROM;
				break;
         case D_DEBUGWORKPATH:
                my_strncpy( debugWorkPath , (&tmp[N][K]) , PATH_MAX);
                break;
		 case D_PRNMODE:
				PrnMode= atoi(&tmp[N][K]);
				if(PrnMode <0) {PrnMode = 0;}
				if(PrnMode >1) {PrnMode = 1;}
				break;
	   }
    }
 printf("Done.\n");
 printf("%s \n",msg);
//#endif
 return(1);
}



// ****************************************************************************
//          COMMAND LINE OPTIONS
// ****************************************************************************
enum options_defs {
   C_VERBOSE,   C_HELP , C_SHM , C_NOSHM, C_TRAP , C_SOUND ,C_NOSOUND , C_PATCH, C_SCALE , C_NOTIMER,
   C_FASTTAPE, C_EXTKANJIROM , C_EXTRAM64 , C_SRLINE , C_DUMMY1, C_DUMMY2 , C_DUMMY3, 
   C_TAPE, C_DISK , C_CLOCK , C_60, C_62 , C_64 ,C_66 , C_68 ,C_CONSOLE,
   	C_SCR4COL , C_DRAWWAIT , C_NOINTLAC , C_ROM1 ,  C_ROM2 , C_EXTROM , 
   	C_SAVETAPE ,
   	C_THREAD, 
   	C_ROMPATH,C_EXTRAM32,
	C_DISK2,
	C_ROMAJI_MODE,
	C_COMPATIBLEROM,
   	} ;

char *Options[]=
{ 
  "verbose","help","shm","noshm","trap","sound","nosound","patch","scale","notimer",
  "fasttape","extkanjirom","extram64","srline","dum","dum","dum",
  "tape","disk","clock","60","62","64","66","68","console",
  "scr4col","drawWait","nointlac","rom1","rom2","extrom",
  "savetape",
  "thread",
  "rompath",
  "extram32",
  "disk2",
  "romaji_mode",
  "compatiblerom",
  NULL
};



// ****************************************************************************
//          chkOption: check command line options
// ****************************************************************************
/** bug fixed: first option starts from argv[1] 2003/5/11 **/
int chkOption( int argc, char *argv[])
{
  int J;
  int N;


  for(N=1; N<argc; N++)
     {
      for(J=0; Options[J];J++)
	      if( !strcmp(argv[N]+1,Options[J]))
     		      break;
     
      switch(J)
        {
        case C_VERBOSE:
        		  N++; /* verbose */
                 if(N<argc) Verbose=atoi(argv[N]);
                 else printf("%s: No verbose level supplied\n",argv[0]);
                 TrapBadOps=Verbose&0x10;
                 break;
        case C_HELP:  /* help */
                 printf("%s by ISHIOKA Hiroshi    (C) 1998,1999\n",Title);
                 for(J=0;HelpText[J];J++) puts(HelpText[J]);
                 return(0);
#ifdef MITSHM
        case C_SHM:  UseSHM=1;break; /* shm */
        case C_NOSHM:  UseSHM=0;break; /* noshm */
#endif
#ifdef DEBUG
        case C_TRAP:
        	      N++; /* trap */
                 if(N>=argc)
                   printf("%s: No trap address supplied\n",argv[0]);
                 else
                   if(!strcmp(argv[N],"now")) Trace=1;
                   else sscanf(argv[N],"%hX",&Trap);
                 break;
#endif
#ifdef SOUND
        case C_SOUND:  UseSound=1;break; /* sound */
        case C_NOSOUND:  UseSound=0;break; /* nosound */
#endif
        case C_PATCH:
        		  N++; /* patch */
                 if(N<argc) PatchLevel=atoi(argv[N]);
                 else printf("%s: No patch level supplied\n",argv[0]);
				 if(PatchLevel<1) PatchLevel=0;
				 if(PatchLevel>1) PatchLevel=1;
                 break;
        case C_SCALE:
        		 N++; /* scale */
	         	if(N<argc) scale=atoi(argv[N]);
				 if(scale<1) scale=1;
				 if(scale>2) scale=2;
                 break;
        case C_NOTIMER:
        		 TimerSWFlag=0;	/* notimer */
        		 break;
        case C_FASTTAPE: FastTape=1;
				 break;
        case C_EXTKANJIROM:
        		 extkanjirom=1;
				 break;
        case C_EXTRAM64:
        		 use_extram64  =1;
			     break;
        case C_EXTRAM32:
        		 use_extram32  =1;
			     break;
        case C_SRLINE:
        		 N++;	/* srline */
	        	 if(N<argc) srline= atoi(argv[N]);
	         	 if( srline>80) srline=80;
                 break;
		case C_DUMMY1: break;
		case C_DUMMY2: break;
		case C_DUMMY3: break;

        case C_TAPE:
        		  N++;  /* tape */
                 if(N<argc)  my_strncpy(CasName[0], argv[N],PATH_MAX);
                 break;
        case C_DISK:
        		 N++;  /* disk */
                 if(N<argc)  {
							 my_strncpy(DskName[0], argv[N],PATH_MAX);
							 disk_num=1;		// patch for Unix
							}
                 break;
		case C_DISK2:
				N++;  /* disk1 */
				if(N<argc)  {
					my_strncpy(DskName[1], argv[N],PATH_MAX);
					disk_num=2;		// patch for Unix
				}
				break;
#ifndef UNIX
        case C_CLOCK:
        		 N++;	/* clock */
	        	 if(N<argc) CPUclock= atoi(argv[N]);
	         	 if( CPUclock==0) CPUclock=4;
                 break;
        case C_60:
        case C_62:
        case C_64:
        case C_66:
        case C_68:
                P6Version =J - C_60;		/* p6version */
        		 break;
        case C_CONSOLE:
        		 Console   =1;		/* console */
        		 break;
        case C_SCR4COL:
        		 scr4col   =1;		/* scr4col */
        		 break;
        case C_DRAWWAIT:
        	    N++;				/* drawWait */
	         	if(N<argc) drawwait= atoi(argv[N]);
	         	if( drawwait<0) drawwait=0;
	         	if( drawwait>262) drawwait=262;
	         	break;
        case C_NOINTLAC:
        		IntLac    =0;		/* nointlac */
				break;
        case C_ROM1:
        		 N++;  				/* Ext1Name */
                 if(N<argc)  my_strncpy(Ext1Name, argv[N],PATH_MAX);
                 break;
        case C_ROM2:
        		  N++;  				/* Ext2Name */
                 if(N<argc)  my_strncpy(Ext2Name, argv[N],PATH_MAX);
                 break;
        case C_SAVETAPE:
        		  N++;  				/* savetape */
                 if(N<argc)  my_strncpy(CasName[1], argv[N],PATH_MAX);
                 break;
        case C_THREAD:
        		  N++;  				/* thread */
                 UseCPUThread =1;
                 break;
#endif
		case C_ROMPATH:
				 N++;
				 if(N<argc) 
					 my_strncpy(RomPath , argv[N] ,PATH_MAX);
				 else
					 my_strncpy(RomPath , "rom/"  ,PATH_MAX);
				 printf("set rom search path is '%s' \n",RomPath);
				 break;
		case C_ROMAJI_MODE:
				N++;
				if (N < argc)
					romaji_mode = 1;
				else
					romaji_mode = 0;
				printf("romaji_mode is '%s' \n", romaji_mode ? "ON" : "OFF");
				break;

		case C_COMPATIBLEROM:
			N++;
			if (N < argc)
				Use_CompatibleROM = 1;
			else
				Use_CompatibleROM = 0;
			new_Use_CompatibleROM = Use_CompatibleROM;
			printf("Use_Compatible is '%d' \n", Use_CompatibleROM	);
			break;

        default: printf("Wrong option '%s'\n",argv[N]);
        }
     }
 return(1);
}
