/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _SMSDECODER_H
#define _SMSDECODER_H
#ifndef QTOPIA_NO_SMS

#include <qstring.h>

class QSMSMessage;
class QMailMessage;
class QWbXmlReader;

class SMSDecoder
{
public:
    SMSDecoder();
    virtual ~SMSDecoder();

    // Convert an SMS application port number into a MIME type.
    // Returns null if no corresponding MIME type.
    static QString mimeTypeForPort( int port );

    // Get an SMS message decoder for a particular port number.
    // Returns null if there is no special decoding support.
    static SMSDecoder *decoder( int port );

    // Decode the contents of an SMS message into a mail message,
    // according to the rules of a port number handler.
    virtual void decode( QMailMessage& mail, const QSMSMessage& msg ) = 0;

    // Format all of the body parts in an SMS message.
    static void formatMessage( QMailMessage& mail, const QSMSMessage& msg );

};

class SMSMultipartDecoder : public SMSDecoder
{
public:
    SMSMultipartDecoder();
    virtual ~SMSMultipartDecoder();

    virtual void decode( QMailMessage& mail, const QSMSMessage& msg );
};

class SMSLogoDecoder : public SMSDecoder
{
public:
    SMSLogoDecoder( bool operatorHeader );
    virtual ~SMSLogoDecoder();

    virtual void decode( QMailMessage& mail, const QSMSMessage& msg );
private:
    bool operatorHeader;
};

class SMSWbXmlDecoder : public SMSDecoder
{
public:
    SMSWbXmlDecoder( QWbXmlReader *reader, const QString& mimeType, bool pushHeader );
    virtual ~SMSWbXmlDecoder();

    virtual void decode( QMailMessage& mail, const QSMSMessage& msg );
private:
    QWbXmlReader *reader;
    QString mimeType;
    bool pushHeader;
};

class SMSWapPushDecoder : public SMSDecoder
{
public:
    SMSWapPushDecoder();
    virtual ~SMSWapPushDecoder();

    virtual void decode( QMailMessage& mail, const QSMSMessage& msg );
};

#endif // QTOPIA_NO_SMS
#endif
