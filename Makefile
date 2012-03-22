INSTALL_PLUGINS_DIR = /usr/lib/ladspa/

INCLUDES  = -I.
LIBRARIES = -ldl -lm
CFLAGS    = $(INCLUDES) -Wall -g -fPIC
PLUGIN    = despiker_4741
CC        = /usr/bin/cc
LD        = /usr/bin/ld
INSTALL   = install -m 644

SYSTEM    = $(shell uname)

ifeq (${SYSTEM},Linux)
LDFLAGS   = -g -shared
else
ifeq (${SYSTEM},Darwin)
LDFLAGS   = -dylib
else
LDFLAGS   = "NOT SUPPORTED"
endif
endif

${PLUGIN}.so: ${PLUGIN}.c
	$(CC) $(CFLAGS) -o ${PLUGIN}.o -c ${PLUGIN}.c
	$(LD) -o ${PLUGIN}.so ${PLUGIN}.o ${LDFLAGS} ${LIBRARIES}

install: ${PLUGIN}.so
	${INSTALL} ${PLUGIN}.so $(INSTALL_PLUGINS_DIR)

clean:
	-rm -f *.o *.so *~ core*

deinstall:
	-rm -f $(INSTALL_PLUGINS_DIR)${PLUGIN}.so
