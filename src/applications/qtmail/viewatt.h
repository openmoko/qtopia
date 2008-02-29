/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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



#ifndef VIEWATT_H
#define VIEWATT_H

#include <qdialog.h>
#include <qlist.h>
#include <qmap.h>

class QLabel;
class QTableWidget;
class QTableWidgetItem;
class QMailMessagePart;
class QMailMessage;

class ViewAtt : public QDialog
{
    Q_OBJECT

public:
    ViewAtt(QMailMessage *mailIn, bool _inbox, QWidget *parent = 0, Qt::WFlags f = 0);
    bool eventFilter( QObject *, QEvent * );

public slots:
    void accept();
    void reject();

private slots:
    void setInstall(QTableWidgetItem* i);

private:
    void init();
    QTableWidget *listView;
    QMailMessage* mail;
    bool inbox;
    QLabel* label;
    QList<QTableWidgetItem*> on;
    QList<QTableWidgetItem*> off;
    QMap<QTableWidgetItem*,int> attachmentMap; //(tableitem,part index)
};

#endif
