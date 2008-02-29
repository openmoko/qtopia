/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <stdlib.h>

#include <qdawg.h>

static void usage( const char *progname )
{
    printf( "%s converts word list files into the qdawg format\n", progname );
    printf( "Usage:\n" );
    printf( "\t%s <output directory> <input file> [input file...]\n", progname );
    exit(-1);
}



int main(int argc, char **argv)
{
    QFile qdawgFile;
    QFile wordlistFile;
    QFile e(argv[0]);
    if (argc < 3)
        usage(e.fileName().toLocal8Bit().constData());
    // Parse the command-line.
    QString path( argv[1] );
    QDir d;
    d.setPath(path);
    if ( !d.exists() ) {
        bool ok = d.mkpath( path );
        if ( !ok ) {
            printf( "Could not create directory %s\n", path.toLocal8Bit().constData() );
            return 1;
        }
    }

    for (int index = 2; index < argc; index++) {
        QFile qdawgFile;
        QFile wordlistFile;

        wordlistFile.setFileName(QString(argv[index]));
        qdawgFile.setFileName(QString("%1/%2.dawg").arg(path).arg(QFileInfo(wordlistFile).fileName()));

        QDawg dawg;
        if (!wordlistFile.open(QIODevice::ReadOnly)) {
            printf("Cannot open %s\n", wordlistFile.fileName().toLocal8Bit().constData());
            continue;
        }
        if (!qdawgFile.open(QIODevice::WriteOnly)) {
            printf("Cannot write %s\n", qdawgFile.fileName().toLocal8Bit().constData());
            continue;
        }

        printf("qdawggen %s %s\n", wordlistFile.fileName().toLocal8Bit().constData(),
                qdawgFile.fileName().toLocal8Bit().constData());
        dawg.createFromWords(&wordlistFile);
        dawg.write(&qdawgFile);
    }

    return 0;
}

