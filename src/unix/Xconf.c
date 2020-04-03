/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                          Xconf.c                        **/
/**                                                         **/
/** by ISHIOKA Hiroshi 1999                                 **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef X11
/** Standard Unix/X #includes ********************************/
#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Toggle.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>


#include "../Z80.h"
#include "../P6.h"
#include "Xconf.h"
#include "Xconfdef.h"
#include "mem.h"

/* static functions */
static void build_status(void);
static void build_periSection(void);
static void build_speedSection(void);
static void build_toggleSection(void);
static void build_fileDialog(void);
static void close_conf(Widget w,XtPointer client_data,XtPointer call_data);
static void periFileSet(Widget w,XtPointer client_data,XtPointer call_data);
static void popup(Widget w,XtPointer client_data,XtPointer call_data);
static void popdown(Widget w,XtPointer client_data,XtPointer call_data);
static void toggle(Widget w,XtPointer client_data,XtPointer call_data);

/** Various X-related variables ******************************/
static XtAppContext app_context;
static Widget toplevel,base,closeButton,colorButton,cpuButton;
static Widget periButton[3],periLabel[3];
static Widget uperiod, clock, drawWait, waitButton, intLacButton;
static Widget state;
static Widget fileDialog;

/** Various variables and short functions ********************/
int in_popup, in_conf;
char *periList[] = { &CasName[0][0], &DskName[0][0], &PrnName[0] };
extern int SaveCPU;
extern int WaitFlag;
extern int IntLac;

void SetByCR(Widget w,XEvent *event,String *params,int nparams)
{
  XtCallCallbacks(XtNameToWidget(XtParent(w),"OK"),XtNcallback,NULL);
}


Xt_init_menu()
{
#if 0
  Widget form = XtVaCreateManagedWidget("form", formWidgetClass, toplevel, NULL);
  
  // create Menu button
  Widget MenuButton1 = XtVaCreateManagedWidget("Menu1", menuButtonWidgetClass , form, 
                                                XtNmenuName, "menu1", 
                                                XtNvertDistance, 5 ,
                                                XtNheight, 10, 
                                                NULL);

  // create Menu shell
  Widget menu1 = XtVaCreatePopupShell( "menu1", simpleMenuWidgetClass, form , NULL);
  
  // add items
  Widget MenuItem1;
  MenuItem1 = XtVaCreateManagedWidget("item1", smeBSBObjectClass, menu1 , XtNlabel , "item1", NULL);
  
  Widget label1 = XtVaCreateManagedWidget("label1", // widget name
                                          labelWidgetClass, // label class
                                          form ,
                                          XtNfromVert, MenuButton1, XtNvertDistance, 2, 
                                          XtNlabel , "", // displayed string
                                          XtNwidth , 100, 
                                          XtNheight , 100, 
                                          XtNborderWidth ,1 , 
                                          NULL);

  XtRealizeWidget(toplevel);
  	XtAppMainLoop( app_context );
#endif
}


/** Xt_init **************************************************/
/** initialize Xt and get resource                          **/
/*************************************************************/
void Xt_init(int *argc, char ***argv)
{
  toplevel = XtAppInitialize(&app_context,"IP6",options,XtNumber(options),
			     argc,*argv,fallback_resources,NULL,0);


  XtVaGetApplicationResources(toplevel,&app_resources,
			      resources,XtNumber(resources),NULL);
  switch(app_resources.p6Version) {
    case 60: case 62: case 64: case 66: case 68:
      P6Version = (app_resources.p6Version-60)/2; break;
    default:
      fprintf(stderr,"invalid P6 Model %d.\n",app_resources.p6Version);
  };
  strcpy(Ext1Name, app_resources.rom1File);
  strcpy(Ext2Name, app_resources.rom2File);
  strcpy(CasName[0], app_resources.tapeFile);
  strcpy(DskName[0], app_resources.diskFile);
  strcpy(PrnName, app_resources.prntFile);
  scr4col = app_resources.scr4col;
  SaveCPU = app_resources.saveCPU;

  UPeriod = app_resources.uperiod;
  UPeriod_bak= UPeriod;				// backup  UPeriod
  SetValidLine_sr(app_resources.drawWait);
  drawwait = app_resources.drawWait;// backup drawwait
  SetClock(app_resources.clock*1000000);
  CPUclock = app_resources.clock;	// backup CPUclock
  WaitFlag = app_resources.waitFlag;
  IntLac = app_resources.intLac;
}

