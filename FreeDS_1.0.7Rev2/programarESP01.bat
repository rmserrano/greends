@echo off
if "%1"=="" goto bad

:good 

esptool --chip esp8266 --port "%1" --baud 115200 --before default_reset --after hard_reset erase_flash
Echo Desconecte el ESP01 del programador, vuelva a conectarlo y a continuacion pulse cualquier tecla para continuar.
PAUSE >nul
esptool --chip esp8266 --port "%1" --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x00000 firmware_ESP01.bin
goto end

:bad 

echo Error: Es necesario especificar el puerto COM a usar, por ejemplo: programarESP01.bat COM4

:end



