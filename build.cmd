@echo off
setlocal EnableDelayedExpansion
echo --- compile ---
g++ %* > compile-out.txt 2>&1
set ERR=!ERRORLEVEL!
echo COMPILE_EXIT=!ERR!
type compile-out.txt
exit /b !ERR!
