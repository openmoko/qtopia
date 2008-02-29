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

#ifndef _WHEELBROWSER_H_
#define _WHEELBROWSER_H_

#include "phone/qabstractbrowserscreen.h"
#include "qpixmapwheel.h"
class QSettings;
class QPixmapWheel;
class QPixmapWheelData;
class QTimeLine;
class QContent;

class WheelBrowserScreen : public QAbstractBrowserScreen
{
Q_OBJECT
public:
    WheelBrowserScreen(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;

    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

protected:
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *);

private slots:
    void moveToCompleted();
    void timelineStep(qreal);
    void clicked(const QString &);

private:
    QPixmapWheelData getData(const QString &entry);
    QContent *readLauncherMenuItem(const QString &entry);
    void doHide();

    unsigned char m_fillAlpha;
    bool m_hiding;
    QPixmapWheel *m_wheel;
    QPixmapWheelData *m_data;
    QTimeLine *m_fillTimeline;
    QStringList m_views;
};

#endif // _WHEELBROWSER_H_
