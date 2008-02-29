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
#ifndef QMAILMESSAGE_H
#define QMAILMESSAGE_H

#include "qmailaddress.h"
#include "qmailid.h"
#include "qmailtimestamp.h"

#include <QByteArray>
#include <QFlags>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>


class QMailMessagePart;
class QMailMessagePartContainerPrivate;
class QDataStream;
class QTextStream;
class QFile;

class QMailMessageHeaderFieldPrivate;

class QTOPIAMAIL_EXPORT QMailMessageHeaderField
{
public:
    typedef QPair<QByteArray, QByteArray> ParameterType;

    enum FieldType
    {
        StructuredField = 1,
        UnstructuredField = 2
    };

    QMailMessageHeaderField();
    QMailMessageHeaderField(const QByteArray& text, FieldType fieldType = StructuredField);
    QMailMessageHeaderField(const QByteArray& name, const QByteArray& text, FieldType fieldType = StructuredField);
    QMailMessageHeaderField(const QMailMessageHeaderField& other);
    virtual ~QMailMessageHeaderField();

    bool isNull() const;

    QByteArray id() const;
    void setId(const QByteArray& text);

    QByteArray content() const;
    void setContent(const QByteArray& text);

    QByteArray parameter(const QByteArray& name) const;
    void setParameter(const QByteArray& name, const QByteArray& value);

    bool isParameterEncoded(const QByteArray& name) const;
    void setParameterEncoded(const QByteArray& name);

    QList<ParameterType> parameters() const;

    virtual QByteArray toString(bool includeName = true, bool presentable = true) const;

    virtual QString decodedContent() const;

    bool operator== (const QMailMessageHeaderField& other) const;

    QMailMessageHeaderField& operator= (const QMailMessageHeaderField& other);

    static QByteArray encodeWord(const QString& input, const QByteArray& charset = "");
    static QString decodeWord(const QByteArray& input);

    static QByteArray encodeParameter(const QString& input, const QByteArray& charset = "", const QByteArray& language = "");
    static QString decodeParameter(const QByteArray& input);

    static QByteArray encodeContent(const QString& input, const QByteArray& charset = "");
    static QString decodeContent(const QByteArray& input);

    static QByteArray removeComments(const QByteArray& input);
    static QByteArray removeWhitespace(const QByteArray& input);

protected:
    void parse(const QByteArray& text, FieldType fieldType);

private:
    friend class QMailMessageHeaderFieldPrivate;
    friend class QMailMessageHeaderPrivate;

    void output(QDataStream& out) const;

    QSharedDataPointer<QMailMessageHeaderFieldPrivate> d;
};


class QTOPIAMAIL_EXPORT QMailMessageContentType : public QMailMessageHeaderField
{
public:
    QMailMessageContentType();
    QMailMessageContentType(const QByteArray& type);
    QMailMessageContentType(const QMailMessageHeaderField& field);

    QByteArray type() const;
    void setType(const QByteArray& type);

    QByteArray subType() const;
    void setSubType(const QByteArray& subType);

    QByteArray name() const;
    void setName(const QByteArray& name);

    QByteArray boundary() const;
    void setBoundary(const QByteArray& boundary);

    QByteArray charset() const;
    void setCharset(const QByteArray& charset);

private:
    // Don't allow the Id to be changed
    void setId(const QByteArray& text);
};


class QTOPIAMAIL_EXPORT QMailMessageContentDisposition : public QMailMessageHeaderField
{
public:
    enum DispositionType
    {
        None = 0,
        Inline = 1,
        Attachment = 2
    };

    QMailMessageContentDisposition();
    QMailMessageContentDisposition(const QByteArray& type);
    QMailMessageContentDisposition(DispositionType disposition);
    QMailMessageContentDisposition(const QMailMessageHeaderField& field);

    DispositionType type() const;
    void setType(DispositionType disposition);

    QByteArray filename() const;
    void setFilename(const QByteArray& filename);

    QMailTimeStamp creationDate() const;
    void setCreationDate(const QMailTimeStamp& timeStamp);

    QMailTimeStamp modificationDate() const;
    void setModificationDate(const QMailTimeStamp& timeStamp);

    QMailTimeStamp readDate() const;
    void setReadDate(const QMailTimeStamp& timeStamp);

    int size() const;
    void setSize(int size);

private:
    // Don't allow the Id to be changed
    void setId(const QByteArray& text);
};


