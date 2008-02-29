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

#include <stdio.h>
#include <stdlib.h>
#include <QImage>
#include <QString>
#include <QPicture>
#include <QSvgRenderer>
#include <QPainter>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <qglobal.h>

#include <math.h>

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s input output [input output] ... [input output]\n", name);
    fprintf(stderr, "  convert input SVG to output QPicture\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    int i;
    for (i = 1; i < argc; i++) {
        QString arg(argv[i]);
        if (arg[0] == '-') {
            usage(argv[0]);
        } else {
            break;
        }
    }

    if ( argc - i < 2 || (argc - i) % 2 == 1 )
        usage(argv[0]);
    for ( ; i < argc; i += 2 ) {
        QString input(argv[i]);
        QString output(argv[i+1]);

        QSvgRenderer renderer;
        if (renderer.load(input)) {
            QPicture picture;
            QSizeF sizef = renderer.viewBoxF().size();
            QSize size((int)ceil(sizef.width()), (int)ceil(sizef.height()));
            picture.setBoundingRect(QRect(QPoint(0,0), size));
            QPainter painter(&picture);
            renderer.render(&painter);
            painter.end();
            if (!picture.save(output)) {
                fprintf(stderr, "Cannot save %s\n", output.toLatin1().data());
                return -1;
            }
        } else {
            fprintf(stderr, "Cannot load %s\n", input.toLatin1().data());
            return -1;
        }
    }

    return 0;
}

