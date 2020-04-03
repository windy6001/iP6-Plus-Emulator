/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                        Xconfdef.h                       **/
/**                                                         **/
/** by ISHIOKA Hiroshi 1999                                 **/
/*************************************************************/

static String fallback_resources[] = {
  "*title: iP6 config",
  "*fdshell.title: file select",
  "*allowShellResize: True",
  "*showGrip: False",
  "*Dialog.resizable: True",
  "*Dialog.value: \000",
  "*Dialog.Text.width: 300",
  "*state.borderWidth: 0",
  "*state.width: 310",
  "*state.displayCaret: FALSE",
  "*tapeButton.label: tape",
  "*tapeButton.fromVert: state",
  "*tapeLabel.borderWidth: 0",
  "*tapeLabel.displayCaret: FALSE",
  "*tapeLabel.fromVert: state",
  "*tapeLabel.fromHoriz: tapeButton",
  "*diskButton.label: disk",
  "*diskButton.fromVert: tapeButton",
  "*diskLabel.borderWidth: 0",
  "*diskLabel.displayCaret: FALSE",
  "*diskLabel.fromVert: tapeButton",
  "*diskLabel.fromHoriz: diskButton",
  "*prntButton.label: prnt",
  "*prntButton.fromVert: diskButton",
  "*prntLabel.borderWidth: 0",
  "*prntLabel.displayCaret: FALSE",
  "*prntLabel.fromVert: diskButton",
  "*prntLabel.fromHoriz: prntButton",
  "*loadButton.label: load",
  "*saveButton.label: save",

  "*colorButton.label: scr4col",
  "*colorButton.fromVert: drawWaitLabel",
  "*cpuButton.label: save CPU",
  "*cpuButton.fromVert: drawWaitLabel",
  "*cpuButton.fromHoriz: colorButton",
  "*waitButton.label: use wait",
  "*waitButton.fromVert: drawWaitLabel",
  "*waitButton.fromHoriz: cpuButton",
  "*intLacButton.label: IntLac",
  "*intLacButton.fromVert: drawWaitLabel",
  "*intLacButton.fromHoriz: waitButton",

  "*closeButton.label: close",
  "*closeButton.fromVert: colorButton",

  "*Label.borderWidth: 0",
  "*fpsLabel.fromVert: prntButton",
  "*fpsLabel.label: uperiod :",
  "*fpsText.fromVert: prntButton",
  "*fpsText.fromHoriz: fpsLabel",
  "*clockLabel.fromVert: fpsLabel",
  "*clockLabel.label: clock   :",
  "*clockText.fromVert: fpsLabel",
  "*clockText.fromHoriz: clockLabel",
  "*drawWaitLabel.fromVert: clockLabel",
  "*drawWaitLabel.label: DrawWait:",
  "*drawWaitText.fromVert: clockLabel",
  "*drawWaitText.fromHoriz: drawWaitLabel",

  //"*diskButton.sensitive: True",		// modified 2002/3/15 for DskName
  NULL,
};

#define XtNp6Version "p6Version"
#define XtCP6Version "P6Version"
#define XtNrom1File "rom1File"
#define XtCRom1File "Rom1File"
#define XtNrom2File "rom2File"
#define XtCRom2File "Rom2File"
#define XtNtapeFile "tapeFile"
#define XtCTapeFile "TapeFile"
#define XtNdiskFile "diskFile"
#define XtCDiskFile "DiskFile"
#define XtNprntFile "prntFile"
#define XtCPrntFile "PrntFile"
#define XtNscr4col "scr4col"
#define XtCScr4col "Scr4col"
#define XtNsaveCPU "saveCPU"
#define XtCSaveCPU "SaveCPU"

#define XtNdrawWait "drawWait"
#define XtCDrawWait "DrawWait"
#define XtNuperiod "uperiod"
#define XtCUPeriod "UPeriod"
#define XtNclock "clock"
#define XtCClock "Clock"
#define XtNwaitFlag "waitFlag"
#define XtCWaitFlag "WaitFlag"
#define XtNintLac "intLac"
#define XtCIntLac "IntLac"

typedef struct {
  int p6Version;
  String rom1File;
  String rom2File;
  String tapeFile;
  String diskFile;
  String prntFile;
  int scr4col;
  int saveCPU;

  int drawWait;
  int uperiod;
  int clock;
  int waitFlag;
  int intLac;
} AppResource;

