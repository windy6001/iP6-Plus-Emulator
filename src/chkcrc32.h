/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                        chkcrc32.h                       **/
/**                                                         **/
/*************************************************************/
unsigned long getMemCRC32( unsigned long crc32, unsigned const char buff[], size_t size );
unsigned long getFileCRC32( const char file[] );
void InitCRC32(void);

unsigned int GetCRC32(unsigned char *buffer, unsigned int bufferlen, unsigned int crc32_start);
