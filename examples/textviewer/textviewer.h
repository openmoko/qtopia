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

#ifndef TEXTVIEWER_H
#define TEXTVIEWER_H
#include <QContentSet>
#include <QMainWindow>
#include <QTextEdit>
class QDocumentSelector;

class TextViewer : public QMainWindow
{
    Q_OBJECT
public:
    TextViewer( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~TextViewer();
    void keyPressEvent(QKeyEvent *e);

private slots:
    void openDocument();
    void documentSelected(const QContent & docContent);


private:
    QTextEdit *textArea;
    QDocumentSelector *docSelector;
};

#endif // TEXTVIEWER_H
