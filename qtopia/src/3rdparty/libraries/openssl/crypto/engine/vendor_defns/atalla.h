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
/* This header declares the necessary definitions for using the exponentiation
 * acceleration capabilities of Atalla cards. The only cryptographic operation
 * is performed by "ASI_RSAPrivateKeyOpFn" and this takes a structure that
 * defines an "RSA private key". However, it is really only performing a
 * regular mod_exp using the supplied modulus and exponent - no CRT form is
 * being used. Hence, it is a generic mod_exp function in disguise, and we use
 * it as such.
 *
 * Thanks to the people at Atalla for letting me know these definitions are
 * fine and that they can be reproduced here.
 *
 * Geoff.
 */

typedef struct ItemStr
	{
	unsigned char *data;
	int len;
	} Item;

typedef struct RSAPrivateKeyStr
	{
	void *reserved;
	Item version;
	Item modulus;
	Item publicExponent;
	Item privateExponent;
	Item prime[2];
	Item exponent[2];
	Item coefficient;
	} RSAPrivateKey;

/* Predeclare the function pointer types that we dynamically load from the DSO.
 * These use the same names and form that Ben's original support code had (in
 * crypto/bn/bn_exp.c) unless of course I've inadvertently changed the style
 * somewhere along the way!
 */

typedef int tfnASI_GetPerformanceStatistics(int reset_flag,
					unsigned int *ret_buf);

typedef int tfnASI_GetHardwareConfig(long card_num, unsigned int *ret_buf);

typedef int tfnASI_RSAPrivateKeyOpFn(RSAPrivateKey * rsaKey,
					unsigned char *output,
					unsigned char *input,
					unsigned int modulus_len);

