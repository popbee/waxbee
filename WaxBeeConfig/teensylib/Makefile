all: teensy64.dll teensy32.dll

CC = gcc
JAVACC = C:\Program Files\Java\jdk1.6.0_23\bin\javac

teensy64.dll: teensy.c
	$(CC) -v -Wall -DUSE_WIN32 -m64 -shared -o teensy64.dll teensy.c -lhid -lsetupapi  
#	dlltool -z teensy64.def --export-all-symbol teensy64.dll

teensy32.dll: teensy.c
	$(CC) -v -Wall -DUSE_WIN32 -shared -o teensy32.dll teensy.c -lhid -lsetupapi  
#	dlltool -z teensy32.def --export-all-symbol teensy32.dll

Teensy.class: Teensy.java
	$(JAVACC) Teensy.java
	
clean:
	rm -f teensy.dll
	rm -f Teensy.class
