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

    enum SurfaceFlag { NonReserved, Reserved };
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
