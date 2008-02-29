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

#ifndef QMULTISCREEN_QWS_P_H
#define QMULTISCREEN_QWS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <qscreen_qws.h>

class QMultiScreenPrivate;

class QMultiScreen : public QScreen
{
public:
    QMultiScreen(int displayId);
    ~QMultiScreen();
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
    void blit(QWSWindow *bs, const QRegion &clip);
    void setDirty(const QRect&);

    QWSWindowSurface* createSurface(QWidget *widget) const;

    QList<QScreen*> subScreens() const;
    QRegion region() const;

private:
    void addSubScreen(QScreen *screen);
    void removeSubScreen(QScreen *screen);

    QMultiScreenPrivate *d_ptr;
};

#endif // QMULTISCREEN_QWS_P_H
