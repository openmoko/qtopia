REM This file is used to run a Qtopia application under Win32
REM qcop will fail if the C:\tmp directory does not exist 
mkdir C:\tmp
set QTDIR=.
set QPEDIR=.
set PATH=%QPEDIR%\bin;%PATH%
C:
cd "\Program Files\Trolltech\QtopiaWin32SDK"
start qpe
