/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef APPVIEWER_H
#define APPVIEWER_H

#include <QContent>
#include <QDialog>
#include <QTextEdit>

class QDocumentSelector;

class AppViewer : public QDialog
{
    Q_OBJECT
public:
    AppViewer( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~AppViewer();

private slots:
    void openApplicationInfo();
    void documentSelected( const QContent & );

private:
    QString getInformation( const QContent & );
    QTextEdit *textArea;
    QDocumentSelector *appSelector;
};

#endif
