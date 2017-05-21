@REM CMD File - note: this file has been generated do not modify

make OBJDIR=Build/waxbee_serial TARGET=Release/waxbee_serial DEP=Build/waxbee_serial.dep "SUPPORT=-DCUSTOM_SUPPORT -DINITGPIO_SUPPORT -DTOUCH_SUPPORT -DBUTTON_SUPPORT -DUSB_PROTOCOL_IV_SUPPORT -DUSB_PROTOCOL_V_SUPPORT -DSERIAL_PROTOCOL_IV_SUPPORT -DSERIAL_PROTOCOL_V_SUPPORT"

make OBJDIR=Build/waxbee_debug TARGET=Release/waxbee_debug DEP=Build/waxbee_debug.dep "SUPPORT=-DCUSTOM_SUPPORT -DDEBUG_SUPPORT -DINITGPIO_SUPPORT -DTOUCH_SUPPORT -DBUTTON_SUPPORT -DADB_SUPPORT -DSERIAL_ISDV4_SUPPORT -DSERIAL_PROTOCOL_IV_SUPPORT -DSERIAL_PROTOCOL_V_SUPPORT -DSERIAL_TOPAZ_SUPPORT"

make OBJDIR=Build/waxbee_isdv4 TARGET=Release/waxbee_isdv4 DEP=Build/waxbee_isdv4.dep "SUPPORT=-DCUSTOM_SUPPORT -DINITGPIO_SUPPORT -DTOUCH_SUPPORT -DBUTTON_SUPPORT -DUSB_PROTOCOL_IV_SUPPORT -DUSB_PROTOCOL_V_SUPPORT -DSERIAL_ISDV4_SUPPORT"

make OBJDIR=Build/waxbee_isdv4_debug TARGET=Release/waxbee_isdv4_debug DEP=Build/waxbee_isdv4_debug.dep "SUPPORT=-DCUSTOM_SUPPORT -DDEBUG_SUPPORT -DINITGPIO_SUPPORT -DTOUCH_SUPPORT -DBUTTON_SUPPORT -DSERIAL_ISDV4_SUPPORT"

make OBJDIR=Build/waxbee_serial_debug TARGET=Release/waxbee_serial_debug DEP=Build/waxbee_serial_debug.dep "SUPPORT=-DCUSTOM_SUPPORT -DDEBUG_SUPPORT -DINITGPIO_SUPPORT -DTOUCH_SUPPORT -DBUTTON_SUPPORT -DSERIAL_PROTOCOL_IV_SUPPORT -DSERIAL_PROTOCOL_V_SUPPORT"

make OBJDIR=Build/waxbee_adb TARGET=Release/waxbee_adb DEP=Build/waxbee_adb.dep "SUPPORT=-DCUSTOM_SUPPORT -DINITGPIO_SUPPORT -DADB_SUPPORT -DUSB_PROTOCOL_IV_SUPPORT -DUSB_PROTOCOL_V_SUPPORT"

make OBJDIR=Build/waxbee_full TARGET=Release/waxbee_full DEP=Build/waxbee_full.dep "SUPPORT=-DCUSTOM_SUPPORT -DDEBUG_SUPPORT -DINITGPIO_SUPPORT -DTOUCH_SUPPORT -DBUTTON_SUPPORT -DADB_SUPPORT -DUSB_PROTOCOL_IV_SUPPORT -DUSB_PROTOCOL_V_SUPPORT -DSERIAL_ISDV4_SUPPORT -DSERIAL_PROTOCOL_IV_SUPPORT -DSERIAL_PROTOCOL_V_SUPPORT -DSERIAL_TOPAZ_SUPPORT"

make OBJDIR=Build/waxbee_adb_debug TARGET=Release/waxbee_adb_debug DEP=Build/waxbee_adb_debug.dep "SUPPORT=-DCUSTOM_SUPPORT -DDEBUG_SUPPORT -DINITGPIO_SUPPORT -DBUTTON_SUPPORT -DADB_SUPPORT"
