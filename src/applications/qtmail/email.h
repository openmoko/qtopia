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



#ifndef Email_H
#define Email_H

#include <qstring.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qlist.h>

#include <qtopianamespace.h>
#include <qtopia/mail/mailmessage.h>
#include <qtopia/pim/qcontactmodel.h>

class EmailFolderList;

class Email : public MailMessage
{
public:
    Email(EmailFolderList *parent=0);
    Email(const MailMessage &m, EmailFolderList *parent=0);

    EmailFolderList* parent();
    void reparent(EmailFolderList *parent);

    void readFromFile();
    void setInMemory(bool b);
    bool inMemory() const {return _inMem;};

    bool dirty() { return _dirty; };
    void setDirty(bool b) { _dirty = b; };

    uint arrival() { return _arrival; };
    void setArrival(uint u) { _arrival = u; };

    bool setAttachmentInstalled(const QString& filename, bool intall);
    bool validateFile(MailMessagePart &part);
    bool hasRecipients() { return to().count() != 0; };

    QString displayName(QContactModel *m, const QString &name) const;

private:
    void copy(const Email &e);
    bool saveFile(MailMessagePart &part);

private:
    uint _arrival;
    bool _inMem;
    bool _dirty;
    EmailFolderList *_parent;
};

#endif