/** build_conf ***********************************************/
/** construct configure panel                               **/
/*************************************************************/
void build_conf(void)
{
  if(Verbose) printf("Building configure panel...");

  XtAppAddActions(app_context,actions,XtNumber(actions));
  /*** base form ***/
  base = XtCreateManagedWidget("base",formWidgetClass,toplevel,NULL,0);

  /*** status text, peripheral button & text ***/
  build_status();
  build_periSection();
  build_speedSection();
  build_toggleSection();
  /*** close button ***/
  closeButton =
    XtVaCreateManagedWidget("closeButton",commandWidgetClass,base,NULL);
  XtAddCallback(closeButton,XtNcallback,(XtCallbackProc)close_conf,NULL);

  /*** popup shell ***/
  build_fileDialog();

  if(Verbose) printf("OK\n");
}

/** run_conf *************************************************/
/** display panel and wait for close                        **/
/*************************************************************/
void run_conf(void)
{
  unsigned int i;
  static unsigned int flag=0;
  XEvent event;
  char str[1024];

  /* set filename */
  for(i=0;i<3;i++) XtVaSetValues(periLabel[i],XtNstring,periList[i],NULL);
  sprintf(str, "%d", UPeriod);
  XtVaSetValues(uperiod,XtNstring,str,NULL);
  sprintf(str, "%d", app_resources.clock);
  XtVaSetValues(clock,XtNstring,str,NULL);
  sprintf(str, "%d", app_resources.drawWait);
  XtVaSetValues(drawWait,XtNstring,str,NULL);

  in_conf=1;
  /* display panel */
  if(flag) XtMapWidget(toplevel);
  else { XtRealizeWidget(toplevel);flag=1; }
  /* event loop */
  for(;;) {
    XtAppNextEvent(app_context,&event);
    XtDispatchEvent(&event);
    if((in_conf==0)&&(event.type==UnmapNotify)) break;
  }
}

/** build_status *********************************************/
/** construct status text                                   **/
/*************************************************************/
void build_status(void)
{
  char str[1024], *model[5] =
    { "PC-6001", "PC-6001mk2", "PC-6001mk2SR", "PC-6601", "PC-6601SR" };
  XFontStruct *fs;
  Position top,bottom;

  state = XtCreateManagedWidget("state",asciiTextWidgetClass,base,NULL,0);
  /* make string */
  sprintf(str,"P6Model: %s\nExtRom1: %s\nExtRom2: %s",
	  model[P6Version],(EXTROM1?(char*)EXTROM1:"(no file)"),
	  (EXTROM2?(char*)EXTROM2:"(no file)"));
  /* get text-widget height */
  XtVaGetValues(state,XtNtopMargin,&top,XtNbottomMargin,&bottom,
		XtNfont,&fs,NULL);
  /* set result */
  XtVaSetValues(state,XtNheight,(fs->ascent+fs->descent)*3+top+bottom,
		XtNstring,str,NULL);
}

/** build_speedSection ***************************************/
/** construct peripheral form                               **/
/*************************************************************/
void build_speedSection(void)
{
  unsigned int i;
  char *labelName[] = { "fpsLabel", "clockLabel", "drawWaitLabel" };
  char *textName[] = { "fpsText", "clockText", "drawWaitText" };
  Widget *widgetlist[] = { &uperiod, &clock, &drawWait };

  for (i=0; i<3; i++) {
    XtVaCreateManagedWidget(labelName[i],labelWidgetClass,base,NULL);
    *widgetlist[i] =
      XtVaCreateManagedWidget(textName[i],asciiTextWidgetClass,base,NULL);
    XtVaSetValues(*widgetlist[i], XtNeditType, XawtextEdit, NULL);
  }
}

/** build_toggleSection **************************************/
/** construct peripheral form                               **/
/*************************************************************/
void build_toggleSection(void)
{
  unsigned int i;
  char *buttonName[] =
    { "colorButton", "cpuButton", "waitButton", "intLacButton" };
  Widget *widgetList[] =
    { &colorButton, &cpuButton, &waitButton, &intLacButton };
  int *varList[] = { &scr4col, &SaveCPU, &WaitFlag, &IntLac };

  for (i=0; i<4; i++) {
    *widgetList[i] =
      XtVaCreateManagedWidget(buttonName[i],toggleWidgetClass,base,NULL);
    XtAddCallback(*widgetList[i],XtNcallback,(XtCallbackProc)toggle,NULL);
    if(*varList[i]) XtVaSetValues(*widgetList[i],XtNstate,True,NULL);
  }
}

/** build_periSection ****************************************/
/** construct peripheral form                               **/
/*************************************************************/
void build_periSection(void)
{
  unsigned int i;
  char *periButtonName[] = { "tapeButton", "diskButton", "prntButton" };
  char *periLabelName[] = { "tapeLabel", "diskLabel", "prntLabel" };

  for(i=0;i<3;i++) {
    periButton[i] =
      XtVaCreateManagedWidget(periButtonName[i],commandWidgetClass,base,NULL);
    XtAddCallback(periButton[i],XtNcallback,(XtCallbackProc)popup,NULL);
    periLabel[i] =
      XtVaCreateManagedWidget(periLabelName[i],asciiTextWidgetClass,base,
			      XtNwidth,250,NULL);
  }
}

