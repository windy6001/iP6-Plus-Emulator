/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           fm.h                          **/
/** written by Windy 2002                                   **/
/*************************************************************/




#ifdef __cplusplus
extern "C" {
#endif
	int  ym2203_Open(void);
	void ym2203_setreg(int r,int v);
	void ym2203_makewave( short *dest ,int size);
	int  ym2203_getreg(int r);
	int  ym2203_Count(int us);
	int  ym2203_GetNextEvent(void);
	void downcast16( short *dest , int *src ,int length);
    
    int  ym2203_get_timera_ovflag(void);
    int  ym2203_get_timerb_ovflag(void);
    int  ym2203_ReadStatus(void);

	void ym2203_SetVolumePSG(int vol);
	void ym2203_SetVolumeFM(int vol);
	int  ym2203_GetVolumePSG(void);
	int  ym2203_GetVolumeFM(void);
	
	void dokodemo_save_fm(void);
	void dokodemo_load_fm(void);
	void ym2203_Reset(void);


#define BASE_VOL     (-80)
#define SCALE_VOL    (6)
#define MAX_VOL      20


#ifdef __cplusplus
}
#endif
