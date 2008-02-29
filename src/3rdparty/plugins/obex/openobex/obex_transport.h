/*********************************************************************
 *                
 * Filename:      obex_transport.h
 * Version:       
 * Description:   
 * Status:        Experimental.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Sat May  1 20:16:45 1999
 * CVS ID         $Id: obex_transport.h,v 1.7 2000/12/04 16:01:16 pof Exp $
 * 
 *     Copyright (c) 1999 Dag Brattli, All Rights Reserved.
 *     
 *     This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU Lesser General Public
 *     License as published by the Free Software Foundation; either
 *     version 2 of the License, or (at your option) any later version.
 *
 *     This library is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *     Lesser General Public License for more details.
 *
 *     You should have received a copy of the GNU Lesser General Public
 *     License along with this library; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *     MA  02111-1307  USA
 *     
 ********************************************************************/

#ifndef OBEX_TRANSPORT_H
#define OBEX_TRANSPORT_H

#ifdef _WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#ifdef HAVE_IRDA
#include "irda_wrap.h"
#endif /*HAVE_IRDA*/

#include "obex_main.h"

typedef union {
#ifdef HAVE_IRDA
	struct sockaddr_irda irda;
#endif /*HAVE_IRDA*/
	struct sockaddr_in   inet;
} saddr_t;

typedef struct obex_transport {
	gint	type;
     	gboolean connected;	/* Link connection state */
     	guint	mtu;		/* Tx MTU of the link */
	saddr_t	self;		/* Source address */
	saddr_t	peer;		/* Destination address */

} obex_transport_t;

gint obex_transport_accept(obex_t *self);

gint obex_transport_handle_input(obex_t *self, gint timeout);
gint obex_transport_connect_request(obex_t *self);
void obex_transport_disconnect_request(obex_t *self);
gint obex_transport_listen(obex_t *self, const char *service);
void obex_transport_disconnect_server(obex_t *self);
gint obex_transport_write(obex_t *self, GNetBuf *msg);
gint obex_transport_read(obex_t *self, gint count, guint8 *buf, gint buflen);


#endif /* OBEX_TRANSPORT_H */



