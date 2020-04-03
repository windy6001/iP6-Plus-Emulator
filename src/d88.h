/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           d88.h                         **/
/**                                                         **/
/*                     Inteligent  disk unit                 */
/*************************************************************/
/* This file is written by windy */



int is1DD(int drive);
int isSYS(int drive);
int read_d88head(int drive);
byte* read_d88sector(int kai,int drive ,int track ,int sector);
int write_d88sector(int kai, int drive ,int track ,int sector, byte *buff);

void setDiskProtect(int drive,int Value);
int  getDiskProtect(int drive);

void dokodemo_save_d88(void);
void dokodemo_load_d88(void);

