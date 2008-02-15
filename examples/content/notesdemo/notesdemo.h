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

#ifndef NOTESDEMO_H
#define NOTESDEMO_H

#include <QDialog>
#include <QContent>

class QDocumentSelector;
class QTextEdit;
class QStackedLayout;
class QTextDocument;

class NotesDemo : public QDialog
{
    Q_OBJECT
public:
    NotesDemo( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

public slots:
    virtual void done( int result );

private slots:
    void newDocument();
    void openDocument( const QContent &document );

private:
    bool readContent( QTextDocument *document, QContent *content );
    bool writeContent( QTextDocument *document, QContent *content );

    QStackedLayout *layout;
    QDocumentSelector *documentSelector;
    QTextEdit *editor;
    QContent currentDocument;
};

#endif
