/*********************************************************************
 *                
 * Filename:      obex_transport.c
 * Version:       
 * Description:   Code to handle different types of transports
 * Status:        Experimental.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Sat May  1 20:15:04 1999
 * CVS ID:        $Id: obex_transport.c,v 1.15 2001/02/27 15:00:39 pof Exp $
 * 
 *     Copyright (c) 1999, 2000 Pontus Fuchs, All Rights Reserved.
 *     Copyright (c) 1999, 2000 Dag Brattli, All Rights Reserved.
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

#include <string.h>

#include "config.h"

#include <stdio.h>

#include "obex_main.h"
#ifdef HAVE_IRDA
#include "irobex.h"
#endif /*HAVE_IRDA*/
#include "inobex.h"

#include "obex_transport.h"

/*
 * Function obex_transport_handle_input(self, timeout)
 *
 *    Used when working in synchronous mode.
 *
 */
gint obex_transport_handle_input(obex_t *self, gint timeout)
{
	int ret;
	
	if(self->trans.type == OBEX_TRANS_CUST) {
		if(self->ctrans.handleinput)
			ret = self->ctrans.handleinput(self, self->ctrans.userdata, timeout);
		else {
			g_message(G_GNUC_FUNCTION "() No handleinput-callback exist!\n");
			ret = -1;
		}
	}
	else {
		struct timeval time;
		fd_set fdset;
		int highestfd = 0;
	
		DEBUG(4, G_GNUC_FUNCTION "()\n");
		g_return_val_if_fail(self != NULL, -1);

		/* Check of we have any fd's to do select on. */
		if(self->fd < 0 && self->serverfd < 0) {
			DEBUG(0, G_GNUC_FUNCTION "() No valid socket is open\n");
			return -1;
		}

		time.tv_sec = timeout;
		time.tv_usec = 0;

		/* Add the fd's to the set. */
		FD_ZERO(&fdset);
		if(self->fd >= 0) {
			FD_SET(self->fd, &fdset);
				highestfd = self->fd;
		}
		if(self->serverfd >= 0) {
			FD_SET(self->serverfd, &fdset);
			if(self->serverfd > highestfd)
				highestfd = self->serverfd;
		}

		/* Wait for input */
		ret = select(highestfd+1, &fdset, NULL, NULL, &time);
	
		/* Check if this is a timeout (0) or error (-1) */
		if (ret < 1)
			return ret;
	
		if( (self->fd >= 0) && FD_ISSET(self->fd, &fdset)) {
			DEBUG(4, G_GNUC_FUNCTION "() Data available on client socket\n");
			ret = obex_data_indication(self, NULL, 0);
		}

		else if( (self->serverfd >= 0) && FD_ISSET(self->serverfd, &fdset)) {
			DEBUG(4, G_GNUC_FUNCTION "() Data available on server socket\n");
			/* Accept : create the connected socket */
			ret = obex_transport_accept(self);

			/* Tell the app to perform the OBEX_Accept() */
			if(self->keepserver)
				obex_deliver_event(self, OBEX_EV_ACCEPTHINT,
						   0, 0, FALSE);
			/* Otherwise, just disconnect the server */
			if((ret >= 0) && (! self->keepserver)) {
				obex_transport_disconnect_server(self);
			}
		}
		else
			ret = -1;
	}
	return ret;
}


/*
 * Function obex_transport_accept(self)
 *
 *    Accept an incoming connection.
 *
 */
gint obex_transport_accept(obex_t *self)
{
	gint ret = -1;

	DEBUG(4, G_GNUC_FUNCTION "()\n");

	switch (self->trans.type) {
#ifdef HAVE_IRDA
	case OBEX_TRANS_IRDA:
		ret = irobex_accept(self);
		break;
#endif /*HAVE_IRDA*/
	case OBEX_TRANS_INET:
		ret = inobex_accept(self);
		break;

	default:
		g_message(G_GNUC_FUNCTION "(), domain not implemented!\n");
		break;
	}
	return ret;
}


/*
 * Function obex_transport_connect_request (self, service)
 *
 *    Try to connect transport
 *
 */
gint obex_transport_connect_request(obex_t *self)
{
	gint ret = -1;


	if(self->trans.connected)
		return 1;

	switch (self->trans.type) {
#ifdef HAVE_IRDA
	case OBEX_TRANS_IRDA:
		ret = irobex_connect_request(self);
		break;
#endif /*HAVE_IRDA*/
	case OBEX_TRANS_INET:
		ret = inobex_connect_request(self);
		break;
	case OBEX_TRANS_CUST:
		DEBUG(4, G_GNUC_FUNCTION "() Custom connect\n");
		if(self->ctrans.connect)
			ret = self->ctrans.connect(self, self->ctrans.userdata);
		else
			g_message(G_GNUC_FUNCTION "(), No connect-callback exist!\n");
		DEBUG(4, G_GNUC_FUNCTION "ret=%d\n", ret);
		break;

	default:
		g_message(G_GNUC_FUNCTION "() Transport not implemented!\n");
		break;
	}
	if (ret >= 0)
		self->trans.connected = TRUE;

	return ret;
}

