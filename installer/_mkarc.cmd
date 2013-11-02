@echo off

pushd %~dp0

7z.exe a tsfvim.7z tsfvim-x64.msi tsfvim-x86.msi ..\README.TXT ..\LICENSE.TXT

popd