class QMailMessageHeaderPrivate;

// This class is not exposed to clients:
class QMailMessageHeader
{
public:
    QMailMessageHeader();
    QMailMessageHeader(const QByteArray& input);
    QMailMessageHeader(const QMailMessageHeader& input);
    ~QMailMessageHeader();

    void update(const QByteArray& id, const QByteArray& content);
    void append(const QByteArray& id, const QByteArray& content);
    void remove(const QByteArray& id);

    QMailMessageHeaderField field(const QByteArray& id) const;
    QList<QMailMessageHeaderField> fields(const QByteArray& id) const;

    QList<const QByteArray*> fieldList() const;

    QMailMessageHeader& operator= (const QMailMessageHeader& other);

private:
    friend class QMailMessageHeaderPrivate;
    friend class QMailMessagePartContainerPrivate;
    friend class QMailMessagePartPrivate;
    friend class QMailMessagePrivate;

    void output(QDataStream& out, const QList<QByteArray>& exclusions, bool stripInternal) const;

    QSharedDataPointer<QMailMessageHeaderPrivate> d;
};


class QMailMessageBodyPrivate;

class QTOPIAMAIL_EXPORT QMailMessageBody
{
public:
    enum TransferEncoding 
    {
        NoEncoding = 0,
        SevenBit = 1, 
        EightBit = 2, 
        Base64 = 3,
        QuotedPrintable = 4,
        Binary = 5, 
    };

    enum EncodingStatus
    {
        AlreadyEncoded = 1,
        RequiresEncoding = 2
    };

    enum EncodingFormat
    {
        Encoded = 1,
        Decoded = 2
    };

    QMailMessageBody(const QMailMessageBody& other);
    ~QMailMessageBody();

    // Construction functions
    static QMailMessageBody fromFile(const QString& filename, const QMailMessageContentType& type, TransferEncoding encoding, EncodingStatus status);

    static QMailMessageBody fromStream(QDataStream& in, const QMailMessageContentType& type, TransferEncoding encoding, EncodingStatus status);
    static QMailMessageBody fromData(const QByteArray& input, const QMailMessageContentType& type, TransferEncoding encoding, EncodingStatus status);

    static QMailMessageBody fromStream(QTextStream& in, const QMailMessageContentType& type, TransferEncoding encoding);
    static QMailMessageBody fromData(const QString& input, const QMailMessageContentType& type, TransferEncoding encoding);

    // Output functions
    bool toFile(const QString& filename, EncodingFormat format) const;

    QByteArray data(EncodingFormat format) const;
    bool toStream(QDataStream& out, EncodingFormat format) const;

    QString data() const;
    bool toStream(QTextStream& out) const;

    // Property accessors
    TransferEncoding transferEncoding() const;
    QMailMessageContentType contentType() const;

    QMailMessageBody& operator= (const QMailMessageBody& other);

private:
    friend class QMailMessagePartContainerPrivate;

    QMailMessageBody();

    uint indicativeSize() const;

    void output(QDataStream& out, bool includeAttachments) const;

    QSharedDataPointer<QMailMessageBodyPrivate> d;
};

class QTOPIAMAIL_EXPORT QMailMessagePartContainer
{
public:
    enum MultipartType 
    {
        MultipartNone = 0,
        MultipartSigned = 1,
        MultipartEncrypted = 2,
        MultipartMixed = 3,
        MultipartAlternative = 4,
        MultipartDigest = 5,
        MultipartParallel = 6,
        MultipartRelated = 7,
        MultipartFormData = 8,
        MultipartReport = 9
    };

    virtual ~QMailMessagePartContainer();

    // Parts management interface:
    MultipartType multipartType() const;
    void setMultipartType(MultipartType type);

    uint partCount() const;
    void appendPart(const QMailMessagePart &part);
    void prependPart(const QMailMessagePart &part);

    const QMailMessagePart& partAt(uint pos) const;
    QMailMessagePart& partAt(uint pos);

    void clearParts();

    QByteArray boundary() const;
    void setBoundary(const QByteArray& text);

    // Body management interface:
    void setBody(const QMailMessageBody& body);
    QMailMessageBody body() const;

    bool hasBody() const;

    // Property accessors
    QMailMessageBody::TransferEncoding transferEncoding() const;
    QMailMessageContentType contentType() const;

