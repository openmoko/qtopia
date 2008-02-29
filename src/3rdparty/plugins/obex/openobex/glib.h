/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

/*
 * The following code is based on the source of GLib, but is just
 * the minimum code needed to compile OpenObex without needing to
 * link with the whole of GLib.
 * Some of this code is copied from GLib, or loosely based on code from GLib.
 */

#ifndef __QT_COMPATIBLE_OBEX_GLIB_H__
#define __QT_COMPATIBLE_OBEX_GLIB_H__


#include "glibconfig.h"
#include "stdlib.h"
#include "stdio.h"


#define xDebug( b... )		    fprintf( stderr, b )
#define FALSE			    0
#define TRUE			    1
#ifdef DEBUG
#  undef DEBUG
#endif
#define DEBUG( a, b... )	    xDebug( b );
#define	G_GNUC_FUNCTION		    __FUNCTION__
#define g_message		    xDebug
#define g_print			    xDebug
#define g_new0( type, count )	    ((type *)calloc((count),sizeof(type)))
//#define g_memmove(d,s,n)	    memmove((d),(s),(n))

// Disable checks
#define g_return_if_fail(expr)
#define g_return_val_if_fail(expr,val)


typedef char		gchar;
typedef short		gshort;
typedef long		glong;
typedef int		gint;
typedef int		gboolean;
typedef unsigned char	guchar;
typedef unsigned short	gushort;
typedef unsigned long	gulong;
typedef unsigned int	guint;
typedef void*		gpointer;
typedef const void*	gconstpointer;
typedef struct _GSList	GSList;
typedef struct _GString	GString; 


struct _GSList {
    gpointer data;
    GSList *next;
};


struct _GString {
    gchar *str;
    gint len;
};


static inline gpointer g_malloc( guint n_bytes ) {
    return ( n_bytes ) ? malloc( n_bytes ) : NULL;
}


static inline gpointer g_malloc0( guint n_bytes ) {
    return ( n_bytes ) ? calloc( 1, n_bytes ) : NULL;
}


static inline void g_free( gpointer mem ) {
    if ( mem )
	free( mem );
}


static inline GString *g_string_new( const gchar *init ) {
    GString *string = (GString *)malloc( sizeof( GString ) );
    string->len = init ? strlen( init ) : 0;
    string->str = (gchar *)malloc( string->len + 2 );
    if ( init )
	memcpy( string->str, init, string->len );
    return string;
}


static inline GSList *g_slist_remove( GSList *list, gconstpointer data ) {
    GSList *tmp = list, *prev = NULL;
    while ( tmp ) {
	if ( tmp->data == data ) {
	    if ( prev )
		prev->next = tmp->next;
	    else
		list = tmp->next;
	    free( tmp );
	    break;
	}
	prev = tmp;
	tmp = prev->next;
    }
    return list;
}


static inline GSList *g_slist_append( GSList *list, gpointer data ) {
    GSList *new_list = (GSList *)malloc( sizeof( GSList ) );
    new_list->data = data;
    new_list->next = NULL;
    if ( list ) {
	GSList *last = list;
	while ( last->next )
	    last = last->next; // YUCK! GLib lists get completely traversed on every append
	last->next = new_list;
	return list;
    }
    return new_list;
}


#endif // __QT_COMPATIBLE_OBEX_GLIB_H__

