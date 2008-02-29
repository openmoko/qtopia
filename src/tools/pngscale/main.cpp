/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <QImage>
#include <QString>
#include <QCoreApplication>
#include <QSvgRenderer>
#include <QPainter>

#include <qglobal.h>

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s -width w -height h input output [input output] ... [input output]\n", name);
    fprintf(stderr, "  scales input image to w x h and writes a PNG to output\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    int width = -1;
    int height = -1;

    int i;
    for (i = 1; i < argc; i++) {
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
        } else {
            break;
        }
    }
    if (width < 0 || height < 0)
        usage(argv[0]);

    if ( (argc - i) % 2 == 1 )
        usage(argv[0]);
    for ( ; i < argc; i += 2 ) {
        QString input(argv[i]);
        QString output(argv[i+1]);

        if (input.endsWith(".svg")) {
            QSvgRenderer renderer;
            if (!renderer.load(input)) {
                fprintf(stderr, "Cannot load %s\n", input.toLatin1().data());
                return -1;
            }
            QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter,QRectF(0,0,width,height));
            painter.end();
            if (!image.save(output, "PNG"))
                fprintf(stderr, "Cannot save %s\n", output.toLatin1().data());
        } else {
            QImage img(input);
            if (!img.isNull()) {
                QImage simg = img.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                if (!simg.save(output, "PNG")) {
                    fprintf(stderr, "Cannot save %s\n", output.toLatin1().data());
                }
            } else {
                fprintf(stderr, "Cannot load %s\n", input.toLatin1().data());
                return -1;
            }
        }
    }

    return 0;
}

