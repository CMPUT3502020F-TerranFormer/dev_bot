# TFBot
To run the project (Windows):

```
mkdir build

cd build

cmake ../ -G "Visual Studio 16 2019"

start TFBot.sln
```

Build the project, then it can be run like in the example, eg.

Make sure that TFBot.db exists in the same folder as the executable (and it is not 0kb)

```
TFBot.exe -c -a zerg -d hard -m CactusValleyLE.SC2Map
```
