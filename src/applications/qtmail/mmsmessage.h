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

#include <qwsppdu.h>
#include <qstring.h>

#include <qtopiaglobal.h>

class MMSMessage
{
public:
    MMSMessage();

    enum Type { Invalid=0, MSendReq=128, MSendConf=129, MNotificationInd=130,
                MNotifyResp=131, MRetrieveConf=132, MAckowledgeInd=133,
                MDeliveryInd=134 };

    bool decode(QIODevice *d);
    bool encode(QIODevice *d);

    Type type() const;
    void setType(Type t);

    const QList<QWspField> &headers() const { return fields; }
    const QWspField *field(const QString &name) const;
    void addField(const QString &name, const QString &value);
    void addField(const QString &name, quint32 value);
    void removeField(const QString &name);

    bool multipartRelated() const;

    int messagePartCount() const { return multiPartData.count(); }
    const QWspPart &messagePart(int idx) const;
    void addMessagePart(const QWspPart &part);

    const QString &errorMessage() const { return err; }

private:
    bool encodeSendRequest(QWspPduEncoder &);
    bool encodeNotificationInd(QWspPduEncoder &enc);
    bool encodeNotifyInd(QWspPduEncoder &enc);
    bool encodeAcknowledgeInd(QWspPduEncoder &enc);

private:
    QList<QWspField> fields;
    QWspMultipart multiPartData;
    QString err;
};

