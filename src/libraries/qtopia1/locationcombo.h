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

#ifndef LOCATIONCOMBO_H
#define LOCATIONCOMBO_H

#include <qtopia/qpeglobal.h>

#ifdef Q_WS_QWS

#include <qstringlist.h>
#include <qcombobox.h>

class AppLnk;
class QListViewItem;
class DocLnk;
class FileSystem;
class StorageInfo;

class DocPropertiesWidgetPrivate;

class QTOPIA_EXPORT LocationCombo : public QComboBox
{
    Q_OBJECT
public:
    LocationCombo( QWidget *parent, const char *name=0 );
    LocationCombo( const AppLnk * lnk, QWidget *parent, const char *name=0 );

    ~LocationCombo();

    void setLocation( const AppLnk * );

    QString path() const;
    QString documentPath() const;
    const FileSystem *fileSystem() const;

    bool isChanged() const;

signals:
    void newPath();

private slots:
    void updatePaths();

private:
    void setupCombo();
    QStringList locations;
    int currentLocation;
    int originalLocation;
    int fileSize;
    StorageInfo *storage;
};


#endif // QWS
#endif // LNKPROPERTIES_H
