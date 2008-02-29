@echo OFF
REM This script is for starting a command prompt with
REM 	the enviroment setup ready to compile Qtopia 
REM     applications under Win32
REM


REM If MSVCDir is set then the MSVC++ enviroment already is setup for command line usage.
IF DEFINED MSVCDir GOTO setvars

REM Supports MSVC++ in either c: or e: but on the same path
IF NOT EXIST "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat" GOTO nonstdvc
CALL "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat" 
goto setvars

:nonstdvc
CALL "E:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat" 

:setvars
SET QPEDIR=C:\Progra~1\Trolltech\QtopiaWin32SDK
SET QTEDIR=%QPEDIR%
SET QTDIR=%QPEDIR%
SET TMAKEDIR=%QPEDIR%\tmake
SET TMAKEPATH=%TMAKEDIR%\lib\qws\win32-msvc
SET PATH=%QPEDIR%\bin;%TMAKEDIR%\bin;%PATH%
SET QCONFIG=CONFIG+=qtopia CONFIG+=qt2 DEFINES+=QT_DLL TMAKE_LFLAGS+=/NODEFAULTLIB:libc
SET SKIN=%QPEDIR%\QtopiaPDA.skin
c:
cd %QPEDIR%
cmd

