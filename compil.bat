@REM Created By Gary Ng
@REM Cause he is too lazy and rather double-click than type in commands 

Set WshShell = WScript.CreateObject("WScript.Shell")

cd build
cmake ../
start TFBot.sln