    // Header fields describing this part container
    QString headerFieldText( const QString &id ) const;
    QMailMessageHeaderField headerField( const QString &id, QMailMessageHeaderField::FieldType fieldType = QMailMessageHeaderField::StructuredField ) const;

    QStringList headerFieldsText( const QString &id ) const;
    QList<QMailMessageHeaderField> headerFields( const QString &id, QMailMessageHeaderField::FieldType fieldType = QMailMessageHeaderField::StructuredField ) const;

    QList<QMailMessageHeaderField> headerFields() const;

    void setHeaderField( const QString &id, const QString& content );
    void setHeaderField( const QMailMessageHeaderField &field );

    void appendHeaderField( const QString &id, const QString& content );
    void appendHeaderField( const QMailMessageHeaderField &field );

    void removeHeaderField( const QString &id );

private:
    friend class QMailMessagePartContainerPrivate;

    void setHeader(const QMailMessageHeader& header, const QMailMessagePartContainerPrivate* parent = 0);

    uint indicativeSize() const;

    void outputParts(QDataStream& out, bool includePreamble, bool includeAttachments, bool stripInternal) const;
    void outputBody(QDataStream& out, bool includeAttachments) const;

    virtual QMailMessagePartContainerPrivate* partContainerPrivate() = 0;
    virtual const QMailMessagePartContainerPrivate* partContainerPrivate() const = 0;
};

class QMailMessagePartPrivate;

class QTOPIAMAIL_EXPORT QMailMessagePart : public QMailMessagePartContainer
{
public:
    QMailMessagePart();
    QMailMessagePart(const QMailMessagePart &other);
    virtual ~QMailMessagePart();

    // Construction functions
    static QMailMessagePart fromFile(const QString& filename, const QMailMessageContentDisposition& disposition, 
                                     const QMailMessageContentType& type, QMailMessageBody::TransferEncoding encoding, 
                                     QMailMessageBody::EncodingStatus status = QMailMessageBody::RequiresEncoding);

    static QMailMessagePart fromStream(QDataStream& in, const QMailMessageContentDisposition& disposition, 
                                       const QMailMessageContentType& type, QMailMessageBody::TransferEncoding encoding, 
                                       QMailMessageBody::EncodingStatus status = QMailMessageBody::RequiresEncoding);
    static QMailMessagePart fromData(const QByteArray& input, const QMailMessageContentDisposition& disposition, 
                                     const QMailMessageContentType& type, QMailMessageBody::TransferEncoding encoding, 
                                     QMailMessageBody::EncodingStatus status = QMailMessageBody::RequiresEncoding);

    static QMailMessagePart fromStream(QTextStream& in, const QMailMessageContentDisposition& disposition, 
                                       const QMailMessageContentType& type, QMailMessageBody::TransferEncoding encoding);
    static QMailMessagePart fromData(const QString& input, const QMailMessageContentDisposition& disposition, 
                                     const QMailMessageContentType& type, QMailMessageBody::TransferEncoding encoding);

    QString contentID() const;
    void setContentID(const QString &s);

    QString contentLocation() const;
    void setContentLocation(const QString &s);

    QString contentDescription() const;
    void setContentDescription(const QString &s);

    QMailMessageContentDisposition contentDisposition() const;
    void setContentDisposition(const QMailMessageContentDisposition& disposition);

    QString contentLanguage() const;
    void setContentLanguage(const QString &s);

    int partNumber() const;
    void setPartNumber(int);

    QString displayName() const;
    QString identifier() const;

    const QMailMessagePart& operator= (const QMailMessagePart& other);

    QString attachmentPath() const;
    bool detachAttachment(const QString& path);

private:
    friend class QMailMessagePrivate;
    friend class QMailMessagePartContainerPrivate;

    virtual QMailMessagePartContainerPrivate* partContainerPrivate();
    virtual const QMailMessagePartContainerPrivate* partContainerPrivate() const;

    void setAttachmentPath(const QString& path);

    void output(QDataStream& out, bool includeAttachments, bool stripInternal) const;

    QSharedDataPointer<QMailMessagePartPrivate> d;
};

class QMailMessagePrivate;

class QTOPIAMAIL_EXPORT QMailMessage : public QMailMessagePartContainer
{
public:
    // Mail content needs to use CRLF explicitly
    static const char CarriageReturn;
    static const char LineFeed;
    static const char* CRLF;

    QMailMessage();
    QMailMessage(const QMailMessage& other);
    virtual ~QMailMessage();

