/*********************************************************************
 *                
 * Filename:      obex_main.c
 * Version:       0.9
 * Description:   Implementation of the Object Exchange Protocol OBEX
 * Status:        Experimental.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Fri Jul 17 23:02:02 1998
 * CVS ID:        $Id: obex_main.c,v 1.16 2000/12/07 15:01:04 pof Exp $
 * 
 *     Copyright (c) 2000 Pontus Fuchs, All Rights Reserved.
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

#include <config.h>
#ifdef _WIN32
#include <winsock.h>
#else /* _WIN32 */

#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#endif /* _WIN32 */

#include "obex_main.h"
#include "obex_header.h"
#include "obex_server.h"
#include "obex_client.h"
#include "obex_const.h"


/*
 * Function obex_create_socket()
 *
 *    Create socket if needed.
 *
 */
gint obex_create_socket(obex_t *self, gint domain)
{
	gint fd;
	DEBUG(4, G_GNUC_FUNCTION "()\n");

	fd = socket(domain, SOCK_STREAM, 0);
	return fd;
}

/*
 * Function obex_delete_socket()
 *
 *    Close socket if opened.
 *
 */
gint obex_delete_socket(obex_t *self, gint fd)
{
	gint ret;

	DEBUG(4, G_GNUC_FUNCTION "()\n");

	if(fd < 0)
		return fd;

#ifdef _WIN32
	ret = closesocket(fd);
#else /* _WIN32 */
	ret = close(fd);
#endif /* _WIN32 */
	return ret;
}


/*
 * Function obex_get_last_response_message (self)
 *
 *    Return a GSting of an OBEX-response
 *
 */
GString *obex_get_response_message(obex_t *self, gint rsp)
{
	GString *string;

	g_return_val_if_fail(self != NULL, NULL);

	switch (rsp) {
	case OBEX_RSP_CONTINUE:
		string = g_string_new("Continue");
		break;
	case OBEX_RSP_SWITCH_PRO:
		string = g_string_new("Switching protocols");
		break;
	case OBEX_RSP_SUCCESS:
		string = g_string_new("OK, Success");
		break;
	case OBEX_RSP_CREATED:
		string = g_string_new("Created");
		break;
	case OBEX_RSP_ACCEPTED:
		string = g_string_new("Accepted");
		break;	
	case OBEX_RSP_NO_CONTENT:
		string = g_string_new("No Content");
		break;
	case OBEX_RSP_BAD_REQUEST:
		string = g_string_new("Bad Request");
		break;
	case OBEX_RSP_UNAUTHORIZED:
		string = g_string_new("Unautorized");
		break;
	case OBEX_RSP_PAYMENT_REQUIRED:
		string = g_string_new("Payment required");
		break;
	case OBEX_RSP_FORBIDDEN:
		string = g_string_new("Forbidden");
		break;
	case OBEX_RSP_NOT_FOUND:
		string = g_string_new("Not found");
		break;
	case OBEX_RSP_METHOD_NOT_ALLOWED:
		string = g_string_new("Method not allowed");
		break;
	case OBEX_RSP_CONFLICT:
		string = g_string_new("Conflict");
		break;
	case OBEX_RSP_INTERNAL_SERVER_ERROR:
		string = g_string_new("Internal server error");
		break;
	case OBEX_RSP_NOT_IMPLEMENTED:
		string = g_string_new("Not implemented!");
		break;
	case OBEX_RSP_DATABASE_FULL:
		string = g_string_new("Database full");
		break;
	case OBEX_RSP_DATABASE_LOCKED:
		string = g_string_new("Database locked");
		break;
	default:
		string = g_string_new("Unknown response");
		break;
	}
	return string;
}

/*
 * Function obex_deliver_event ()
 *
 *    Deliver an event to app.
 *
 */
void obex_deliver_event(obex_t *self, gint event, gint cmd, gint rsp, gboolean del)
{
	if(self->state & MODE_SRV)
		self->eventcb(self, self->object, OBEX_SERVER, event, cmd, rsp);
	else
		self->eventcb(self, self->object, OBEX_CLIENT, event, cmd, rsp);
	
	if(del == TRUE && self->object != NULL) {
		obex_object_delete(self->object);
		self->object = NULL;
	}
}

/*
 * Function obex_response_request (self, opcode)
 *
 *    Send a response to peer device
 *
 */
void obex_response_request(obex_t *self, guint8 opcode)
{
	GNetBuf *msg;

	g_return_if_fail(self != NULL);

	msg = g_netbuf_recycle(self->tx_msg);
	g_netbuf_reserve(msg, sizeof(obex_common_hdr_t));

	obex_data_request(self, msg, opcode | OBEX_FINAL);
}

