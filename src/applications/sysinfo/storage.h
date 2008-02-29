/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qwidget.h>
#include <qframe.h>
#include <qlist.h>
#include <qdict.h>

class QLabel;
class GraphData;
class Graph;
class GraphLegend;
class FileSystem;
class MountInfo;
class StorageInfo;
class QVBox;


class StorageInfoView : public QWidget
{
    Q_OBJECT
public:
    StorageInfoView( QWidget *parent=0, const char *name=0 );
    QSize sizeHint() const;
    
protected:
    void timerEvent(QTimerEvent*);
    void resizeEvent(QResizeEvent*);

signals:
    void updated();

private slots:
    void updateMounts();

private:
    void setVBGeom();
    StorageInfo *sinfo;
    QVBox *vb;
};

class MountInfo : public QWidget
{
    Q_OBJECT
public:
    MountInfo( const FileSystem*, QWidget *parent=0, const char *name=0 );
    ~MountInfo();

public slots:
    void refresh();

private:
    QString title;
    const FileSystem *fs;
    QLabel *totalSize;
    GraphData *data;
    Graph *graph;
    GraphLegend *legend;
};
