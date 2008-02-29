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
#ifndef ADDATT_H
#define ADDATT_H

#include "email.h"

#include <qcontent.h>

#include <QListWidget>
#include <QPushButton>
#include <QMenu>
#include <QString>
#include <QList>

class AttachmentItem : public QListWidgetItem
{
public:
    AttachmentItem(QListWidget *parent, const QContent&);
    AttachmentItem(QListWidget *parent, const QString &filename );
    ~AttachmentItem();

    bool isDocument() const;
    const QContent& document() const;
    QString fileName() const;
    int sizeKB() const;
private:
    QContent mattachment;
    QString mFileName;
    int mSizeKB;
};

class AddAttBase
{
public:
    static const int MAX_ATTACH_KB = 2048;
public:
    virtual ~AddAttBase();
    QList< AttachmentItem* > attachedFiles() const;
    void clear();
    virtual void getFiles() = 0;
    void setMailMessageParts(Email *mail);
    bool addAttachment(const QString &filename);
    bool addAttachment(const QContent&);
    int totalSizeKB();
protected:
    QStringList mimeTypes();
    QListWidget *attView;
    bool modified;
};

class AddAtt : public QWidget, public AddAttBase
{
    Q_OBJECT
public:
    AddAtt(QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
    void getFiles() {}
protected:
    void resizeEvent(QResizeEvent *);
public slots:
    void removeAttachment();
    void removeCurrentAttachment();
signals:
    void currentChanged(bool);
private slots:
    void currentItemChanged(QListWidgetItem*,QListWidgetItem* previous);
};

#ifdef QTOPIA_PHONE
#include "addattdialogphone.h"
#else
#include "addattdialogpda.h"
#endif

#endif
