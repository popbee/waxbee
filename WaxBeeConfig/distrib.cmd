@echo off
ECHO.

IF "%JAVA_HOME%" == "" set JAVA_HOME=C:\Program Files\Java\jdk1.6.0_23

IF [%1]==[] GOTO ERROR2

set JAR_EXE=%JAVA_HOME%\bin\jar.exe

IF NOT EXIST "%JAR_EXE%" GOTO ERROR

set ROOT=%~dp0.
set DISTRIBDIR=%ROOT%\distrib
set AVRPROJECTROOT=%ROOT%\..\WaxB_Adb2Usb

rem cleanup the last distrib and start fresh
rd "%DISTRIBDIR%" /s /q 
mkdir "%DISTRIBDIR%"

rem waxbee.jar

mkdir "%DISTRIBDIR%\jar"
xcopy "%ROOT%\bin\org" "%DISTRIBDIR%\jar\org\" /E > nul
copy "%ROOT%\teensylib\teensy64.dll" "%DISTRIBDIR%\jar\" > nul
copy "%ROOT%\teensylib\teensy32.dll" "%DISTRIBDIR%\jar\" > nul
copy "%ROOT%\rawhidlib\rawhid64.dll" "%DISTRIBDIR%\jar\" > nul
copy "%AVRPROJECTROOT%\Release\WaxB_Adb2Usb.hex" "%DISTRIBDIR%\jar\" > nul

rem final package to zipping
mkdir "%DISTRIBDIR%\waxbee"
xcopy "%ROOT%\config_templates" "%DISTRIBDIR%\waxbee\config_templates\" /E > nul 
copy "%ROOT%\miglayout*.jar" "%DISTRIBDIR%\waxbee" > nul
copy "%ROOT%\readme.html" "%DISTRIBDIR%\waxbee" > nul

rem build the waxbee.jar application
"%JAR_EXE%" cfme0 "%DISTRIBDIR%\waxbee\waxbee.jar" "%ROOT%\manifest.txt" org.waxbee.WaxBee -C "%DISTRIBDIR%\jar" .

rem build the final zip
"%JAR_EXE%" cfM "%DISTRIBDIR%\waxbee-%1.zip" -C "%DISTRIBDIR%" waxbee

ECHO final distribution zip:  "%DISTRIBDIR%\waxbee-%1.zip"

goto END

:ERROR

ECHO Properly set JAVA_HOME to point to a Java JDK 1.6 or later
ECHO.

goto END

:ERROR2

ECHO Please pass the version number in the format 0-7b (no dots).
ECHO.

:END