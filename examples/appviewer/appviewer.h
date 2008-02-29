/****************************************************************************
 **
 ** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#endif // APPVIEWER_H