    static QMailMessage fromRfc2822(const QByteArray &ba);
    static QMailMessage fromRfc2822File(const QString& fileName);

    enum EncodingFormat
    {
        HeaderOnlyFormat = 1,
        StorageFormat = 2,
        TransmissionFormat = 3,
        IdentityFormat = 4
    }; 

    QByteArray toRfc2822(EncodingFormat format = TransmissionFormat) const;
    void toRfc2822(QDataStream& out, EncodingFormat format = TransmissionFormat) const;

    enum MailDataSelection
    {
        Header,
        HeaderAndBody	
    };

    explicit QMailMessage(const QMailId& id, const MailDataSelection& selection = HeaderAndBody );

    enum MessageType
    {
        Mms     = 0x1,
        Sms     = 0x4,
        Email   = 0x8,
        System  = 0x10,
        None    = 0,
        AnyType = Mms | Sms | Email | System
    };

    enum MessageStatusFlag {
        Incoming       = 0x0001,
        Outgoing       = 0x0002,
        Sent           = 0x0004,
        Replied        = 0x0008,
        RepliedAll     = 0x0010,
        Forwarded      = 0x0020,
        Downloaded     = 0x0040,
        Read           = 0x0080,
        Removed        = 0x0100,
        ReadElsewhere  = 0x0200,
    };
    Q_DECLARE_FLAGS(Status, MessageStatusFlag)

    QMailId id() const;
    void setId(QMailId id);

    QMailId parentFolderId() const;
    void setParentFolderId(QMailId val);

    MessageType messageType() const;
    void setMessageType(MessageType t);

    QMailAddress from() const;
    void setFrom(const QMailAddress &s);

    QString subject() const;
    void setSubject(const QString &s);

    QMailTimeStamp date() const;
    void setDate(const QMailTimeStamp &s);

    QList<QMailAddress> to() const;
    void setTo(const QList<QMailAddress>& s);
    void setTo(const QMailAddress& s);

    QList<QMailAddress> cc() const;
    void setCc(const QList<QMailAddress>& s);
    QList<QMailAddress> bcc() const;
    void setBcc(const QList<QMailAddress>& s);

    QList<QMailAddress> recipients() const;
    bool hasRecipients() const;

    QMailAddress replyTo() const;
    void setReplyTo(const QMailAddress &s);

    QString inReplyTo() const;
    void setInReplyTo(const QString &s);

    Status status() const;
    void setStatus(Status s);
    void setStatus(MessageStatusFlag flag, bool set);

    QString fromAccount() const;
    void setFromAccount(const QString &s);

    QString fromMailbox() const;
    void setFromMailbox(const QString &s);

    QString serverUid() const;
    void setServerUid(const QString &s);

    uint size() const;
    void setSize(uint i);

    uint indicativeSize() const;

    const QMailMessage& operator= (const QMailMessage& other);

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

    virtual QMailMessagePartContainerPrivate* partContainerPrivate();
    virtual const QMailMessagePartContainerPrivate* partContainerPrivate() const;

    bool uncommittedChanges() const;
    bool uncommittedMetadataChanges() const;
    void changesCommitted();

    QSharedDataPointer<QMailMessagePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMailMessage::Status)

typedef QList<QMailMessage> QMailMessageList;


// There is no good place to put this code; define it here, but we won't expose it to external clients
namespace QMail
{

    template<typename StringType>
    StringType unquoteString(const StringType& src)
    {
        // If a string has double-quote as the first and last characters, return the string
        // between those characters
        int length = src.length();
        if (length)
        {
            typename StringType::const_iterator const begin = src.constData();
            typename StringType::const_iterator const last = begin + length - 1;

            if ((last > begin) && (*begin == '"' && *last == '"'))
                return src.mid(1, length - 2);
        }

        return src;
    }

    template<typename StringType>
    StringType quoteString(const StringType& src)
    {
        StringType result("\"\"");

        // Return the input string surrounded by double-quotes, which are added if not present
        int length = src.length();
        if (length)
        {
            result.reserve(length + 2);

            typename StringType::const_iterator begin = src.constData();
            typename StringType::const_iterator last = begin + length - 1;

            if (*begin == '"')
                begin += 1;

            if ((last >= begin) && (*last == '"'))
                last -= 1;

            if (last >= begin)
                result.insert(1, StringType(begin, (last - begin + 1)));
        }

        return result;
    }

}

#endif
