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

#ifndef APPICONS_H
#define APPICONS_H


#include <qmap.h>
#include <qlabel.h>

class QHBoxLayout;

class AppIcons : public QWidget
{
    Q_OBJECT

public:
    AppIcons( QWidget *parent );
    ~AppIcons();

    void setIcon(int id, const QPixmap&);
    void remove(int id);

private slots:
    void receive( const QString &msg, const QByteArray &data );
    void clicked(const QPoint& relpos, int button, bool dbl);

private:
    QMap<int,QLabel*> buttons;
    QLabel* button(int id);
    int findId(QLabel*);
    QHBoxLayout *hbLayout;
};


#endif