static AppResource app_resources;

static XtResource resources[] = {
  {XtNp6Version, XtCP6Version, XtRInt, sizeof(int),
   XtOffsetOf(AppResource, p6Version), XtRString, "62"},
  {XtNrom1File, XtCRom1File, XtRString, sizeof(String),
   XtOffsetOf(AppResource, rom1File), XtRString, ""},
  {XtNrom2File, XtCRom2File, XtRString, sizeof(String),
   XtOffsetOf(AppResource, rom2File), XtRString, ""},
  {XtNtapeFile, XtCTapeFile, XtRString, sizeof(String),
   XtOffsetOf(AppResource, tapeFile), XtRString, ""},
  {XtNdiskFile, XtCDiskFile, XtRString, sizeof(String),   // XtNprntFile -> XtNdiskFile fixed 2002/3/15
   XtOffsetOf(AppResource, diskFile), XtRString, ""},
  {XtNprntFile, XtCPrntFile, XtRString, sizeof(String),
   XtOffsetOf(AppResource, prntFile), XtRString, ""},
  {XtNscr4col, XtCScr4col, XtRInt, sizeof(int),
   XtOffsetOf(AppResource, scr4col), XtRString, "0"},
  {XtNsaveCPU, XtCSaveCPU, XtRInt, sizeof(int),
   XtOffsetOf(AppResource, saveCPU), XtRString, "1"},

  {XtNdrawWait, XtCDrawWait, XtRInt, sizeof(int),
   XtOffsetOf(AppResource, drawWait), XtRString, "192"},
  {XtNuperiod, XtCUPeriod, XtRInt, sizeof(int),
   XtOffsetOf(AppResource, uperiod), XtRString, "2"},
  {XtNclock, XtCClock, XtRInt, sizeof(int),
   XtOffsetOf(AppResource, clock), XtRString, "4"},
  {XtNwaitFlag, XtCWaitFlag, XtRInt, sizeof(int),
   XtOffsetOf(AppResource, waitFlag), XtRString, "1"},		 /* 2007/1/7 -nowait default -> -wait default */
  {XtNintLac, XtCIntLac, XtRInt, sizeof(int),
   XtOffsetOf(AppResource, intLac), XtRString, "0"},
};

XrmOptionDescRec options[] = {
  {"-60", ".p6Version", XrmoptionNoArg, (XtPointer)"60"},
  {"-62", ".p6Version", XrmoptionNoArg, (XtPointer)"62"},
  {"-64", ".p6Version", XrmoptionNoArg, (XtPointer)"64"},
  {"-66", ".p6Version", XrmoptionNoArg, (XtPointer)"66"},
  {"-68", ".p6Version", XrmoptionNoArg, (XtPointer)"68"},
  {"-rom1", ".rom1File", XrmoptionSepArg, (XtPointer)NULL},
  {"-rom2", ".rom2File", XrmoptionSepArg, (XtPointer)NULL},
  {"-tape", ".tapeFile", XrmoptionSepArg, (XtPointer)NULL},
  {"-disk", ".diskFile", XrmoptionSepArg, (XtPointer)NULL},
  {"-printer", ".prntFile", XrmoptionSepArg, (XtPointer)NULL},
  {"-scr4col", ".scr4col", XrmoptionNoArg, (XtPointer)"1"},
  {"-saver", ".saveCPU", XrmoptionNoArg, (XtPointer)"1"},
  {"-nosaver", ".saveCPU", XrmoptionNoArg, (XtPointer)"0"},

  {"-drawWait", ".drawWait", XrmoptionSepArg, (XtPointer)NULL},
  {"-uperiod", ".uperiod", XrmoptionSepArg, (XtPointer)NULL},
  {"-clock", ".clock", XrmoptionSepArg, (XtPointer)NULL},
  {"-wait", ".waitFlag", XrmoptionNoArg, (XtPointer)"1"},
  {"-nowait", ".waitFlag", XrmoptionNoArg, (XtPointer)"0"},
  {"-intlac", ".intLac", XrmoptionNoArg, (XtPointer)"1"},
  {"-nointlac", ".intLac", XrmoptionNoArg, (XtPointer)"0"},
};

extern void SetByCR();

XtActionsRec actions[] = {
  {"SetByCR", SetByCR}
};
String translationsTable =
  "<Key>Return: SetByCR() end-of-line()";

