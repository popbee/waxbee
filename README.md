This hardware project aims at building a USB bridge for ADB or Serial-based Wacom/Calcomp tablets and effectively convert serial/adb tablets to USB.

This is a C++ based project meant to run on low cost AVR [Teensy][] hardware. Java-based configuration utility running on Win32/64, Mac OS X & Linux.

NOTE: Soldering is required to perform a tablet conversion. This is done at your own risks.

Projects
========

| Project | Platform | Notes |
|-------------|--------------|-----------|
| Serial/ADB to USB bridge | Teensy 2.0 | Basic functionality working, on going |
| Waxbee Config | Java | Early version working, on going | 
| ADB Sniffer | Arduino UNO | Done |
| USB virtual serial port | Teensy 2.0 | Done |

### Native Tablet Support Status

|   |   |   |
|---|---|---|
| *ADB Ultrapad/Digitizer/DigitizerII/ArtZII* | UD-xxxx-A | basic pen functionality|
| *ADB Intuos* | GD-xxxx-A | Non functional. See ADB Intuos section below. | 
| *Serial Penenabled* (TabletPC) | (various) | (ISDV4) Basic Pen functionality works |
| *Serial Penenabled TabletPC Touch* | (various) | (ISDV4) early coding stage |
| *Serial Ultrapad/Digitizer II/ArtZII* | UD-xxxx-R | (Protocol IV) is working (basic pen functionality). |
| *Serial ArtPad II* | KT-xxxx-R | (Protocol IV) basic pen functionality |
| *Serial Intuos* | GD-xxxx-R | (Protocol V) all tools supported, menu strip works when emulating USB Intuos 2|
| *Serial Intuos2* | XD-xxxx-R | (Protocol V) all tools supported, menu strip works when emulating USB Intuos 2 |
| *Serial Topaz* |  | Protocol implemented but largely untested |

### USB Emulation Status
|   |   |
|---|---|
| *Graphire 3* | Basic functionality works |
| *Intuos2 12x18, 12x12, 9x12, 6x8, 4x5* | All tools and menu strip working. Note that Intuos2 does not have official Windows 8 drivers as of this writing. |
| *Intuos5* | Basic pen functionality. (Limited templates “UD-1212-R to Intuos5 S” only for now. See Intuos5 talk below. |
| *Bamboo Touch* | Early coding stage. Development not far (superseeded by Intuos5 work) |

Latest Template list: (bundled in the distribution zip) <https://github.com/popbee/waxbee/tree/master/WaxBeeConfig/config_templates>

### Waxbee Config tool status

Native libraries not well handled and specific to Windows for now. (code is present for other platforms but not compiled). Native libraries are not needed if you “flash” your Teensy using PRJC.com's Loader utility (use the “generate hex file” option).

### Credits

* Portions of the code from the open source Linux Wacom driver
* Intuos1/2 Tools & serial number support by Chris Dragon of [www.dracoventions.com]
* Thanks to all people helping out!

### Interfacing ADB

Some rough instructions to interface ADB Wacom tablets here: [InterfacingADB]

### USB Serial

This implements a virtual Serial port on the Teensy 2.0 specifically targeted to talk to tablets

  [Teensy]: http://www.pjrc.com/teensy
  [`Dracoventions.com`]: http://www.dracoventions.com