/*
 * Function obex_transport_disconnect_request (self)
 *
 *    Disconnect transport
 *
 */
void obex_transport_disconnect_request(obex_t *self)
{

	switch (self->trans.type) {
#ifdef HAVE_IRDA
	case OBEX_TRANS_IRDA:
		irobex_disconnect_request(self);
		break;
#endif /*HAVE_IRDA*/
	case OBEX_TRANS_INET:
		inobex_disconnect_request(self);
		break;	
	case OBEX_TRANS_CUST:
		DEBUG(4, G_GNUC_FUNCTION "() Custom disconnect\n");
		if(self->ctrans.disconnect)
			self->ctrans.disconnect(self, self->ctrans.userdata);
		else
			g_message(G_GNUC_FUNCTION "(), No disconnect-callback exist!\n");
		break;
	default:
		g_message(G_GNUC_FUNCTION "() Transport not implemented!\n");
		break;
	}
	self->trans.connected = FALSE;
}

/*
 * Function obex_transport_listen (self, service)
 *
 *    Prepare for incomming connections
 *
 */
gint obex_transport_listen(obex_t *self, const char *service)
{
	int ret = -1;

	switch (self->trans.type) {
#ifdef HAVE_IRDA
	case OBEX_TRANS_IRDA:
		ret = irobex_listen(self, service);
		break;
#endif /*HAVE_IRDA*/
	case OBEX_TRANS_INET:
		ret = inobex_listen(self, service);
		break;
	case OBEX_TRANS_CUST:
		DEBUG(4, G_GNUC_FUNCTION "() Custom listen\n");
		if(self->ctrans.listen)
			ret = self->ctrans.listen(self, self->ctrans.userdata);
		else
			g_message(G_GNUC_FUNCTION "(), No listen-callback exist!\n");
		break;
	default:
		g_message(G_GNUC_FUNCTION "() Transport not implemented!\n");
		break;
	}
	return ret;
}
	
/*
 * Function obex_transport_disconnect_server (self)
 *
 *    Disconnect the listening server
 *
 * Used either after an accept, or directly at client request (app. exits)
 * Note : obex_delete_socket() will catch the case when the socket
 * doesn't exist (-1)...
 */
void obex_transport_disconnect_server(obex_t *self)
{

	switch (self->trans.type) {
#ifdef HAVE_IRDA
	case OBEX_TRANS_IRDA:
		irobex_disconnect_server(self);
		break;
#endif /*HAVE_IRDA*/
	case OBEX_TRANS_INET:
		inobex_disconnect_server(self);
		break;	
	case OBEX_TRANS_CUST:
		DEBUG(4, G_GNUC_FUNCTION "() Custom disconnect\n");
		break;
	default:
		g_message(G_GNUC_FUNCTION "() Transport not implemented!\n");
		break;
	}
}

/*
 * Function obex_transport_write ()
 *
 *    Do the writing
 *
 */
gint obex_transport_write(obex_t *self, GNetBuf *msg)
{
	gint actual = -1;
	gint size;

	DEBUG(4, G_GNUC_FUNCTION "()\n");

	switch(self->trans.type)	{
#ifdef HAVE_IRDA
	case OBEX_TRANS_IRDA:
#endif /*HAVE_IRDA*/
	case OBEX_TRANS_INET:
		/* Send and fragment if necessary  */
		while (msg->len) {
			if (msg->len > self->trans.mtu)
				size = self->trans.mtu;
			else
				size = msg->len;
			DEBUG(1, G_GNUC_FUNCTION "(), sending %d bytes\n", size);

			actual = send(self->fd, msg->data, size, 0);

			if (actual != size) {
				perror("send");
				return actual;
			}
			/* Hide sent data */
			g_netbuf_pull(msg, size);
		}
		break;
	case OBEX_TRANS_CUST:
		DEBUG(4, G_GNUC_FUNCTION "() Custom write\n");
		if(self->ctrans.write)
			actual = self->ctrans.write(self, self->ctrans.userdata, msg->data, msg->len);
		else
			g_message(G_GNUC_FUNCTION "(), No write-callback exist!\n");
		break;
	default:
		g_message(G_GNUC_FUNCTION "() Transport not implemented!\n");
		break;
	}	
	return actual;
}

/*
 * Function obex_transport_read ()
 *
 *    Do the reading
 *
 */
gint obex_transport_read(obex_t *self, gint max, guint8 *buf, gint buflen)
{
	gint actual = -1;
	GNetBuf *msg = self->rx_msg;

	DEBUG(4, G_GNUC_FUNCTION "() Request to read max %d bytes\n", max);

	switch(self->trans.type)	{
#ifdef HAVE_IRDA
	case OBEX_TRANS_IRDA:
#endif /*HAVE_IRDA*/
	case OBEX_TRANS_INET:
		actual = recv(self->fd, msg->tail, max, 0);
		break;
	case OBEX_TRANS_CUST:
		if(buflen > max) {
			memcpy(msg->tail, buf, max);
			actual = max;
		}
		else {
			memcpy(msg->tail, buf, buflen);
			actual = buflen;
		}
		break;
	default:
		g_message(G_GNUC_FUNCTION "() Transport not implemented!\n");
		break;
	}	
	return actual;
}


