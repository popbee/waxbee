all: teensy64.dll

CC = gcc
JAVACC = C:\Program Files\Java\jdk1.6.0_23\bin\javac

teensy64.dll: teensy.c
	$(CC) -v -Wall -DUSE_WIN32 -m64 -shared -o teensy64.dll teensy.c -lhid -lsetupapi  
#	dlltool -z teensy.def --export-all-symbol teensy.dll

Teensy.class: Teensy.java
	$(JAVACC) Teensy.java
	
clean:
	rm -f teensy.dll
	rm -f Teensy.class
