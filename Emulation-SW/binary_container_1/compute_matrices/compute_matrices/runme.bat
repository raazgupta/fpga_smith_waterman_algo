@echo off

rem  xocc (TM)
rem  runme.bat: a xocc-generated Script
rem  Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.


set HD_SDIR=%~dp0
cd /d "%HD_SDIR%"
set PATH=%SYSTEMROOT%\system32;%PATH%
cscript /nologo /E:JScript "%HD_SDIR%\rundef.js" %*
