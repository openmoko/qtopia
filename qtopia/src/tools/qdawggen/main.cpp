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

#include <qfile.h>
#include <stdlib.h>

#include <qtopia/qdawg.h>

static void usage( const char *progname )
{
    qDebug( "%s converts a given word list into the qdawg format", progname );
    qDebug( "Usage:" );
    qDebug( "\t%s -w <wordlist-file> <qdawg-file>", progname );
    exit(-1);
}



int main(int argc, char **argv) {
   
    QFile qdawgFile;
    QFile wordlistFile;
    QFile e(argv[0]);
    if (argc == 1)
        usage(e.name());
    // Parse the command-line.
    int index = 1;
    
    for (index = 1; index < argc; index++) {
	if (strcmp(argv[index],"-w") == 0) {
	    index++;
	    if (index >= argc) {
		usage( e.name() );
	    } else {
		wordlistFile.setName(QString(argv[index]));
	    }
	} else {
	    // must be qdawg filename.  SHOULD be last argument.
	    if (index != argc-1) {
		usage( e.name() );
	    }
	    qdawgFile.setName(argv[index]);
	}
    }

    qDebug("%s %s", wordlistFile.name().latin1(), qdawgFile.name().latin1());

    
    QDawg dawg;
    if (!wordlistFile.open(IO_ReadOnly)) {
        qWarning("Cannot open %s.", wordlistFile.name().latin1());
        exit(-1);
    }
    if (!qdawgFile.open(IO_WriteOnly)) {
        qWarning("Cannot write %s.", qdawgFile.name().latin1());
        exit(-1);
    }

    qDebug("Converting %s to %s", wordlistFile.name().latin1(),
            qdawgFile.name().latin1());
    dawg.createFromWords(&wordlistFile);
    dawg.write(&qdawgFile);

}
