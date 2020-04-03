/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**            インテリジェントディスクユニット             **/
/**                                                         **/
/**                     name is disk.h                      **/
/**                                                         **/
/** by windy                                                **/
/*************************************************************/
/* written by windy */
void disk_out( byte Port, byte Value);
byte disk_inp( byte Port);


void exec_command(void);
void dokodemo_save_disk(void);
void dokodemo_load_disk(void);

