# Makefile for Lua BitOp -- a bit operations library for Lua 5.1.
# To compile with MSVC please run: msvcbuild.bat
# To compile with MinGW please run: mingw32-make -f Makefile.mingw
# To compile with lua52: run: make LUAVER=5.2.4 macosx

# Include path where lua.h, luaconf.h and lauxlib.h reside:
LUAVER=5.1.5

INCLUDES= -I/mnt/hgfs/workspace/zdc/external/lua-$(LUAVER)/src -I/usr/local/include -I/opt/local/include

# Use this if you need to compile for an old ARM ABI with swapped FPA doubles
#DEFINES= -DSWAPPED_DOUBLE

# Lua executable name. Used to find the install path and for testing.
LUA= lua

CC= $(CROSS_COMPILE)gcc
SOCFLAGS= -fPIC
SOCC= $(CC) -shared $(SOCFLAGS)
CFLAGS= -Wall -O2 -fomit-frame-pointer $(SOCFLAGS) $(DEFINES) $(INCLUDES)
RM= rm -f
INSTALL= install -p
INSTALLPATH= $(LUA) installpath.lua

MODNAME= znettool
MODSO= $(MODNAME).so

all: $(MODSO)

# Alternative target for compiling on Mac OS X:
macosx:
	$(MAKE) all "SOCC=MACOSX_DEPLOYMENT_TARGET=10.3 $(CC) -dynamiclib -single_module -undefined dynamic_lookup $(SOCFLAGS)"

$(MODSO): $(MODNAME).o
	$(SOCC) -o $@ $<

install: $(MODSO)
	#$(INSTALL) $< `$(INSTALLPATH) $(MODNAME)`

test: $(MODSO)
	#@$(LUA) md5test.lua && echo "MD5 test OK"

clean:
	$(RM) *.o *.so *.obj *.lib *.exp *.dll *.manifest

.PHONY: all macosx install test clean

