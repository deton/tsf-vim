set PF=%1
if "%PF%"=="" set PF=Win32
msbuild tsf-vim.sln /p:Configuration=Release /p:Platform=%PF%
