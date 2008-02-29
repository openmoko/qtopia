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
#ifndef QDOCUMENTSELECTORSERVICE_H
#define QDOCUMENTSELECTORSERVICE_H
#include <QDocumentSelector>

class QIODevice;

class QDocumentSelectorServicePrivate;

class QTOPIA_EXPORT QDocumentSelectorService : public QObject
{
    Q_OBJECT
public:
    explicit QDocumentSelectorService( QObject *parent = 0 );

    virtual ~QDocumentSelectorService();

    QContentFilter filter() const;
    void setFilter( const QContentFilter &filter );

    QContentSortCriteria sortCriteria() const;
    void setSortCriteria( const QContentSortCriteria &sort );

    QContent selectedDocument() const;

    QIODevice *selectedDocumentData();

    bool newDocument( const QString &name, const QString &type, QWidget *widget = 0 );
    bool newDocument( const QString &name, const QStringList &types, QWidget *widget = 0 );
    bool openDocument( QWidget *widget = 0 );
    bool saveDocument( QWidget *widget = 0 );

public slots:
    void close();

signals:
    void documentOpened( const QContent &document, QIODevice *data );

private:
    QDocumentSelectorServicePrivate *d;
};


#endif
