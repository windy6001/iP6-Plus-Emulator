int open_waveout(void);
void close_waveout(void);
void write_waveout(short *src,int len);
void waveout_ResumeSound(void);
void waveout_StopSound(void);

extern const struct plugin_struct sysdep_dsp_waveout; 
