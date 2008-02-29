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



#include <qfileinfo.h>
#include <qbuffer.h>
#include <qtextcodec.h>

#include "email.h"
#include "emailfolderlist.h"
#include "longstream.h"

#include <qcontent.h>
#include <qmimetype.h>

Email::Email(EmailFolderList *parent)
    : MailMessage()
{
    _parent = parent;

    _inMem = false;
    _dirty = false;
    _arrival = 0;
}

Email::Email(const MailMessage &m, EmailFolderList *parent)
    : MailMessage(m)
{
    _parent = parent;

    _inMem = false;
    _dirty = false;
    _arrival = 0;
}

EmailFolderList* Email::parent()
{
    return _parent;
}

void Email::reparent(EmailFolderList *parent)
{
    _parent = parent;
}

void Email::readFromFile()
{
    if ( _parent && !_inMem)
        _parent->swapMailIn( this );
}

void Email::setInMemory(bool b)
{
    _inMem = b;
}

bool Email::setAttachmentInstalled(const QString& filename, bool install)
{
    for ( int i = 0; i < (int)messagePartCount(); i++ ) {
        MailMessagePart &part = messagePartAt( i );
        if (part.name() == filename ||
            (part.name().isEmpty() && part.contentLocation() == filename) ) {

            QString file = part.storedFilename();
            QString lnk = part.linkFile();

            QContent c(lnk);
            if ( install ) {
                if ( !validateFile(part) )  //install file to documents folder
                    return false;

                file = part.storedFilename();
                if ( !c.isValid() || c.file() != file ) {
                    // QContent determines it's internal type in the constructor
                    QContent d( file );

                    if( d.drmState() == QContent::Unprotected )
                        d.setType( part.contentType() );
                    d.setName( !part.name().isEmpty() ? part.name() : part.contentLocation() );
                    d.commit();
                    part.setLinkFile( d.linkFile() );

                    _dirty = true;
                }
                return true;
            } else {
                if ( c.isValid() )
                    c.removeFiles();

                part.setLinkFile( "" );
                part.setStoredFilename("");

                _dirty = true;

                return true;
            }
        }
    }
    return false;
}

bool Email::validateFile(MailMessagePart &part)
{
    QString str = part.storedFilename().trimmed();

    if ( str.isEmpty() ) {
        //str = Qtopia::applicationFileName("qtmail", "enclosures/");
        str = Qtopia::documentDir();

        uint i = 0;
        while ( i < messagePartCount() ) {
            if ( messagePartAt(i).filename() == part.filename() )
                break;
            i++;
        }

        if (QFile::exists( str + part.filename() )) {
            str += QString("%1_%2_%3").arg( uuid().data1 ).arg( i ).arg( part.filename() );
        } else {
            str += part.filename();
        }

        part.setStoredFilename( str );
        _dirty = true;
    }

    QFile f( str );
    if (!LongStream::freeSpace(  Qtopia::documentDir(), part.encodedSize() ))
        return false;

    if ( !f.exists() )
        return saveFile(part);

    return true;
}

bool Email::saveFile(MailMessagePart &part)
{
    QFile f( part.storedFilename() );

    if ( !f.open(QIODevice::WriteOnly) ) {
        qWarning("could not save attachment %s", part.storedFilename().toLatin1().data() );
        return false;
    }
    QDataStream out(&f);
    part.decodedBody(out);
    f.close();

    return true;
}

QString Email::displayName(QContactModel *m, const QString &name) const
{
    // For SMS messages look for a name in the addressbook for the phone number
    if (type() != MailMessage::SMS)
        return name;

    QString tempName;
    QString lookupName = name;
    if (!m)
        return name;

    if (lookupName.endsWith( "@sms" ))
        lookupName.truncate( lookupName.length() - 4 );
    tempName = m->matchPhoneNumber( lookupName ).label();
    if (!tempName.isEmpty())
        return tempName;

    // No name found in addressbook
    return name;
}
