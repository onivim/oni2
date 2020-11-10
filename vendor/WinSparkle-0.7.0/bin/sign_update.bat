@echo off

set argC=0
for %%i in (%*) do set /A argC+=1

if not "%argC%"=="2" (
  echo Usage: %0 update_file private_key
  exit /b 1
)

openssl dgst -sha1 -binary < "%~1" | openssl dgst -sha1 -sign "%~2" | openssl enc -base64
