c:
cd\guide8
@echo off
if not exist scope.mar goto err_msg
copy startup.mar startup.old
del startup.mar
rename scope.mar startup.mar
goto start_guide
:err_msg
echo WARNING: Scope.mar wasn't found! Guide will not actually go to the
echo position selected from AltAz.
pause
:start_guide
dosguide.exe
