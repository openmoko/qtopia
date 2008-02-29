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
/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /usr/local/qtopiaroot/Source/qtopia-phone-2.1.1/src/3rdparty/applications/kphone/gsm/proto.h,v 1.1 2005/09/05 10:57:17 pankaj Exp $*/

#ifndef	PROTO_H
#define	PROTO_H

#if __cplusplus
#	define	NeedFunctionPrototypes	1
#endif

#if __STDC__
#	define	NeedFunctionPrototypes	1
#endif

#ifdef	_NO_PROTO
#	undef	NeedFunctionPrototypes
#endif

#undef	P	/* gnu stdio.h actually defines this... 	*/
#undef	P0
#undef	P1
#undef	P2
#undef	P3
#undef	P4
#undef	P5
#undef	P6
#undef	P7
#undef	P8

#if NeedFunctionPrototypes

#	define	P( protos )	protos

#	define	P0()				(void)
#	define	P1(x, a)			(a)
#	define	P2(x, a, b)			(a, b)
#	define	P3(x, a, b, c)			(a, b, c)
#	define	P4(x, a, b, c, d)		(a, b, c, d)	
#	define	P5(x, a, b, c, d, e)		(a, b, c, d, e)
#	define	P6(x, a, b, c, d, e, f)		(a, b, c, d, e, f)
#	define	P7(x, a, b, c, d, e, f, g)	(a, b, c, d, e, f, g)
#	define	P8(x, a, b, c, d, e, f, g, h)	(a, b, c, d, e, f, g, h)

#else /* !NeedFunctionPrototypes */

#	define	P( protos )	( /* protos */ )

#	define	P0()				()
#	define	P1(x, a)			x a;
#	define	P2(x, a, b)			x a; b;
#	define	P3(x, a, b, c)			x a; b; c;
#	define	P4(x, a, b, c, d)		x a; b; c; d;
#	define	P5(x, a, b, c, d, e)		x a; b; c; d; e;
#	define	P6(x, a, b, c, d, e, f)		x a; b; c; d; e; f;
#	define	P7(x, a, b, c, d, e, f, g)	x a; b; c; d; e; f; g;
#	define	P8(x, a, b, c, d, e, f, g, h)	x a; b; c; d; e; f; g; h;

#endif  /* !NeedFunctionPrototypes */

#endif	/* PROTO_H */