/** build_fileDialog *****************************************/
/** construct file dialog                                   **/
/*************************************************************/
void build_fileDialog(void)
{
  char *fdButtonName[] = { "OK", "Cancel" };
  Widget shell;
  XtTranslations translations = XtParseTranslationTable(translationsTable);

  shell = XtCreatePopupShell
    ("fdshell", transientShellWidgetClass, base, NULL, 0);
  fileDialog =
    XtVaCreateManagedWidget("fileDialog",dialogWidgetClass,shell,NULL);
  XtOverrideTranslations(XtNameToWidget(fileDialog,"value"),translations);
  XawDialogAddButton(fileDialog,fdButtonName[0],periFileSet,fileDialog);
//  XawDialogAddButton(fileDialog,fdButtonName[1],periFileSet,"");
  XawDialogAddButton(fileDialog,fdButtonName[1],periFileSet,NULL); /* 2003/11/3 */
}

/** close_conf ***********************************************/
/** when clicked close button, set flag                     **/
/*************************************************************/
void close_conf(Widget w, XtPointer client_data, XtPointer call_data)
{
  int Clock, DrawWait;
  String buf;
  XtUnmapWidget(toplevel);
  in_conf=0;
  XtVaGetValues(uperiod, XtNstring, &buf, NULL);
  UPeriod = atoi(buf);
  if (UPeriod==0) UPeriod=2;
  XtVaGetValues(clock, XtNstring, &buf, NULL);
  Clock = atoi(buf);
  if (Clock == 0) Clock=4;
  XtVaGetValues(drawWait, XtNstring, &buf, NULL);
  DrawWait = atoi(buf);
  if (DrawWait == 0) DrawWait = 192;
  SetValidLine_sr(DrawWait);
  SetClock(Clock*1000000);
  app_resources.clock = Clock;
  app_resources.drawWait = DrawWait;
}

/** popdown **************************************************/
/** when clicked ok/cancel button, set flag                 **/
/*************************************************************/
void popdown(Widget w, XtPointer client_data, XtPointer call_data)
{ XtPopdown(XtParent(XtParent(w))); in_popup=0; }

/** periFileSet **********************************************/
/** close and open peripherial file                         **/
/*************************************************************/
void periFileSet(Widget w, XtPointer client_data, XtPointer call_data)
{
  unsigned int i;
  String str;
  char *peri[] = { "tape file   ", "disk file   ", "printer file" };

  //printf("periFileSet(): client='%s'  call='%s'\n",client_data, call_data);

//  if(!strcmp(client_data,"")) { /* if OK button */	/* add ! (Not)  2002/4/25 windy */
  if( client_data !=NULL) {		/* 2003/11/3 */
    XtVaGetValues(XtParent(w), XtNlabel, &str, NULL);
    /* which file? */
    for(i=0; i<3; i++) if(!(strcmp(str,peri[i]))) break;
    /* get filename */
    XtVaGetValues(client_data, XtNvalue, &str, NULL);
    /* close and open */
    switch (i) {
      case 0: strcpy(CasName[0],str); OpenFile1(FILE_LOAD_TAPE); break; /* tape */
      case 1: strcpy(DskName[0],str); OpenFile1(FILE_DISK1); break; /* disk */
      case 2: strcpy(PrnName,str); OpenFile1(FILE_PRNT); break; /* prnt */
    }
    XtVaSetValues(periLabel[i], XtNstring, str, NULL);
  }
  popdown(w,NULL,NULL);
}

/** popup ****************************************************/
/** popup peripherial file dialog                           **/
/*************************************************************/
void popup(Widget w, XtPointer client_data, XtPointer call_data)
{
  unsigned int i;
  char *labelList[] = { "tape file   ", "disk file   ", "printer file" };
  /* which file? */
  for(i=0;(i<3)&&(periButton[i]!=w);i++)
	;
  /* set text */
  XtVaSetValues
    (fileDialog, XtNlabel, labelList[i], XtNvalue, periList[i], NULL);
  /* popup shell */
  in_popup = 1;
  XtManageChild(fileDialog);
  XtPopup(XtParent(fileDialog), XtGrabExclusive);
  while (in_popup || XtAppPending(app_context))
    XtAppProcessEvent(app_context, XtIMXEvent);
}

/** toggle ***************************************************/
/** toggle button and variable                              **/
/*************************************************************/
void toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
  if(w==colorButton) scr4col=1-scr4col;
  else if(w==cpuButton) SaveCPU=1-SaveCPU;
  else if(w==waitButton) WaitFlag=1-WaitFlag;
  else if(w==intLacButton) IntLac=1-IntLac;
}
#endif
