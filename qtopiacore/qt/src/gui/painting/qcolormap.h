/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOLORMAP_H
#define QCOLORMAP_H

#include <QtCore/qatomic.h>
#include <QtGui/qrgb.h>
#include <QtCore/qvector.h>
#include <QtGui/qwindowdefs.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QColor;
class QColormapPrivate;

class Q_GUI_EXPORT QColormap
{
public:
    enum Mode { Direct, Indexed, Gray };

    static void initialize();
    static void cleanup();

    static QColormap instance(int screen = -1);

    QColormap(const QColormap &colormap);
    ~QColormap();

    QColormap &operator=(const QColormap &colormap);

    Mode mode() const;

    int depth() const;
    int size() const;

    uint pixel(const QColor &color) const;
    const QColor colorAt(uint pixel) const;

    const QVector<QColor> colormap() const;

#ifdef Q_WS_WIN
    static HPALETTE hPal();
#endif

private:
    QColormap();
    QColormapPrivate *d;
};

QT_END_HEADER

#endif // QCOLORMAP_H
