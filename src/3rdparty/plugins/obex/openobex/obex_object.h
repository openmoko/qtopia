/*********************************************************************
 *                
 * Filename:      obex_object.h
 * Version:       0.9
 * Description:   
 * Status:        Stable.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Fri Apr 23 14:05:28 1999
 * CVS ID:        $Id: obex_object.h,v 1.9 2001/02/27 15:00:39 pof Exp $
 * 
 *     Copyright (c) 1999, 2000 Dag Brattli, All Rights Reserved.
 *     Copyright (c) 1999, 2000 Pontus Fuchs, All Rights Reserved.
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

#ifndef OBEX_OBJECT_H
#define OBEX_OBJECT_H

#include "obex_main.h"
#include "netbuf.h"

/* If an object has no expected length we have to 
reallocated every OBEX_OBJECT_ALLOCATIONTRESHOLD bytes */
#define OBEX_OBJECT_ALLOCATIONTRESHOLD 10240

struct obex_header_element {
	GNetBuf *buf;
	guint8 hi;
	guint length;
	guint offset;
	gboolean body_touched;
	gboolean stream;
};

typedef struct {
        time_t time;

	GSList *tx_headerq;		/* List of headers to transmit*/
	GSList *rx_headerq;		/* List of received headers */
	GSList *rx_headerq_rm;		/* List of recieved header already read by the app */
	GNetBuf *rx_body;		/* The rx body header need some extra help */
	GNetBuf *tx_nonhdr_data;	/* Data before of headers (like CONNECT and SETPATH) */
	GNetBuf *rx_nonhdr_data;	/* -||- */

	guint8 cmd;			/* The command of this object */
					
					/* The opcode fields are used as
					   command when sending and response
					   when recieving */

	guint8 opcode;			/* Opcode for normal packets */
	guint8 lastopcode;		/* Opcode for last packet */
	guint headeroffset;		/* Where to start parsing headers */

	gint hinted_body_len;		/* Hinted body-length or 0 */
	gint totallen;			/* Size of all headers */
        gboolean abort;			/* Request shall be aborted */
	
	const guint8 *s_buf;		/* Pointer to streaming data */
	guint s_len;			/* Length of stream-data */
        guint s_offset;			/* Current offset in buf */
        gboolean s_stop;		/* End of stream */
        gboolean s_srv;			/* Deliver body as stream when server */

} obex_object_t;


obex_object_t *obex_object_new(void);
gint obex_object_delete(obex_object_t *object);
gint obex_object_addheader(obex_t *self, obex_object_t *object, guint8 hi,
				obex_headerdata_t hv, guint32 hv_size,
				guint flags);
gint obex_object_getnextheader(obex_t *self, obex_object_t *object, guint8 *hi,
				obex_headerdata_t *hv, guint32 *hv_size);
gint obex_object_reparseheaders(obex_t *self, obex_object_t *object);
gint obex_object_setcmd(obex_object_t *object, guint8 cmd, guint8 lastcmd);
gint obex_object_setrsp(obex_object_t *object, guint8 rsp, guint8 lastrsp);
gint obex_object_send(obex_t *self, obex_object_t *object, gint allowfinal);
gint obex_object_receive(obex_t *self, GNetBuf *msg);
gint obex_object_readstream(obex_t *self, obex_object_t *object, const guint8 **buf);

#endif
