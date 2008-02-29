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

#ifndef MEMORY_H
#define MEMORY_H

#include <QWidget>

class GraphData;
class Graph;
class GraphLegend;
class QLabel;

class MemoryInfo : public QWidget
{
    Q_OBJECT
public:
    MemoryInfo( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~MemoryInfo();

protected:
    void timerEvent(QTimerEvent *);
private slots:
    void updateData();
    void init();

private:
    QLabel *totalMem;
    GraphData *data;
    Graph *graph;
    GraphLegend *legend;
};

#endif
