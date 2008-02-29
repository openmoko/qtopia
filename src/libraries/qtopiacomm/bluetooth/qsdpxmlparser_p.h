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

#ifndef __QSDPXMLPARSER_P_H__
#define __QSDPXMLPARSER_P_H__

#include <QXmlDefaultHandler>

#include <QStack>
#include <QVariant>
#include <QBluetoothSdpRecord>
#include <QString>

#include <qtopiaglobal.h>

class QTOPIACOMM_EXPORT QSdpXmlHandler : public QXmlDefaultHandler
{
public:
    QSdpXmlHandler();

    bool startElement(const QString &namespaceURI,
                      const QString &localName,
                      const QString &qName,
                      const QXmlAttributes &attributes);

    bool endElement(const QString &namespaceURI,
                    const QString &localName,
                    const QString &qName);

    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    QString errorString() const;

    const QBluetoothSdpRecord &record() const;
    void reset();

private:
    enum Encoding { Default, Hex };

    bool parseElement(const QString &tag,
                      const QString &name,
                      const QString &value,
                      QSdpXmlHandler::Encoding encoding);

    bool parseBoolElement(const QString &value);
    bool parseUInt8Element(const QString &value);
    bool parseUInt16Element(const QString &value);
    bool parseUInt32Element(const QString &value);
    bool parseUInt64Element(const QString &value);
    bool parseUInt128Element(const QString &value);
    bool parseInt8Element(const QString &value);
    bool parseInt16Element(const QString &value);
    bool parseInt32Element(const QString &value);
    bool parseInt64Element(const QString &value);
    bool parseInt128Element(const QString &value);
    bool parseUuidElement(const QString &value);
    bool parseUrlElement(const QString &value);
    bool parseTextElement(const QString &value, QSdpXmlHandler::Encoding encoding);
    bool parseNilElement();

    QString m_errorString;
    bool m_isRecord;
    QStack<QVariant> m_stack;
    int m_currentId;
    QBluetoothSdpRecord m_record;

    Q_DISABLE_COPY(QSdpXmlHandler)
};

class QSdpXmlParser_Private;

class QTOPIACOMM_EXPORT QSdpXmlParser
{
public:
    enum Error { NoError, ParseError };

    QSdpXmlParser();
    ~QSdpXmlParser();

    bool parseRecord(const QByteArray &data);
    bool parseRecord(QIODevice *device);
    QSdpXmlParser::Error lastError() const;
    const QBluetoothSdpRecord &record() const;
    QString errorString() const;

private:
    QSdpXmlParser_Private *m_data;
    Q_DISABLE_COPY(QSdpXmlParser)
};

#endif
