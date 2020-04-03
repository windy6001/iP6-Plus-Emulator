/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           wav.h                         **/
/**                                                         **/
/** by Windy 2003                                           **/
/*************************************************************/


struct _spec {
	int channels;
	int freq;
	int samplebit;
};


int loadWav( char *path ,struct _spec *spec, short **out_buff , int *len);
void freeWav( char *buff);

