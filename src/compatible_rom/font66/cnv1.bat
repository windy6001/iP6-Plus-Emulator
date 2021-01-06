SET DIR=..\basic66-v031\

rem %DIR%bdf2cgrom66 mplus_j10r.bdf 
rem %DIR%bdf2cgrom66 k8x12S.bdf 
%DIR%bdf2cgrom66 misaki_gothic.bdf 


%DIR%bdf2kanjirom shnmk16.bdf
..\basic66-v022\bdf2exkanji  shnmk16.bdf EXKANJI.ROM

MOVE CGROM60.66  c_CGROM60.66n
MOVE CGROM66.66  c_CGROM66.66n
MOVE KANJIROM.66 c_KANJIROM.66n
MOVE EXKANJI.ROM c_EXKANJI.ROMn

SET DIR=
pause
