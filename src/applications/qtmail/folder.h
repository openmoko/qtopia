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



#ifndef FOLDER_H
#define FOLDER_H

#include <qstring.h>
#include <qobject.h>

class Search;
class Email;

const int FolderTypeSystem = 1;
const int FolderTypeSearch = 2;
const int FolderTypeAccount = 3;
const int FolderTypeMailbox = 4;

const int SystemTypeMailbox = 1;
const int SystemTypeSearch = 2;

class Folder : public QObject
{
    Q_OBJECT

public:
    Folder(int type);
    virtual ~Folder();

    int folderType() { return _folderType; };

    virtual bool matchesEmail(Email *);
    virtual QString mailbox();
    virtual QString name();
    virtual QString fullName();
    virtual QString displayName();
    virtual void setName(QString str);

private:
    int _folderType;
    QString _name;
};


class SystemFolder : public Folder
{
    Q_OBJECT
public:
    SystemFolder(int systemType, const QString &mailbox);
    bool matchesEmail(Email *);
    QString mailbox();
    bool isSearch();
    int systemType();
    QString name();
    QString fullName() { return name(); };

    void setSearch(Search *in);
    Search *search();
private:
    int _systemType;
    QString _mailbox;
    Search *_search;
};

class SearchFolder : public Folder
{
    Q_OBJECT
public:
    SearchFolder(Search *in);
    ~SearchFolder();

    void setSearch(Search *in);
    Search *search();

    bool matchesEmail(Email *);
    QString mailbox();
    QString name();
    QString fullName() { return name(); };

private:
    Search *_search;
};

#endif
