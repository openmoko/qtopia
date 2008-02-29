/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <sys/time.h>
#include <QWidget>

/*
  Little load meter
*/
class Load : public QWidget {
    Q_OBJECT
public:
    Load( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~Load();

protected:
    void paintEvent( QPaintEvent *ev );

private slots:
    void timeout();

private:
    int points;
    double *userLoad;
    double *systemLoad;
    double maxLoad;
    struct timeval last;
    int lastUser;
    int lastUsernice;
    int lastSys;
    int lastIdle;
    bool first;
};

class LoadInfo : public QWidget
{
    Q_OBJECT
public:
    LoadInfo( QWidget *parent = 0, Qt::WFlags f = 0 );

private:
    QPixmap makeLabel( const QColor &col );
    QString getCpuInfo();
private slots:
    void init();
};

