/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include <QTextStream>
#include <QDir>
#include <QMap>

#include <stdlib.h>

#define QTOPIA_INTERNAL_QDAWG_TRIE
#include <qdawg.h>

static void usage( const char *progname )
{
    // See also doc/src/tools/qdawggen.qdoc
    // or http://doc.trolltech.com/qtopia/qdawggen.html

    printf("%s converts word list files into the qdawg format\n", progname);
    printf("Usage:\n");
    printf("\t%s "
#ifdef QTOPIA_INTERNAL_QDAWG_TRIE
        "[-t] "
#endif
        "[-v] [-d] <output directory> <input file> [input file...]\n", progname);
#ifdef QTOPIA_INTERNAL_QDAWG_TRIE
    printf("\t  [-t]  Output a Trie (one-node-per-word)\n");
#endif
    printf("\t  [-v]  Verify result\n");
    printf("\t  [-d]  Dump result\n");
    exit(-1);
}



int main(int argc, char **argv)
{
    QFile qdawgFile;
    QFile wordlistFile;
    QFile e(argv[0]);
    bool error=false,dump=false;
#ifdef QTOPIA_INTERNAL_QDAWG_TRIE
    bool trie=false;
#endif
    int verify=0;
    if (argc < 3)
        error=true;
    // Parse the command-line.
    QString path( argv[1] );

    while ( path[0] == '-' ) {
#ifdef QTOPIA_INTERNAL_QDAWG_TRIE
        if ( path == "-t" ) trie=true;
        else
#endif
        if ( path == "-v" ) ++verify;
        else if ( path == "-d" ) dump=true;
        else error=true;
        --argc; ++argv; path=argv[1];
    }

    if (error)
        usage(e.fileName().toLocal8Bit().constData());

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
#ifdef QTOPIA_INTERNAL_QDAWG_TRIE
        if ( trie )
            dawg.createTrieFromWords(&wordlistFile);
        else
#endif
            dawg.createFromWords(&wordlistFile);
        dawg.write(&qdawgFile);
        if ( dump )
            dawg.dump();

        if ( verify ) {
            QMap<QString,int> trieval;
            QStringList wrote = dawg.allWords();
            QStringList read;
            wordlistFile.reset();
            QTextStream ts(&wordlistFile);
            while (!ts.atEnd()) {
                QString line = ts.readLine();
                QStringList t = line.split(' ');
                read += t[0];
                trieval[line]=t[1].toUInt();
            }
            if ( wrote == read ) {
                printf("  verified\n");
            } else {
                printf("  error in verification (are words sorted?)\n");
                if ( verify > 1 ) {
                    // XXX ... debug
                }
            }
        }
    }

    return 0;
}

