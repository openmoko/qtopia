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
#include <QWidget>
#include <QFrame>
#include <QList>

class QLabel;
class GraphData;
class Graph;
class GraphLegend;
class QFileSystem;
class MountInfo;
class QStorageMetaInfo;
class QScrollArea;


class StorageInfoView : public QWidget
{
    Q_OBJECT
public:
    StorageInfoView( QWidget *parent=0 );
    QSize sizeHint() const;

protected:
    void timerEvent(QTimerEvent*);

signals:
    void updated();

private slots:
    void updateMounts();
    void init();

private:
    void setVBGeom();
    QStorageMetaInfo *sinfo;
    QScrollArea *area;
};

class MountInfo : public QWidget
{
    Q_OBJECT
public:
    MountInfo( const QFileSystem*, QWidget *parent=0 );
    ~MountInfo();

public slots:
    void refresh();

private:
    void getSizeString( double &size, QString &string );

    QString title;
    const QFileSystem *fs;
    QLabel *totalSize;
    GraphData *data;
    Graph *graph;
    GraphLegend *legend;
};
