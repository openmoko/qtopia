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


#include "folder.h"
#include "search.h"
#include "email.h"
#include "common.h"

Folder::Folder(int type)
{
    _folderType = type;
}

Folder::~Folder()
{
}


// dummy.  Must be reimplemented
bool Folder::matchesEmail(Email *)
{
    return true;
}

// dummy.  Must be reimplemented
QString Folder::mailbox()
{
    return QtMail::InboxString;
}

QString Folder::name()
{
    return _name;
}

QString Folder::fullName()
{
    return _name;
}

QString Folder::displayName()
{
    return name();
}

void Folder::setName(QString str)
{
    _name = str;
}


/*  SystemFolder  */

SystemFolder::SystemFolder(int systemType, const QString &mailbox)
        : Folder( FolderTypeSystem )
{
    _systemType = systemType;
    _mailbox = mailbox;
    _search = NULL;

    if ( _systemType == SystemTypeSearch ) {
        _search = new Search();
        _search->setMailbox( _mailbox );
    }
}

bool SystemFolder::matchesEmail(Email *mail)
{
    switch ( _systemType ) {
        case SystemTypeSearch: return _search->matches(mail);
        default: return true;           // SystemTypeMailbox
    }
}

QString SystemFolder::mailbox()
{
    if ( _systemType == SystemTypeSearch )
        return ( _search->mailbox() );

    return _mailbox;
}

bool SystemFolder::isSearch()
{
    return _systemType == SystemTypeSearch;
}

int SystemFolder::systemType()
{
    return _systemType;
}

QString SystemFolder::name()
{
    return QtMail::mailboxTrName( _mailbox );
}

void SystemFolder::setSearch(Search *in)
{
    if ( _search )
        delete _search;

    _search = in;
}

Search* SystemFolder::search()
{
    return _search;
}

/*  SearchFolder  */

SearchFolder::SearchFolder(Search *in)
    : Folder( FolderTypeSearch )
{
    _search = in;
}

SearchFolder::~SearchFolder()
{
    delete _search;
}

void SearchFolder::setSearch(Search *in)
{
    _search = in;
}

Search* SearchFolder::search()
{
    return _search;
}

bool SearchFolder::matchesEmail(Email *mail)
{
    return _search->matches(mail);
}

QString SearchFolder::mailbox()
{
    return _search->mailbox();
}

QString SearchFolder::name()
{
    return _search->name();
}

