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

#ifdef Q_WS_QWS

#include <QScreen>

class ProxyScreen : public QScreen
{
public:
    ProxyScreen(int display_id);
    ~ProxyScreen();

    void setScreen(QScreen *screen);

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
    bool isTransformed() const;

    int memoryNeeded(const QString&);
    int sharedRamSize(void *);

    void haltUpdates();
    void resumeUpdates();

    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);
    void setDirty(const QRect&);

    QWSWindowSurface* createSurface(QWidget *widget) const;
    QWSWindowSurface* createSurface(const QString &key) const;

    QList<QScreen*> subScreens() const;
    QRegion region() const;

private:
    void configure();

    QScreen *realScreen;
};

#endif
