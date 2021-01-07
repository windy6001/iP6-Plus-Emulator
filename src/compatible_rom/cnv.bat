
set DIR=basic60-v072
..\..\tools\bin2c %DIR%\c_BASICROM.60n >basicrom60.h
set DIR=

..\..\tools\bin2c CGROM60\c_CGROM60.60n >cgrom60.h


set DIR=basic66-v031
..\..\tools\bin2c %DIR%\c_BASICROM.66n >basicrom66.h
..\..\tools\bin2c %DIR%\c_VOICEROM.66n >voicerom66.h

..\..\tools\bin2c font66\c_KANJIROM.66n >kanjirom66.h
..\..\tools\bin2c font66\c_CGROM60.66n  >cgrom6066.h
..\..\tools\bin2c font66\c_CGROM66.66n  >cgrom6666.h

..\..\tools\bin2c font66\c_EXKANJI.ROMn >exkanjirom.h


set DIR=
pause