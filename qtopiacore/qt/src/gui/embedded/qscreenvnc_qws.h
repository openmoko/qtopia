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

#ifndef QSCREENVNC_QWS_H
#define QSCREENVNC_QWS_H

#include <QtGui/qscreenlinuxfb_qws.h>

#ifndef QT_NO_QWS_VNC

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QVNCScreenPrivate;

class QVNCScreen : public QScreen
{
public:
    explicit QVNCScreen(int display_id);
    virtual ~QVNCScreen();

    bool initDevice();
    bool connect(const QString &displaySpec);
    void disconnect();
    void shutdownDevice();
    void setMode(int,int,int);
    bool supportsDepth(int) const;

    void save();
    void restore();
    void blank(bool on);

    bool onCard(const unsigned char *) const;
    bool onCard(const unsigned char *, ulong& out_offset) const;

    bool isInterlaced() const;

    int memoryNeeded(const QString&);
    int sharedRamSize(void *);

    void haltUpdates();
    void resumeUpdates();

    void exposeRegion(QRegion r, int changing);

    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);
    void setDirty(const QRect&);

    QWSWindowSurface* createSurface(QWidget *widget) const;

    QList<QScreen*> subScreens() const;
    QRegion region() const;

private:
    friend class QVNCServer;
    friend class QVNCScreenPrivate;
    QVNCScreenPrivate *d_ptr;
};

QT_END_HEADER

#endif // QT_NO_QWS_VNC
#endif // QSCREENVNC_QWS_H
