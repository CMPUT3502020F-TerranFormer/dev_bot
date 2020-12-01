# TFBot
To run the project (Windows):
```
mkdir build

compil.bat
```
OR

```
mkdir build

cd build

cmake ../ -G "Visual Studio 16 2019"

start TFBot.sln
```
If there is a problem building civetweb.c open the file, go to Project -> civetweb-c library Properties -> Configuration Properties -> C/C++ -> General
And set "Treat Warnings as Errors" to No

**Make sure that TFBot.db exists in the same folder as the executable (and it is not 0kb)**

Build the project, then it can be run like in the example, eg.

**When running on Windows, the project must be launched from the command line. The "Local Windows Debugger" doesn't like sqlite.**


```
TFBot.exe -c -a zerg -d hard -m CactusValleyLE.SC2Map
```
