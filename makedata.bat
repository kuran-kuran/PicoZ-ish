rem arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm data\1R13KAN.ROM build\1R13KAN.o
rem arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm data\1R13DIC.ROM build\1R13DIC.o

cd data
python mkdat.py 1R13DIC.ROM
python mkdat.py 1R13KAN.ROM
python mkdat.py BOOT_A_LOADER.bin
copy 1R13DIC.cpp ..\source
copy 1R13KAN.cpp ..\source
copy BOOT_A_LOADER.cpp ..\source
del 1R13DIC.cpp
del 1R13KAN.cpp
del BOOT_A_LOADER.cpp
cd ..

pause
