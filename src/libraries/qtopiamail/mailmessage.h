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
#ifndef MAILMESSAGE_H
#define MAILMESSAGE_H

#include <qtopia/mail/qtopiamail.h>

#include <qtopiaglobal.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qdatetime.h>
#include <quuid.h>

#include <longstring.h>

enum TransferEncoding {
    EightBit, Base64, QuotedPrintable
};

enum EmailStatusFlag {
    EFlag_Incoming      = 0x0001,
    EFlag_Outgoing      = 0x0002,
    EFlag_Sent          = 0x0004,
    EFlag_Replied       = 0x0008,
    EFlag_RepliedAll    = 0x0010,
    EFlag_Forwarded     = 0x0020,
    EFlag_Downloaded    = 0x0040,
    EFlag_Read          = 0x0080,
    EFlag_GoneFromServer= 0x0100,
    EFlag_IMAP_Seen     = 0x0200,

    EFlag_TypeMms       = 0x01000000,
    EFlag_TypeEms       = 0x02000000,
    EFlag_TypeSms       = 0x04000000,
    EFlag_TypeEmail     = 0x08000000,
    EFlag_TypeSystem    = 0x10000000
};

typedef uint EmailStatus;

class MailMessagePart;
class QTextStream;

/*
    WARNING!!!

    This class is part of the Qtopia mail library and is considered unfinished.  Any usage
    of this class or library is not advisable.

    The current library will not be source or binary compatible with subsequent releases and updates.
*/

class QTOPIAMAIL_EXPORT MailMessage
{
public:
    MailMessage();
    MailMessage(const QString &s);
    virtual ~MailMessage();

    void init();

    virtual QString toRFC822() const;
    QString fileName() const;
    virtual void fromRFC822(const LongString &ls);
    virtual void fromRFC822(const QString &s);
    virtual void fromRFC822(const QByteArray &ba);

    enum MessageType
    {
        MMS     = 0x1,
        EMS     = 0x2,
        SMS     = 0x4,
        Email   = 0x8,
        System  = 0x10,
        Default = Email
    };

    void setType( int t );
    int type() const;
    QStringList phoneRecipients() const;
    QStringList mailRecipients() const;

    bool saveMail(QTextStream& out, const QString& checkPath = QString::null);
    bool encodeMail();
    bool encodeMail(QTextStream& out, bool privateHeaders = false, bool attachments = true,
                    const QString &checkPath = QString::null);

    QUuid uuid() const;
    void setUuid(const QUuid id);

    QString from() const;
    void setFrom(const QString &s);

    QString fromName() const;
    QString fromEmail() const;

    QString subject() const;
    void setSubject(const QString &s);

    QString dateString() const;
    void setDateString(const QString &s);

    QDateTime dateTime() const;
    void setDateTime(QDateTime d);

    QStringList to() const;
    void setTo(QStringList s);
    QStringList cc() const;
    void setCc(QStringList s);
    QStringList bcc() const;
    void setBcc(QStringList s);
// obsolete. use isSms to determine if the message is an sms
//    QStringList sms() const;
//    void setSms(QStringList s);

    QString replyTo() const;
    void setReplyTo(const QString &s);

    QString messageId() const;
    void setMessageId(const QString &s);

    QString inReplyTo() const;
    void setInReplyTo(const QString &s);

    EmailStatus status(EmailStatus es) const;
    void setStatus(EmailStatus es, bool b);

    uint allStatusFields() const;
    void setAllStatusFields(uint s);

    bool unfinished();

    QString fromAccount() const;
    void setFromAccount(const QString &s);

    QString fromMailbox() const;
    void setFromMailbox(const QString &s);

    QString serverUid() const;
    void setServerUid(const QString &s);

    uint size() const;
    void setSize(uint i);

    TransferEncoding transferEncoding() const { return _transferEncoding; }
    void setTransferEncoding( TransferEncoding t ) { _transferEncoding = t; }

    QString plainTextBody() const;
    void setPlainTextBody(const QString &s);

    QString htmlBody() const;
    void setHtmlBody(const QString &s);

    uint messagePartCount() const;
    void addMessagePart(const MailMessagePart part);
    void prependMessagePart(const MailMessagePart part);

