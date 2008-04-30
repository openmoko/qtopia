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

#ifndef SIMICONS_H
#define SIMICONS_H

#include <QSimIconReader>
#include <QIcon>
#include <QMap>

class SimIcons : public QObject
{
    Q_OBJECT
public:
    SimIcons( QSimIconReader *reader, QWidget *parent );
    ~SimIcons();

    QIcon icon( int iconId );
    QString iconFile( int iconId );

public slots:
    void needIcon( int iconId );
    void needIconInFile( int iconId );
    void requestIcons();

signals:
    void iconsReady();

private slots:
    void iconDone( int iconId );

private:
    QSimIconReader *reader;
    QWidget *parent;
    QList<int> pendingIcons;
    QList<int> loadedIcons;
    QList<int> fileIcons;
    QMap<int, QString> files;

    void copyIconsToFiles();
};

#endif

