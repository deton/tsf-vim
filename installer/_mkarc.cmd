@echo off

pushd %~dp0

7z.exe a -tzip tsf-vim.zip tsfvim-x64.msi tsfvim-x86.msi ..\README.TXT ..\README_JA.TXT ..\LICENSE.TXT

popd
