TEMPLATE	= lib
CONFIG		+= qtopia warn_on
HEADERS = inobex.h    \	
	irda.h 	      \	
	irda_wrap.h   \	
	irobex.h      \	
	netbuf.h      \	
	obex.h	      \	
	obex_client.h \	
	obex_connect.h \	
	obex_const.h  \	
	obex_header.h \	
	obex_main.h   \	
	obex_object.h \	
	obex_server.h \	
	obex_transport.h


SOURCES = inobex.c     \
	irobex.c       \
	netbuf.c       \
	obex.c	       \
	obex_client.c  \
	obex_connect.c \
	obex_header.c  \
	obex_main.c    \
	obex_object.c  \
	obex_server.c  \
	obex_transport.c


INCLUDEPATH	+= .
DESTDIR		= $(QPEDIR)/lib
VERSION		= 0.9.8
