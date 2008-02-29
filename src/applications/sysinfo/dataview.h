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

#ifndef DATAVIEW_H
#define DATAVIEW_H

#include <qwidget.h>

class QStorageMetaInfo;
class QFileSystem;
class GraphData;
class Graph;
class GraphLegend;

class DataView: public QWidget
{
    Q_OBJECT
public:
    DataView( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~DataView();

protected:
    void timerEvent(QTimerEvent *);
private slots:
    void updateData();
    void init();

private:
    int documentSize(QString& mimeFilter);
    void fileSystemMetrics(const QFileSystem *fs, long *avail, long *total);

    GraphData *data;
    Graph *graph;
    GraphLegend *legend;
    QStorageMetaInfo *storage;
};

#endif