    const MailMessagePart& messagePartAt(uint pos) const;
    MailMessagePart& messagePartAt(uint pos);

    void removeAllMessageParts();

    bool multipartRelated() const;
    void setMultipartRelated(bool value);

    QString header( const QString &headerName ) const;

    void setExtraHeader(const QString &name, const QString &value);

    static QDateTime parseDate(QString in);
    static QString formattedDateTimeString(QDateTime d);
    static void parseEmailAddress(QString fromStr, QString &nameStr, QString &emailStr);

    static QString decodeBase64( const QString &encoded );
    static QString decodeQuotedPrintable( const QString &s, bool RFC2047 = false );

    static QString encodeBase64(QByteArray fileData);
    static QString encodeQuotedPrintable( const QByteArray &fileData, bool RFC2047 = false );

    static QString quoteString(const QString &str);
    static QString unquoteString(const QString &str);

    static QString decodeEncodedWord(const QString &str);
    static QString encodeEncodedWord(const QString &str);
    static QString localeExpansion(const QString &str);
    static QString localeEncode(const QString& str);

private:
    void encodeHeaders();
    void setPhoneRecipients();
    void parseHeader(const QString &_rawMessageHeader);
    bool parseHeaderField(const QString &str);
    QString contents(const QString &str, const QString &target);
    QString headerFieldContent(QString str, QString target);

    void updateHeader(QString header, QString content);
    QString messageHeaders(bool encodePrivate = false) const;

    void parseBody();
    void parseMimePart(LongString body, QString boundary, int &bodyId);

    static int parse64base(char *src, char *bufOut);


private:
    LongString _rawMessageBody;
    QStringList _headerFields;
    QStringList _extraFields;   // Fields we don't know about explicitly

    QUuid _uuid;
    QString _from;
    QString _fromName, _fromEmail;
    QString _subject;
    QString _dateString;
    QDateTime _dateTime;
    QStringList _to, _cc, _bcc;
    QString _replyTo;
    QString _messageId;
    QString _inReplyTo;

    QString _mimeType;
    QString _boundary;

    EmailStatus _status;
    QString _fromAccount;
    QString _fromMailbox;
    QString _serverUid;
    uint _size;

    TransferEncoding _transferEncoding;
    QString _plainTextBody;
    QString _htmlBody;

    QList<MailMessagePart> _messageParts;

    bool _multipartRelated;

    QStringList _phoneRecipients;
};

class QTOPIAMAIL_EXPORT MailMessagePart
{
public:
    MailMessagePart();

    TransferEncoding transferEncoding() const { return _transferEncoding; }
    void setTransferEncoding( TransferEncoding t ) { _transferEncoding = t; }

    bool operator==(const MailMessagePart &orig);

    QString contentType() const;
    void setContentType(const QString &s);

    // not used, app needs to decice whether to display inline or not
    QString contentDisposition() const;
    void setContentDisposition(const QString &s);

    QString contentID() const;
    void setContentID(const QString &s);

    QString contentLocation() const;
    void setContentLocation(const QString &s);

    QString name() const;
    void setName(const QString &s);
    QString prettyName() const; // Never empty

    // Filename is mime header part, don't confuse with local filename and location
    QString filename() const;
    void setFilename(const QString &s);

    QString linkFile() const;
    void setLinkFile(const QString &s);

    QString storedFilename() const;
    void setStoredFilename(const QString &s);

    void setBody(QDataStream& instream, TransferEncoding te); //performs encoding
    int encodedSize() const;
    void setEncodedBody(const QString &s, TransferEncoding te);
    void setRawEncodedBody(const QByteArray b, TransferEncoding te);
    void setLongEncodedBody(const LongString &ls, TransferEncoding te);

    QString encodedBody() const;
    void encodedBody(QTextStream& out) const;
    QByteArray rawEncodedBody() const;
    QString decodedBody() const;
    bool decodedBody(QDataStream& result) const;

private:
    TransferEncoding _transferEncoding;
    QString _contentType;
    QString _contentDisposition;
    QString _name;
    QString _filename;
    LongString _body;

    QString _storedFilename;
    QString _linkFile;
    QString _contentID;
    QString _contentLocation;
};


#endif