/*
 * Function obex_data_request (self, opcode, cmd)
 *
 *    Send response or command code along with optional headers/data.
 *
 */
gint obex_data_request(obex_t *self, GNetBuf *msg, int opcode)
{
	obex_common_hdr_t *hdr;
	int actual = 0;

	g_return_val_if_fail(self != NULL, -1);
	g_return_val_if_fail(msg != NULL, -1);

	/* Insert common header */
	hdr = (obex_common_hdr_t *) g_netbuf_push(msg, sizeof(obex_common_hdr_t));

	hdr->opcode = opcode;
	hdr->len = htons((guint16)msg->len);

#if DEBUG_DUMPBUFFERS & 1
	g_netbuf_print(msg);
#endif
	DEBUG(1, G_GNUC_FUNCTION "(), len = %d bytes\n", msg->len);

	actual = obex_transport_write(self, msg);
	return actual;
}

/*
 * Function obex_data_indication (self)
 *
 *    Read/Feed some input from device and find out which packet it is
 *
 */
gint obex_data_indication(obex_t *self, guint8 *buf, gint buflen)
{
	obex_common_hdr_t *hdr;
	GNetBuf *msg;
	gint final;
	gint actual = 0;
	guint size;
	
	DEBUG(4, G_GNUC_FUNCTION "()\n");

	g_return_val_if_fail(self != NULL, -1);

	msg = self->rx_msg;
	
	/* First we need 3 bytes to be able to know how much data to read */
	if(msg->len < 3)  {
		actual = obex_transport_read(self, 3 - (msg->len), buf, buflen);
		
		DEBUG(4, G_GNUC_FUNCTION "() Got %d bytes\n", actual);

		/* Check if we are still connected */
		if (actual <= 0)	{
			obex_deliver_event(self, OBEX_EV_LINKERR, 0, 0, TRUE);
			return actual;
		}
		buf += actual;
		buflen -= actual;
		g_netbuf_put(msg, actual);
	}

	/* If we have 3 bytes data we can decide how big the packet is */
	if(msg->len >= 3) {
		hdr = (obex_common_hdr_t *) msg->data;
		size = ntohs(hdr->len);

		actual = 0;
		if(msg->len != (gint) ntohs(hdr->len)) {

			actual = obex_transport_read(self, size - msg->len, buf,
				buflen);

			/* Check if we are still connected */
			if (actual <= 0)	{
				obex_deliver_event(self, OBEX_EV_LINKERR, 0, 0, TRUE);
				return actual;
			}
		}
	}
        else {
		/* Wait until we have at least 3 bytes data */
		DEBUG(3, G_GNUC_FUNCTION "() Need at least 3 bytes got only %d!\n", msg->len);
		return actual;
        }


	/* New data has been inserted at the end of message */
	g_netbuf_put(msg, actual);
	DEBUG(1, G_GNUC_FUNCTION "() Got %d bytes msg len=%d\n", actual, msg->len);

	/*
	 * Make sure that the buffer we have, actually has the specified
	 * number of bytes. If not the frame may have been fragmented, and
	 * we will then need to read more from the socket.  
	 */

	/* Make sure we have a whole packet */
	if (size > msg->len) {
		DEBUG(3, G_GNUC_FUNCTION "() Need more data, size=%d, len=%d!\n",
		      size, msg->len);

		/* I'll be back! */
		return msg->len;
	}

#if DEBUG_DUMPBUFFERS & 2
	g_netbuf_print(msg);
#endif

	actual = msg->len;
	final = hdr->opcode & OBEX_FINAL; /* Extract final bit */

	/* Dispatch to the mode we are in */
	if(self->state & MODE_SRV) {
		obex_server(self, msg, final);
		g_netbuf_recycle(msg);
		
	}
	else	{
		obex_client(self, msg, final);
		g_netbuf_recycle(msg);
	}

	return actual;
}

/*
 * Function obex_cancel_request ()
 *
 *    Cancel an ongoing request
 *
 */
gint obex_cancelrequest(obex_t *self, gboolean nice)
{
	/* If we have no ongoing request do nothing */
	if(self->object == NULL)
		return 0;

	/* Abort request without sending abort */
	if(!nice) {
		/* Deliver event will delete the object */
		obex_deliver_event(self, OBEX_EV_ABORT, 0, 0, TRUE);
		g_netbuf_recycle(self->tx_msg);
		g_netbuf_recycle(self->rx_msg);
		/* Since we didn't send ABORT to peer we are out of sync
		   and need to disconnect transport immediately, so we signal
		   link error to app */
		obex_deliver_event(self, OBEX_EV_LINKERR, 0, 0, FALSE);
		return 1;
	}
	
	/* Do a "nice" abort */
	g_message("Nice abort not implemented yet!!\n");
	self->object->abort = TRUE;
	return 0;
}
