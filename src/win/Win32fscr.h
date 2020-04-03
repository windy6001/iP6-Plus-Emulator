/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32fscr.h                   **/
/** by Windy 2002-2003                                      **/
/*************************************************************/
#ifndef _WIN32FSCR_H
#define _WIN32FSCR_H

/* ----------- full screeen  --------------- */
int toggleFullScr(void);			//  フルスクリーン <---> ウインドウ 切り替え
int isFullScreen(void);

void saveDisplayMode(void);			//  ディスプレイモードの保存
void storeDisplayMode(void);		//  ディスプレィモードを元に戻す。


#endif
