/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDIRECTPAINTER_QWS_H
#define QDIRECTPAINTER_QWS_H

#include <QtCore/qobject.h>
#include <QtGui/qregion.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_DIRECTPAINTER
class QDirectPainterPrivate;

class Q_GUI_EXPORT QDirectPainter : public QObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDirectPainter)
public:

    enum SurfaceFlag { NonReserved = 0,
                       Reserved = 1,
                       ReservedSynchronous = 3 };

    explicit QDirectPainter(QObject *parentObject = 0, SurfaceFlag flag = NonReserved);
    ~QDirectPainter();

    void setRegion(const QRegion&);
    QRegion requestedRegion() const;
    QRegion allocatedRegion() const;

    void setGeometry(const QRect&);
    QRect geometry() const;

    WId winId() const;
    virtual void regionChanged(const QRegion &exposedRegion);

    void startPainting(bool lockDisplay = false);
    void endPainting();
    void endPainting(const QRegion &region);
    void flush(const QRegion &region);

    void raise();
    void lower();


    static QRegion reserveRegion(const QRegion&);
    static QRegion reservedRegion();
    static QRegion region() { return reservedRegion(); }

    static uchar* frameBuffer();
    static int screenDepth();
    static int screenWidth();
    static int screenHeight();
    static int linestep();

    static void lock();
    static void unlock();
private:
    friend  void qt_directpainter_region(QDirectPainter *dp, const QRegion &alloc, int type);
};

#endif

QT_END_HEADER

#endif // QDIRECTPAINTER_QWS_H
