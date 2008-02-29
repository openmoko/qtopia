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
 * Simple IDCT
 *
 * Copyright (c) 2001 Michael Niedermayer <michaelni@gmx.at>
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file simple_idct.h
 * simple idct header.
 */
 
void simple_idct_put(uint8_t *dest, int line_size, DCTELEM *block);
void simple_idct_add(uint8_t *dest, int line_size, DCTELEM *block);
void ff_simple_idct_mmx(int16_t *block);
void ff_simple_idct_add_mmx(uint8_t *dest, int line_size, int16_t *block);
void ff_simple_idct_put_mmx(uint8_t *dest, int line_size, int16_t *block);
void simple_idct(DCTELEM *block);

void simple_idct248_put(uint8_t *dest, int line_size, DCTELEM *block);

void simple_idct84_add(uint8_t *dest, int line_size, DCTELEM *block);
void simple_idct48_add(uint8_t *dest, int line_size, DCTELEM *block);
