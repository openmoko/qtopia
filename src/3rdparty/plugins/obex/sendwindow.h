/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef SENDWINDOW_H
#define SENDWINDOW_H

#include <qmainwindow.h>
#include <qvaluelist.h>
#include <qstring.h>

#include "sendwidgetbase.h"

class QIrServer;

struct BeamItem
{
    QString name;
    QString fileName;
    QString mimeType;
    int fileSize;
};

class SendWindow : public QMainWindow
{
    Q_OBJECT
public:
    SendWindow( QIrServer *server, QWidget *parent = 0, const char * name = 0 );
    ~SendWindow();

    void add(BeamItem item);
    void beamingFirst();
    void itemBeamed();
    int count() const { return beamItems.count(); }
    BeamItem next() const { return beamItems.first(); }
    
    void initBeam();
    void failed();
    void finished();

public slots:
    void setProgress(int i);

protected:
    void keyPressEvent(QKeyEvent*);

protected slots:
    void canceled();
    void statusMsg(const QString &);

private:
    void setTotalSteps(int i);
    void setStatus(const QString &);
    void init();

private:
    QIrServer *server;

    SendWidgetBase *w;
    QValueList<BeamItem> beamItems;
    QString currentItem;
};

#endif
