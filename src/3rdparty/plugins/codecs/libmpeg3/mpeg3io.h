/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef MPEG3IO_H
#define MPEG3IO_H


#include <stdio.h>
#include "mpeg3css.h"
#include "mpeg3private.inc"

/* Filesystem structure */

typedef struct
{
	FILE *fd;
	mpeg3_css_t *css;          /* Encryption object */
	char path[MPEG3_STRLEN];
/* Hypothetical position of file pointer */
	long current_byte;
	long total_bytes;
	char has_id3_tag;
} mpeg3_fs_t;

#define mpeg3io_tell(fs) (((mpeg3_fs_t *)(fs))->current_byte)

// End of file
#define mpeg3io_eof(fs) (((mpeg3_fs_t *)(fs))->current_byte >= ((mpeg3_fs_t *)(fs))->total_bytes - 1)

// Beginning of file
#define mpeg3io_bof(fs)	(((mpeg3_fs_t *)(fs))->current_byte < 0)


#define mpeg3io_total_bytes(fs) (((mpeg3_fs_t *)(fs))->total_bytes)

extern inline unsigned int mpeg3io_read_int32(mpeg3_fs_t *fs)
{
	int a, b, c, d;
	unsigned int result;
/* Do not fread.  This breaks byte ordering. */
	a = (unsigned char)fgetc(fs->fd);
	b = (unsigned char)fgetc(fs->fd);
	c = (unsigned char)fgetc(fs->fd);
	d = (unsigned char)fgetc(fs->fd);
	result = ((int)a << 24) |
					((int)b << 16) |
					((int)c << 8) |
					((int)d);
	fs->current_byte += 4;
	return result;
}

extern inline unsigned int mpeg3io_read_char(mpeg3_fs_t *fs)
{
	fs->current_byte++;
	return fgetc(fs->fd);
}

#endif
