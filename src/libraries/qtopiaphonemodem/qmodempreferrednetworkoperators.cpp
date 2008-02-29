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

#include <qmodempreferrednetworkoperators.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>

/*!
    \class QModemPreferredNetworkOperators
    \mainclass
    \brief The QModemPreferredNetworkOperators class implements the preferred operator list functions for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CPOL}, \c{AT+CPLS} and \c{AT+COPN} commands from 3GPP TS 27.007.

    QModemPreferredNetworkOperators implements the QPreferredNetworkOperators telephony
    interface.  Client applications should use QPreferredNetworkOperators instead
    of this class to access the modem's preferred operator list.

    \sa QPreferredNetworkOperators
*/

class QModemPreferredNetworkOperatorsPrivate
{
public:
    QModemService *service;
    QPreferredNetworkOperators::List requestedList;
    QPreferredNetworkOperators::List revertList;
    QPreferredNetworkOperators::List selectedList;

    struct Pending
    {
        QPreferredNetworkOperators::List list;
        QPreferredNetworkOperators::Info oper;
        Pending *next;
    };
    Pending *firstPending;
    Pending *lastPending;
};

/*!
    Create a new preferred network operator handler for \a service.
*/
QModemPreferredNetworkOperators::QModemPreferredNetworkOperators
        ( QModemService *service )
    : QPreferredNetworkOperators( service->service(), service,
                                  QCommInterface::Server )
{
    d = new QModemPreferredNetworkOperatorsPrivate();
    d->service = service;
    d->requestedList = QPreferredNetworkOperators::Current;
    d->revertList = QPreferredNetworkOperators::Current;
    d->selectedList = QPreferredNetworkOperators::Current;
    d->firstPending = 0;
    d->lastPending = 0;
}

/*!
    Destroy this preferred network operator handler.
*/
QModemPreferredNetworkOperators::~QModemPreferredNetworkOperators()
{
    delete d;
}

