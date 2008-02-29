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

#ifndef LOCATIONCOMBO_H
#define LOCATIONCOMBO_H

#include <qtopiaglobal.h>
#include <qstringlist.h>
#include <qcombobox.h>

class QContent;
class QListViewItem;
class QDocumentMetaInfo;
class QFileSystem;
class QStorageMetaInfo;
class QFileSystemFilter;

class QStorageDeviceSelectorPrivate;

class QTOPIA_EXPORT QStorageDeviceSelector : public QComboBox
{
    Q_OBJECT
public:
    explicit QStorageDeviceSelector( QWidget *parent=0 );
    explicit QStorageDeviceSelector( const QContent &lnk, QWidget *parent=0 );

    ~QStorageDeviceSelector();

    void setLocation( const QString& path );
    void setLocation( const QContent & );

    QString installationPath() const;
    QString documentPath() const;
    const QFileSystem *fileSystem() const;

    bool isChanged() const;

    void setFilter( QFileSystemFilter *fsf );

signals:
    void newPath();

private slots:
    void updatePaths();

private:
    void setupCombo();
    QStringList locations;
    QStorageMetaInfo *storage;
    QStorageDeviceSelectorPrivate *d;
    QFileSystemFilter *filter;
};

#endif // LNKPROPERTIES_H
