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

#ifndef SIMINFO_H
#define SIMINFO_H

#include <QWidget>
#include <qtopiaphone/qsmsreader.h>
#include <qtopiaphone/qphonebook.h>

class GraphData;
class Graph;
class GraphLegend;
class QLabel;

class SimInfo : public QWidget
{
    Q_OBJECT
public:
    SimInfo( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~SimInfo();

private slots:
    void updateData();
    void limits( const QString& store, const QPhoneBookLimits& value );
    void init();

private:
    QSMSReader *sms;
    QPhoneBook *pb;
    GraphData *smsData;
    Graph *smsGraph;
    GraphLegend *smsLegend;
    GraphData *pbData;
    Graph *pbGraph;
    GraphLegend *pbLegend;
    int pbused;
    int pbtotal;
};

#endif
