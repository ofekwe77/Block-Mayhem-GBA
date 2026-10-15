#---------------------------------------------------------------------------------------------------------------------
# BLOCK MAYHEM: THE SPICE PROTOCOL - Game Boy Advance build (Butano + devkitARM).
#
# Build from an MSYS2 shell with devkitPro installed:   make -j8
# On Windows use build.ps1, which maps the project to an ASCII-only drive first
# (GNU make and gcc cannot handle spaces / non-ASCII characters in paths).
#
# LIBBUTANO can be overridden from the environment: make LIBBUTANO=/path/to/butano/butano
#---------------------------------------------------------------------------------------------------------------------
TARGET      	:=  BlockMayhem
BUILD       	:=  build
LIBBUTANO   	?=  /opt/butano/butano
PYTHON      	:=  python3
SOURCES     	:=  src core
INCLUDES    	:=  include core
DATA        	:=
GRAPHICS    	:=  graphics
AUDIO       	:=  audio
AUDIOBACKEND	:=  maxmod
AUDIOTOOL		:=
DMGAUDIO    	:=  dmg_audio
DMGAUDIOBACKEND	:=  default
ROMTITLE    	:=  BLOCKMAYHEM
ROMCODE     	:=  BMSP
USERFLAGS   	:=  -DBM_GBA=1
USERCXXFLAGS	:=
USERASFLAGS 	:=
USERLDFLAGS 	:=
USERLIBDIRS 	:=
USERLIBS    	:=
DEFAULTLIBS 	:=
STACKTRACE		:=
USERBUILD   	:=
EXTTOOL     	:=

#---------------------------------------------------------------------------------------------------------------------
# Export absolute butano path:
#---------------------------------------------------------------------------------------------------------------------
ifndef LIBBUTANOABS
	export LIBBUTANOABS	:=	$(realpath $(LIBBUTANO))
endif

#---------------------------------------------------------------------------------------------------------------------
# Include main makefile:
#---------------------------------------------------------------------------------------------------------------------
include $(LIBBUTANOABS)/butano.mak
