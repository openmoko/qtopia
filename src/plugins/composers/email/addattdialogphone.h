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
#ifndef ADDATTDIALOG_H
#define ADDATTDIALOG_H

#include <QAction>
#include <QDialog>

#include "addatt.h"

class QDocumentSelector;

// phone version, wraps AddAtt
class AddAttDialog : public QDialog, public AddAttBase
{
    Q_OBJECT
public:
    AddAttDialog(QWidget *parent = 0, QString name = QString(), Qt::WFlags f = 0);
    QList< AttachmentItem* > attachedFiles() const;
    void getFiles() {}
    void clear();
    void setMailMessageParts(QMailMessage *mail);
    QDocumentSelector* documentSelector() const;

public slots:
    void removeAttachment(AttachmentItem*);
    void removeCurrentAttachment();
    void attach( const QString &filename );
    void attach( const QContent &doclnk );
    void selectAttachment();
    virtual void done(int r);

protected slots:
    void openFile(const QContent&);
    void updateDisplay(bool);

signals:
    void currentChanged(bool);

private:
    AddAtt *addAtt;
    QDocumentSelector *fileSelector;
    QDialog *fileSelectorDialog;
    QAction *removeAction;
};

#endif
