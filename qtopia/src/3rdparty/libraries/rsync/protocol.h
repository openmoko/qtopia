/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * 
 * librsync -- library for network deltas
 * $Id: protocol.h,v 1.1 2001/09/16 17:20:32 lknoll Exp $
 * 
 * Copyright (C) 1999, 2000, 2001 by Martin Pool <mbp@samba.org>
 * Copyright (C) 1999 by Andrew Tridgell
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


/* 
 * TODO: Have a way to copy from the old signature into the new
 * one. This will be useful for the case where the files are in fact
 * identical, which will be significantly common.
 */


                          /*
                           | "The IETF already has more than enough
                           | RFCs that codify the obvious, make
                           | stupidity illegal, support truth,
                           | justice, and the IETF way, and generally
                           | demonstrate the author is a brilliant and
                           | valuable Contributor to The Standards
                           | Process."
                           |     -- Vernon Schryver
                           */



#define RS_DELTA_MAGIC          0x72730236      /* r s \2 6 */
#define RS_SIG_MAGIC            0x72730136      /* r s \1 6 */
