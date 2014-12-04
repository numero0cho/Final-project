# MPLAB IDE generated this makefile for use with GNU make.
# Project: Final.mcp
# Date: Wed Dec 03 12:11:10 2014

AS = xc16-as.exe
CC = xc16-gcc.exe
LD = xc16-ld.exe
AR = xc16-ar.exe
HX = xc16-bin2hex.exe
RM = rm

Final.hex : Final.cof
	$(HX) "Final.cof" -omf=coff

Final.cof : Lab3.o lcd.o RobotMove.o
	$(CC) -omf=coff -mcpu=24FJ64GA002 "Lab3.o" "lcd.o" "RobotMove.o" -o"Final.cof" -Wl,-L"C:\Program Files (x86)\Microchip\xc16\v1.21\lib",-Tp24FJ64GA002.gld,--defsym=__MPLAB_BUILD=1,-Map="Final.map",--report-mem

Lab3.o : lcd.h c:/program\ files\ (x86)/microchip/xc16/v1.22/include/stdarg.h c:/program\ files\ (x86)/microchip/xc16/v1.22/include/stddef.h c:/program\ files\ (x86)/microchip/xc16/v1.22/include/stdio.h RobotMove.h c:/program\ files\ (x86)/microchip/xc16/v1.22/support/PIC24F/h/p24fj64ga002.h Lab3.c
	$(CC) -omf=coff -mcpu=24FJ64GA002 -x c -c "Lab3.c" -o"Lab3.o" -I"C:\Program Files (x86)\Microchip\xc16\v1.21\inc" -g -Wall

lcd.o : c:/program\ files\ (x86)/microchip/xc16/v1.22/support/PIC24F/h/p24fj64ga002.h lcd.c
	$(CC) -omf=coff -mcpu=24FJ64GA002 -x c -c "lcd.c" -o"lcd.o" -I"C:\Program Files (x86)\Microchip\xc16\v1.21\inc" -g -Wall

RobotMove.o : lcd.h c:/program\ files\ (x86)/microchip/xc16/v1.22/support/PIC24F/h/p24fj64ga002.h RobotMove.c
	$(CC) -omf=coff -mcpu=24FJ64GA002 -x c -c "RobotMove.c" -o"RobotMove.o" -I"C:\Program Files (x86)\Microchip\xc16\v1.21\inc" -g -Wall

clean : 
	$(RM) "Lab3.o" "lcd.o" "RobotMove.o" "Final.cof" "Final.hex"

