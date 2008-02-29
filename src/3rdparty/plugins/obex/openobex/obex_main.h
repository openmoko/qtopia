/*********************************************************************
 *                
 * Filename:      obex_main.h
 * Version:       0.9
 * Description:   
 * Status:        Experimental.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Mon Jul 20 22:28:23 1998
 * CVS ID:        $Id: obex_main.h,v 1.14 2000/12/07 15:01:04 pof Exp $
 * 
 *     Copyright (c) 1999, 2000 Pontus Fuchs, All Rights Reserved.
 *     Copyright (c) 1998, 1999, 2000 Dag Brattli, All Rights Reserved.
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

#ifndef OBEX_MAIN_H
#define OBEX_MAIN_H

#include <time.h>

#ifndef _WIN32
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

/* Forward decl */
typedef struct obex obex_t;

#include "obex_const.h"
#include "obex_object.h"
#include "obex_transport.h"
#include "netbuf.h"

/* use 0 for none, 1 for sendbuff, 2 for receivebuff and 3 for both */
#ifndef DEBUG_DUMPBUFFERS
#define DEBUG_DUMPBUFFERS 0
#endif

/* use 0 for production, 1 for verification, >2 for debug */
#ifdef OBEX_DEBUG
unsigned int obex_debug;
#endif

#ifndef _WIN32

#ifdef OBEX_DEBUG
#define DEBUG(n, args...) if (obex_debug >= (n)) g_print(args)
#else
#define DEBUG(n, args, ...)
#endif /* OBEX_DEBUG */

#else /* _WIN32 */

void DEBUG(unsigned int n, ...);

#endif /* _WIN32 */

#define OBEX_VERSION		0x11      /* Version 1.1 */
#define OBEX_DEFAULT_MTU	1024
#define OBEX_MINIMUM_MTU	255      

// Note that this one is also defined in obex.h
typedef void (*obex_event_t)(obex_t *handle, obex_object_t *obj, gint mode, gint event, gint obex_cmd, gint obex_rsp);

#define MODE_SRV	0x80
#define MODE_CLI	0x00

enum
{
	STATE_IDLE,
	STATE_START,
	STATE_SEND,
	STATE_REC,
};

struct obex {
	guint16 mtu_tx;			/* Maximum OBEX TX packet size */
        guint16 mtu_rx;			/* Maximum OBEX RX packet size */

	gint fd;			/* Socket descriptor */
	gint serverfd;
        guint state;
	
	gboolean keepserver;		/* Keep server alive */
	gboolean filterhint;		/* Filter devices based on hint bits */
	gboolean filterias;		/* Filter devices based on IAS entry */

	GNetBuf *tx_msg;		/* Reusable transmit message */
	GNetBuf *rx_msg;		/* Reusable receive message */

	obex_object_t	*object;	/* Current object being transfered */      
	obex_event_t	eventcb;	/* Event-callback */

	obex_transport_t trans;		/* Transport being used */
	obex_ctrans_t ctrans;
	gpointer userdata;		/* For user */
};


gint obex_create_socket(obex_t *self, gint domain);
gint obex_delete_socket(obex_t *self, gint fd);

void obex_deliver_event(obex_t *self, gint event, gint cmd, gint rsp, gboolean del);
gint obex_data_indication(obex_t *self, guint8 *buf, gint buflen);

void obex_response_request(obex_t *self, guint8 opcode);
gint obex_data_request(obex_t *self, GNetBuf *msg, gint opcode);
gint obex_cancelrequest(obex_t *self, gboolean nice);

GString *obex_get_response_message(obex_t *self, gint rsp);

#endif
