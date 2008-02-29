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

#ifndef QSCREENTRANSFORMED_QWS_H
#define QSCREENTRANSFORMED_QWS_H

#include <QtGui/qscreenlinuxfb_qws.h>
#include <QtGui/qscreenvfb_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_TRANSFORMED

class QTransformedScreenPrivate;

class QTransformedScreen : public QScreen
{
public:
    explicit QTransformedScreen(int display_id);
    ~QTransformedScreen();

    enum Transformation { None, Rot90, Rot180, Rot270 };

    void setTransformation(Transformation t);
    Transformation transformation() const;
    int transformOrientation() const;

    QSize mapToDevice(const QSize &s) const;
    QSize mapFromDevice(const QSize &s) const;

    QPoint mapToDevice(const QPoint &, const QSize &) const;
    QPoint mapFromDevice(const QPoint &, const QSize &) const;

    QRect mapToDevice(const QRect &, const QSize &) const;
    QRect mapFromDevice(const QRect &, const QSize &) const;

    QRegion mapToDevice(const QRegion &, const QSize &) const;
    QRegion mapFromDevice(const QRegion &, const QSize &) const;

    bool connect(const QString &displaySpec);
    bool initDevice();
    void shutdownDevice();
    void disconnect();

    void setMode(int width, int height, int depth);
    bool supportsDepth(int) const;

    void save();
    void restore();
    void blank(bool on);

    bool onCard(const unsigned char *) const;
    bool onCard(const unsigned char *, ulong& out_offset) const;

    bool isInterlaced() const;
    bool isTransformed() const { return transformation() != None; }

    int memoryNeeded(const QString&);
    int sharedRamSize(void *);

    void haltUpdates();
    void resumeUpdates();

    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);
    void setDirty(const QRect&);

    QWSWindowSurface* createSurface(QWidget *widget) const;

    QList<QScreen*> subScreens() const;
    QRegion region() const;

private:
    friend class QTransformedScreenPrivate;
    QTransformedScreenPrivate *d_ptr;
};

#endif // QT_NO_QWS_TRANSFORMED

QT_END_HEADER

#endif // QSCREENTRANSFORMED_QWS_H
