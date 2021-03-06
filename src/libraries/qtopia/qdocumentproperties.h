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

#ifndef DOCPROPERTIES_H
#define DOCPROPERTIES_H

#include <qtopiaglobal.h>
#include <qcontent.h>
#include <QDialog>

class QContent;
class QFormLayout;
class QDocumentPropertiesWidgetPrivate;

class QTOPIA_EXPORT QDocumentPropertiesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QDocumentPropertiesWidget( const QContent &lnk, QWidget* parent = 0 );
    ~QDocumentPropertiesWidget();

    QContent document() const;

public slots:
    void applyChanges();

signals:
    void done();

protected slots:
    void beamLnk();
    void unlinkLnk();
    void duplicateLnk();
    void showLicenses();

private:
    bool moveLnk();
    void addRights( const QDrmRights &rights, QFormLayout *layout );
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

    QContent document() const;

    void done(int);

private:
    QDocumentPropertiesWidget *d;
};

#endif // DOCPROPERTIES_H