/*!
    \reimp
*/
void QModemPreferredNetworkOperators::requestOperatorNames()
{
    d->service->secondaryAtChat()->chat
        ( "AT+COPN", this, SLOT(copn(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemPreferredNetworkOperators::requestPreferredOperators
        ( QPreferredNetworkOperators::List list )
{
    d->requestedList = list;
    d->selectedList = QPreferredNetworkOperators::Current;
    d->service->secondaryAtChat()->chat
        ( "AT+CPLS?", this, SLOT(cplsQuery(bool,QAtResult)) );
}

class QPolUserData : public QAtResult::UserData
{
public:
    QPolUserData( const QString& command )
    { this->command = command; }

    QString command;
};

/*!
    \reimp
*/
void QModemPreferredNetworkOperators::writePreferredOperator
        ( QPreferredNetworkOperators::List list,
          const QPreferredNetworkOperators::Info & oper )
{
    // Add the change to the list of pending changes.
    QModemPreferredNetworkOperatorsPrivate::Pending *pending;
    pending = new QModemPreferredNetworkOperatorsPrivate::Pending();
    pending->list = list;
    pending->oper = oper;
    pending->next = 0;
    if ( d->lastPending )
        d->lastPending->next = pending;
    else
        d->firstPending = pending;
    d->lastPending = pending;

    // If this is the only entry on the list, then write it now.
    if ( d->firstPending == d->lastPending )
        writeNextPreferredOperator();
}

void QModemPreferredNetworkOperators::copn( bool, const QAtResult& result )
{
    QList<QPreferredNetworkOperators::NameInfo> names;
    QPreferredNetworkOperators::NameInfo name;
    QAtResultParser parser( result );
    while ( parser.next( "+COPN:" ) ) {
        name.id = parser.readNumeric();
        // numeric value might be enclosed in double quotation mark. e.g. YUAWEI modem
        if ( name.id == 0 )
            name.id = parser.readString().toUInt();
        name.name = parser.readString();
        names += name;
    }
    emit operatorNames( names );
}

void QModemPreferredNetworkOperators::cplsQuery( bool, const QAtResult& result )
{
    // Parse the name of the current preferred operator list.
    QAtResultParser parser( result );
    parser.next( "+CPLS:" );
    uint nlist = parser.readNumeric();
    QPreferredNetworkOperators::List list;
    switch ( nlist ) {
        case 1:     list = QPreferredNetworkOperators::OperatorControlled;
                    break;

        case 4:     list = QPreferredNetworkOperators::HPLMN;
                    break;

        default:    list = QPreferredNetworkOperators::UserControlled;
                    break;
    }
    d->selectedList = list;

    // If we need a different list, then switch away.
    if ( list != d->requestedList &&
         d->requestedList != QPreferredNetworkOperators::Current ) {
        d->revertList = list;
        d->service->secondaryAtChat()->chat
            ( "AT+CPLS=" + QString::number( listNumber( d->requestedList ) ) );
    } else {
        d->revertList = QPreferredNetworkOperators::Current;
        d->requestedList = list;
    }

    // Fetch the contents of the preferred operator list.
    d->service->secondaryAtChat()->chat
        ( "AT+CPOL?", this, SLOT(cpolQuery(bool,QAtResult)) );
}

void QModemPreferredNetworkOperators::cpolQuery( bool, const QAtResult& result )
{
    // Parse the provided operator list.
    QList<QPreferredNetworkOperators::Info> opers;
    QPreferredNetworkOperators::Info info;
    QAtResultParser parser( result );
    while ( parser.next( "+CPOL:" ) ) {
        info.index = parser.readNumeric();
        info.format = parser.readNumeric();
        if ( info.format == 2 ) {
            info.id = parser.readNumeric();
            // numeric value might be enclosed in double quotation mark. e.g. Greenphone
            if ( info.id == 0 )
                info.id = parser.readString().toUInt();
            info.name = QString();
        } else {
            info.id = 0;
            info.name = parser.readString();
        }
        info.technologies = QStringList();
        if ( parser.readNumeric() != 0 ) {
            info.technologies += "GSM";             // No tr
        }
        if ( parser.readNumeric() != 0 ) {
            info.technologies += "GSMCompact";      // No tr
        }
        if ( parser.readNumeric() != 0 ) {
            info.technologies += "UTRAN";           // No tr
        }
        if ( info.technologies.isEmpty() ) {
            // If no technolgies reported, then assume that this
            // modem's AT+CPOL command does not return those fields
            // and report "GSM" as the only technology supported.
            info.technologies += "GSM";             // No tr
        }
        opers.append( info );
    }

    // Send the list to the requesting client.
    emit preferredOperators( d->requestedList, opers );

    // Reset the AT+CPLS state if necessary.
    if ( d->revertList != QPreferredNetworkOperators::Current ) {
        d->selectedList = d->revertList;
        d->service->secondaryAtChat()->chat
            ( "AT+CPLS=" + QString::number( listNumber( d->revertList ) ) );
    }
}

void QModemPreferredNetworkOperators::cpolSet( bool ok, const QAtResult& result )
{
    if ( ok ) {
        emit writePreferredOperatorResult( QTelephony::OK );
        writeNextPreferredOperator();
    } else {
        // Retry without the technology flags, in case modem is pre-27.007.
        QString basicModify = ((QPolUserData *)result.userData())->command;
        d->service->secondaryAtChat()->chat
            ( basicModify, this, SLOT(cpolSet2(bool,QAtResult)) );
    }
}

void QModemPreferredNetworkOperators::cpolSet2( bool, const QAtResult& result )
{
    emit writePreferredOperatorResult
        ( (QTelephony::Result)result.resultCode() );
    writeNextPreferredOperator();
}

uint QModemPreferredNetworkOperators::listNumber
            ( QPreferredNetworkOperators::List list )
{
    switch ( list ) {
        default:
        case QPreferredNetworkOperators::UserControlled:
                return 0;

        case QPreferredNetworkOperators::OperatorControlled:
                return 1;

        case QPreferredNetworkOperators::HPLMN:
                return 4;
    }
}

void QModemPreferredNetworkOperators::writeNextPreferredOperator()
{
    // Remove the next pending item from the list.
    QModemPreferredNetworkOperatorsPrivate::Pending *pending;
    pending = d->firstPending;
    if ( !pending )
        return;
    QPreferredNetworkOperators::List list = pending->list;
    QPreferredNetworkOperators::Info oper = pending->oper;
    d->firstPending = pending->next;
    if ( !d->firstPending )
        d->lastPending = 0;
    delete pending;

    // Switch to the new list if necessary.
    if ( list != QPreferredNetworkOperators::Current ) {
        if ( list != d->selectedList ) {
            d->service->secondaryAtChat()->chat
                ( "AT+CPLS=" + QString::number( listNumber( list ) ) );
            d->selectedList = list;
        }
    }

    // Construct the AT+CPOL command to use to modify the operator setting.
    QString command = "AT+CPOL=";
    if ( oper.index != 0 ) {
        if ( oper.format == 2 && oper.id != 0 ) {
            // Update an existing numeric entry.
            command += QString::number( oper.index ) + ",2," +
                       QString::number( oper.id );
        } else if ( oper.format < 2 && oper.name.length() != 0 ) {
            // Update an existing alphabetic entry.
            command += QString::number( oper.index ) + "," +
                       QString::number( oper.format ) + ",\"" +
                       QAtUtils::quote( oper.name ) + "\"";
        } else {
            // Delete an entry.
            command += QString::number( oper.index );
        }
    } else if ( oper.format == 2 && oper.id != 0 ) {
        // Add a new numeric entry in a free location.
        command += ",2," + QString::number( oper.id );
    } else if ( oper.format < 2 && oper.name.length() != 0 ) {
        // Add a new alphabetic entry in a free location.
        command += "," + QString::number( oper.format ) + ",\"" +
                   QAtUtils::quote( oper.name ) + "\"";
    } else {
        // Only set the list - don't change anything else.
        return;
    }

    // Save the current command without technology flags so we
    // can reissue it if the expanded command does not work.
    QString basicModify = command;

    // Add the technology flags.
    if ( oper.technologies.contains( "GSM" ) )      // No tr
        command += ",2";
    else
        command += ",0";
    if ( oper.technologies.contains( "GSMCompact" ) )      // No tr
        command += ",3";
    else
        command += ",0";
    if ( oper.technologies.contains( "UTRAN" ) )      // No tr
        command += ",1";
    else
        command += ",0";

    // Set the AT+CPOL command to the modem.
    d->service->secondaryAtChat()->chat
        ( command, this, SLOT(cpolSet(bool,QAtResult)),
          new QPolUserData( basicModify ) );
}
