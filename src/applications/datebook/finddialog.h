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


#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <qdatetime.h>
#include <qdialog.h>

class FindWidget;

class FindDialogPrivate;
class FindDialog : public QDialog
{
    Q_OBJECT
public:
    FindDialog( const QString &appName, QWidget *parent = 0 );
    ~FindDialog();

    QString findText() const;
    void setUseDate( bool show );
    void setDate( const QDate &dt );

public slots:
    void slotNotFound();
    void slotWrapAround();

signals:
    void signalFindClicked( const QString &txt, bool caseSensitive,
                            bool backwards, int category );
    void signalFindClicked( const QString &txt, const QDate &dt,
                            bool caseSensitive, bool backwards, int category );

private:
    FindWidget *fw;
    FindDialogPrivate *d;
};

#endif
