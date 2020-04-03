set DIR=basic60-v07
..\..\tools\bin2c %DIR%\BASICROM.60 >basicrom60.h
rem ..\..\tools\bin2c %DIR%\BASICROM.62 >basicrom62.h
set DIR=

..\..\tools\bin2c CGROM60\CGROM60.60 >cgrom60.h

set DIR=basic66-v022
..\..\tools\bin2c %DIR%\BASICROM.66 >basicrom66.h
..\..\tools\bin2c %DIR%\VOICEROM.66 >voicerom66.h

..\..\tools\bin2c font66\KANJIROM.66 >kanjirom66.h
..\..\tools\bin2c font66\CGROM60.66  >cgrom6066.h
..\..\tools\bin2c font66\CGROM66.66  >cgrom6666.h


rem copy %DIR%\VOICEROM.66 %DIR%\VOICEROM.62
rem copy %DIR%\KANJIROM.66 %DIR%\KANJIROM.62
rem copy %DIR%\CGROM60.66  %DIR%\CGROM60.62
rem copy %DIR%\CGROM66.66  %DIR%\CGROM60m.62
rem ..\..\tools\bin2c %DIR%\CGROM60.62  >cgrom6062.h
rem ..\..\tools\bin2c %DIR%\CGROM60m.62  >cgrom60m62.h
rem ..\..\tools\bin2c %DIR%\VOICEROM.62 >voicerom62.h
rem ..\..\tools\bin2c %DIR%\KANJIROM.62 >kanjirom62.h

..\..\tools\bin2c %DIR%\EXKANJI.ROM >exkanjirom.h

set DIR=
