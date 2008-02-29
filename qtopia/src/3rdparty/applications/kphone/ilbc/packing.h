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

   /******************************************************************

       iLBC Speech Coder ANSI-C Source Code

       packing.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #ifndef __PACKING_H
   #define __PACKING_H

   void packsplit(
       int *index,                 /* (i) the value to split */
       int *firstpart,             /* (o) the value specified by most
                                          significant bits */
       int *rest,                  /* (o) the value specified by least
                                          significant bits */
       int bitno_firstpart,    /* (i) number of bits in most
                                          significant part */
       int bitno_total             /* (i) number of bits in full range
                                          of value */
   );

   void packcombine(
       int *index,                 /* (i/o) the msb value in the
                                          combined value out */
       int rest,                   /* (i) the lsb value */
       int bitno_rest              /* (i) the number of bits in the
                                          lsb part */
   );

   void dopack(
       unsigned char **bitstream,  /* (i/o) on entrance pointer to
                                          place in bitstream to pack
                                          new data, on exit pointer
                                          to place in bitstream to
                                          pack future data */
       int index,                  /* (i) the value to pack */
       int bitno,                  /* (i) the number of bits that the
                                          value will fit within */
       int *pos                /* (i/o) write position in the
                                          current byte */
   );





   void unpack(
       unsigned char **bitstream,  /* (i/o) on entrance pointer to
                                          place in bitstream to
                                          unpack new data from, on
                                          exit pointer to place in
                                          bitstream to unpack future
                                          data from */
       int *index,                 /* (o) resulting value */
       int bitno,                  /* (i) number of bits used to
                                          represent the value */
       int *pos                /* (i/o) read position in the
                                          current byte */
   );

   #endif

