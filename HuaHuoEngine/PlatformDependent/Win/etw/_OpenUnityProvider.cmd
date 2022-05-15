call "%VS140COMNTOOLS%VsDevCmd.bat"
cd "%~dp0"
echo "%~dp0\UnityETWProvider.man"
ECManGen.exe UnityETWProvider.man
pause