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

#ifndef DOCPROPERTIES_H
#define DOCPROPERTIES_H

#include <qtopiaglobal.h>
#include <qcontent.h>

#ifdef Q_WS_QWS

#include <QDialog>

class QContent;

class QDocumentPropertiesWidgetPrivate;

class QTOPIA_EXPORT QDocumentPropertiesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QDocumentPropertiesWidget( const QContent &lnk, QWidget* parent = 0 );
    ~QDocumentPropertiesWidget();

public slots:
    void applyChanges();

signals:
    void done();

protected slots:
    void beamLnk();
    void unlinkLnk();
    void duplicateLnk();

private:
    bool moveLnk();
    QString formatRights( const QList< QDrmRights > &rights ) const;

    QContent lnk;
    int fileSize;

    QString safePath( const QString &name, const QString &location, const QString &type, const QString &oldPath ) const;

    QDocumentPropertiesWidgetPrivate *d;
};

class QTOPIA_EXPORT QDocumentPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QDocumentPropertiesDialog( const QContent &lnk, QWidget* parent = 0 );
    ~QDocumentPropertiesDialog();

    void done(int);

private:
    QDocumentPropertiesWidget *d;
};

#endif // QWS
#endif // DOCPROPERTIES_H
