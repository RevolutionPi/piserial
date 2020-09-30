#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Release

#Toolchain
CC := C:/SysGCC/Raspberrystretch/bin/arm-linux-gnueabihf-gcc.exe
CXX := C:/SysGCC/Raspberrystretch/bin/arm-linux-gnueabihf-g++.exe
LD := $(CXX)
AR := C:/SysGCC/Raspberrystretch/bin/arm-linux-gnueabihf-ar.exe
OBJCOPY := C:/SysGCC/Raspberrystretch/bin/arm-linux-gnueabihf-objcopy.exe

#Additional flags
PREPROCESSOR_MACROS := NDEBUG RELEASE __KUNBUSPI__ __USE_POSIX199309
INCLUDE_DIRS := . ..\..\..\..\platform\wiringPi ..\..\..\..\platformFbus\common\sw ..\..\..\..\platformFbus\compiler\sw\gnuArm ..\..\..\..\platformFbus\bsp\sw\bsp\spi ..\..\..\..\platformFbus\bsp\LinuxRT\sw ..\..\..\..\platformFbus\bsp\sw ..\..\..\..\platformFbus\ModGateCom\sw ..\..\..\..\platformFbus\utilities\sw
LIBRARY_DIRS := 
LIBRARY_NAMES := rt
ADDITIONAL_LINKER_INPUTS := ..\..\..\..\platform\lib\libwiringPi.so ..\..\..\..\platform\lib\libwiringPiDev.so
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O3
CXXFLAGS := -ggdb -ffunction-sections -O3
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS := 
LINKER_SCRIPT := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
USE_DEL_TO_CLEAN := 1
CP_NOT_AVAILABLE := 1
IS_LINUX_PROJECT := 1
