all: rawhid64.dll #RawHid.class

OS = WINDOWS
CC = gcc
JAVACC = C:\Program Files\Java\jdk1.6.0_23\bin\javac

rawhid64.dll: jni_stubs.c rawhid_$(OS).c
	$(CC) -v -Wall -m64 -shared -o rawhid64.dll jni_stubs.c rawhid_$(OS).c -lhid -lsetupapi  
#	dlltool -z rawhid64.def --export-all-symbol rawhid64.dll

#RawHid.class: org/waxbee/RawHid.java
#	$(JAVACC) org/waxbee/RawHid.java
	
clean:
	rm -f rawhid64.dll
	rm -f RawHid.class
