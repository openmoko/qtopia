/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef CONTACTBROWSER_H
#define CONTACTBROWSER_H

#include <QDLBrowserClient>

#include "contactdocument.h"
#include "qcontent.h"

class QContact;
class QContactModel;
class QtopiaServiceRequest;
class ContactTextEdit;
class ContactToneButton;
class ContactPictureButton;

class ContactBrowser : public QDLBrowserClient
{
    Q_OBJECT

public:
    ContactBrowser( QWidget *parent, const char *name = 0 );
    ~ContactBrowser();
    void init(const QContact& contact, ContactDocument::ContactDocumentType docType);

    void setModel(QContactModel *model);
    void processKeyPressEvent(QKeyEvent *e) {keyPressEvent(e);}

    QContact contact() const;

public slots:
    void linkClicked(const QString& link);
    void linkHighlighted(const QString& link);

signals:
    void okPressed();
    void previous();
    void next();
    void externalLinkActivated();
    void backClicked();

protected:
    void keyPressEvent( QKeyEvent *e );
    void mouseMoveEvent(QMouseEvent *) {}

    void setSource(const QUrl & name);

private:
    QRect fakeSelectionRect(QTextDocument* doc, const QTextCursor &cursor);
    QString mLink;
    ContactDocument *mDocument;
    QContactModel *mModel;
};

#endif // CONTACTBROWSER_H

