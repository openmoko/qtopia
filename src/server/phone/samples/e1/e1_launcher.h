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

#ifndef _e1_LAUNCHER_H_
#define _e1_LAUNCHER_H_

#include <QWidget>
#include <QPixmap>
#include "qabstractserverinterface.h"
#include <qvaluespace.h>
#include <custom.h>

class QExportedBackground;
class E1Header;
class E1PhoneBrowser;
class E1Dialer;

class E1ServerInterface : public QAbstractServerInterface
{
Q_OBJECT
public:
    E1ServerInterface(QWidget *parent = 0, Qt::WFlags flags = 0);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void showEvent(QShowEvent *);

private slots:
    void wallpaperChanged();
    void showDialer( const QString& msg, const QByteArray& );
    void messageCountChanged();

private:
    QPixmap m_wallpaper;
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    QExportedBackground *m_eb;
#endif
    E1Header *m_header;
    E1PhoneBrowser *m_browser;
    E1Dialer* m_dialer;
    QValueSpaceItem m_newMessages;
};

#endif // _e1_LAUNCHER_H_

