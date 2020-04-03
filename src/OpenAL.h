int open_openal(void);
void close_openal(void);
void write_openal(short *src,int len);
void openal_ResumeSound(void);
void openal_StopSound(void);

extern const struct plugin_struct sysdep_dsp_openal; 
