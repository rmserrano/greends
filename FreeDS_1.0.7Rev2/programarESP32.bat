@echo off
if "%1"=="" goto bad

:good 

esptool --chip esp32 --port "%1" --baud 460800 --before default_reset --after hard_reset erase_flash
esptool --chip esp32 --port "%1" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0x1000 bootloader_dio_80m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin 0x290000 spiffs.bin
goto end

:bad 

echo Error: Es necesario especificar el puerto COM a usar, por ejemplo: programar.bat COM4

:end



