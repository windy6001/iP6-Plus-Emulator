/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                          Help.h                         **/
/**                                                         **/
/** by ISHIOKA Hiroshi 1998, 1999                           **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/

char *HelpText[] =
{
  "Usage: iP6 [-option1 [-option2...]]",
  "[-option]   =",
  "  -uperiod <period>   - Number of interrupts per screen update [2]",
  "  -help               - Print this help page",
#ifdef MITSHM
  "  -shm/-noshm         - Use/don't use MIT SHM extensions for X [-shm]",
#endif
#ifdef DEBUG
  "  -trap <address>     - Trap execution when PC reaches address [FFFFh]",
#endif
  "  -60/62/64/66/68     - Select P6 model [-62]",
  "  -rom1 <filename>    - Load external ROM file to &H4000 [off]",
  "  -rom2 <filename>    - Load external ROM file to &H6000 [off]",
  "  -disk <filename>    - Set disk image file to drive 1 [off]",			/* 2002/3/30 windy */
  "  -disk2 <filename>   - Set disk image file to drive 2 [off]",			/* 2002/3/30 windy */
  "  -tape <filename>    - Set tape image file for load [off]",
  "  -savetape <filename>- Set tape image file for save [off]", /* 2003/12/30 windy*/
  "  -printer <filename> - Redirect printer output to file [stdout]",
  "  -scr4col            - Colored N60 screen 4 mode [off]",
#ifdef UNIX
  "  -saver/-nosaver     - Save/don't save CPU when inactive [-saver]",
#endif
#ifdef SOUND
  "  -sound/-nosound     - Play/don't play sound [-nosound]",
#endif
  "  -patch <n>          - set patch level(0-1) [1]",

  "  -drawWait <n>       - screen draw wait [192]",
  "  -clock <n>          - Z80 clock [4MHz]",
  "  -wait/-nowait       - Use/don't use wait [-wait]", /* 2007/1/7 -nowait default -> -wait default */
  "  -scale <n>          - Set window size(1-2) [2]",
  "  -intlac/-nointlac   - Use/don't interlace [-intlac]",
  "  -notimer            - disable 2ms timer Interrupt ",/* add 2002/12/14*/
  "  -fasttape           - Set Tape Speed High [off]",
  "  -extkanjirom        - Use Ext kanjirom (128kb)  [off]",
  "  -extram             - Use Ext RAM (64kb)  [off]",
  "  -srline <n>         - sr line (0-80)      [80]",
  "  -rompath <path>     - set rom search path [rom/]",
  "  -extram32           - use Extended RAM    [off]",
  "  -romaji_mode        - Romaji mode on      [off]",
  NULL
};
