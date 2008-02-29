/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qobject.h>
#include <qstring.h>
#include <qmap.h>
#include <qpixmap.h>
#include <QRegion>

class SmilTransferServer;
class SmilModule;
class SmilElement;
class QTimer;

class SmilSystem : public QObject
{
    Q_OBJECT
public:
    SmilSystem();
    ~SmilSystem();

    void play();
    void reset();

    QString systemBitRate() const { return bitRate; }
    QString systemCaptions() const { return captions; }
    QString systemLanguage() const { return language; }
    QString systemOverdubOrSubtitle() const { return overdub; }
    QString systemRequired() const { return required; }
    QString systemScreenDepth() const { return screenDepth; }
    QString systemScreenSize() const { return screenSize; }

    SmilTransferServer *transferServer() { return xferServer; }
    const QMap<QString,SmilModule *> &modules() const { return mods; }

    void addModule(const QString &n, SmilModule *m) { mods[n] = m; }
    SmilModule *module(const QString &n) const { return mods[n]; }

    void setRootElement(SmilElement *e);
    SmilElement *rootElement() const { return root; }
    void setTarget(QWidget *w) { targetWidget = w; }

    SmilElement *findElement(SmilElement *e, const QString &id) const;
    QColor rootColor() const;

    void setDirty(const QRect &r);
    const QRegion &dirtyRegion() const { return updRgn; }
    void update(const QRect &r);
    void bodyFinished();

signals:
    void finished();

protected:
    void paint(QPainter *p);
    void paint(SmilElement *e, QPainter *p);

private:
    SmilTransferServer *xferServer;
    QString bitRate;
    QString captions;
    QString language;
    QString overdub;
    QString required;
    QString screenDepth;
    QString screenSize;
    QMap<QString,SmilModule *> mods;
    SmilElement *root;
    QPixmap buffer;
    QWidget *targetWidget;
    QTimer *timer;
    QRegion updRgn;
    friend class SmilView;
};

