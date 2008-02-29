/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "qimage.h"

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s -width w -height h input output\n", name);
    fprintf(stderr, "  scales input PNG image to w x h and writes to output\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    QString input;
    QString output;
    int width = -1;
    int height = -1;

    for (int i = 1; i < argc; i++) {
	QString arg(argv[i]);
	if (arg[0] == '-') {
	    if (arg == "-width") {
		arg = argv[++i];
		width = arg.toInt();
	    } else if (arg == "-height") {
		arg = argv[++i];
		height = arg.toInt();
	    } else {
		usage(argv[0]);
	    }
	} else if (input.isEmpty()) {
	    input = arg;
	} else if (output.isEmpty()) {
	    output = arg;
	} else {
	    usage(argv[0]);
	}
    }

    if (width < 0 || height < 0 || input.isEmpty() || output.isEmpty())
	usage(argv[0]);

    QImage img(input);
    if (!img.isNull()) {
	QImage simg = img.smoothScale(width, height);
	if (!simg.save(output, "PNG")) {
	    fprintf(stderr, "Cannot save %s\n", output.latin1());
	}
    } else {
	fprintf(stderr, "Cannot load %s\n", input.latin1());
	return -1;
    }

    return 0;
}

