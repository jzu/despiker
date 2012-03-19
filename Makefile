INSTALL_PLUGINS_DIR = /usr/lib/ladspa/

INCLUDES  = -I.
LIBRARIES = -ldl -lm
CFLAGS    = $(INCLUDES) -Wall -g -fPIC
PLUGIN    = despiker_4741
CC        = /usr/bin/cc
LD        = /usr/bin/ld
INSTALL   = install -m 644

${PLUGIN}.so:
	$(CC) $(CFLAGS) -o ${PLUGIN}.o -c ${PLUGIN}.c
	$(LD) -g -o ${PLUGIN}.so ${PLUGIN}.o -shared ${LIBRARIES}

install: ${PLUGIN}.so
	${INSTALL} ${PLUGIN}.so $(INSTALL_PLUGINS_DIR)

clean:
	-rm -f *.o *.so *~ core*


