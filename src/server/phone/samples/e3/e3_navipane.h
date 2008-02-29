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

#ifndef E3_NAVIPANE
#define E3_NAVIPANE

#include <QWidget>

class E3NaviPanePrivate;
class QWSWindow;

class E3NaviPane : public QWidget
{
    Q_OBJECT
public:
    E3NaviPane(QWidget *parent=0, Qt::WFlags=0);
    
    enum Location {
        Beginning = 0, Middle, End, NA
    };

private:
    void setTabs(QPair<int,int> id, QList<QPair<QString,QIcon> > tabs);
    void setLocationHint(QPair<int,int> id, QString text, QIcon icon, Location loc = NA);

private slots:
    void activeChanged(QString,QRect,WId);
    void received(const QString &msg, const QByteArray &data);

private:
    E3NaviPanePrivate *d;
    friend class NaviPaneService;
};

#endif

