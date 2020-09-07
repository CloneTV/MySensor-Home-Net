@echo off
rem Path example, %ATMEL_STUDIO% = E:/.../.../Arduino/AtmelStudio/7.0/
%ATMEL_STUDIO%\shellutils\make clean ATMELSTUDIO=%ATMEL_STUDIO% ARDUINOULIB=%ARDUINO_USER_LIB%
%ATMEL_STUDIO%\shellutils\make out OS=windows ENV=arduino ATMELSTUDIO=%ATMEL_STUDIO% ARDUINOULIB=%ARDUINO_USER_LIB% %*
