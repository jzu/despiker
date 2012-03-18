INSTALL_PLUGINS_DIR	=	/usr/lib/ladspa/
INSTALL_INCLUDE_DIR	=	/usr/include/
INSTALL_BINARY_DIR	=	/usr/bin/

INCLUDES	=	-I.
LIBRARIES	=	-ldl -lm
CFLAGS		=	$(INCLUDES) -Wall -g -fPIC
CXXFLAGS	=	$(CFLAGS)
PLUGIN		=	despiker_4741
CC			=	cc
INSTALL		=	install -m 644

${PLUGIN}.so:
	$(CC) $(CFLAGS) -o ${PLUGIN}.o -c ${PLUGIN}.c
	$(LD) -g -o ${PLUGIN}.so ${PLUGIN}.o -shared

install:	${PLUGIN}.so
	${INSTALL} ${PLUGIN}.so $(INSTALL_PLUGINS_DIR)

clean:
	-rm -f *.o *.so *~ core*


