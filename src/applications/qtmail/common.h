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

#ifndef COMMON_H
#define COMMON_H

#include <qapplication.h>

#ifdef SMTPAUTH
#include <qtopiaapplication.h>
#endif

namespace QtMail {

    static const char* InboxString = "inbox_ident";     // No tr
    static const char* OutboxString = "outbox_ident"; // No tr
    static const char* DraftsString = "drafts_ident"; // No tr
    static const char* SentString = "sent_ident"; // No tr
    static const char* TrashString = "trash_ident"; // No tr
    static const char* LastSearchString = "last_search_ident"; // No tr

    static QString mailboxTrName(const QString &s)
    {
        if ( s == InboxString )
            return qApp->translate( "QtMail", "Inbox" );
        else if ( s == OutboxString )
            return qApp->translate( "QtMail", "Outbox" );
        else if ( s == TrashString )
            return qApp->translate( "QtMail", "Trash" );
        else if ( s == SentString )
            return qApp->translate( "QtMail", "Sent" );
        else if ( s == DraftsString )
            return qApp->translate( "QtMail", "Drafts" );
        else if ( s == LastSearchString )
            return qApp->translate( "QtMail", "Last Search" );

        return s;
    }
#ifdef SMTPAUTH
    static QString sslCertsPath()
    {
        static QString temp(Qtopia::qtopiaDir() + "/etc/ssl/certs");
        return temp;
    }
#endif

    // Suppress declared but not used warnings
    static void qtmail_unused()
    {
        qtmail_unused();
        mailboxTrName("");
#ifdef SMTPAUTH
        sslCertsPath();
#endif
    }

    void replace( QString& str, const QString& before, const QString& after );
};

#endif
