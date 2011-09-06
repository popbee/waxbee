all: teensy32.dll teensy64.dll

CC = c:\minGW64\bin\gcc

teensy32.dll: teensy.c
	$(CC) -v -Wall -Wl,--kill-at,--output-def,teensy32_out.def -DUSE_WIN32 -DMINGW64 -m32 -shared -o teensy32.dll teensy.c -lhid -lsetupapi  

teensy64.dll: teensy.c
	$(CC) -v -Wall -Wl,--kill-at,--output-def,teensy64_out.def -DUSE_WIN32 -DMINGW64 -m64 -shared -o teensy64.dll teensy.c -lhid -lsetupapi  

clean:
	rm -f teensy32.dll
	rm -f teensy64.dll

