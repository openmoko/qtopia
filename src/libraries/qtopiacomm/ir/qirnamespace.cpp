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

#include <qirnamespace.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/irda.h>

#include <QByteArray>
#include <QString>
#include <QVariant>
#include <QTextCodec>

using namespace QIr;

/*!
    \enum QIr::DeviceClass
    Defines possible DeviceTypes as defined by the IrDA standard.

    \value PlugNPlay Device supports PlugNPlay.
    \value PDA Device is a PDA.
    \value Computer Device is a Computer device.
    \value Printer Device is a printer.
    \value Modem Device is a modem.
    \value Fax Device is a fax.
    \value LAN Device is a LAN.
    \value Telephony Device is a telephony device.
    \value FileServer Device is a file server device.
    \value Communications Device is a communications device.
    \value Message Device is a messaging device.
    \value HTTP Device supports HTTP.
    \value OBEX Device supports the OBEX protocol.
 */

/*!
    \internal Converts a QIr::CharacterSet to a string useable by
    QTextCoded::codecForName
*/
QByteArray QIr::convert_charset_to_string(int set)
{
    switch (set) {
        case CS_ASCII:
            return "ASCII";
        case CS_ISO_8859_1:
            return "ISO-8859-1";
        case CS_ISO_8859_2:
            return "ISO-8859-2";
        case CS_ISO_8859_3:
            return "ISO-8859-3";
        case CS_ISO_8859_4:
            return "ISO-8859-4";
        case CS_ISO_8859_5:
            return "ISO-8859-5";
        case CS_ISO_8859_6:
            return "ISO-8859-6";
        case CS_ISO_8859_7:
            return "ISO-8859-7";
        case CS_ISO_8859_8:
            return "ISO-8859-8";
        case CS_ISO_8859_9:
            return "ISO-8859-9";
        case CS_UNICODE:
            return "UTF-16";
        default:
            qWarning("Charset not found!");
            return QByteArray();
    };
}

QVariant convert_ias_entry(struct irda_ias_set &entry)
{
    // Class name and attrib name are always in ASCII
    QString className = QString::fromAscii(entry.irda_class_name);
    QString attribName = QString::fromAscii(entry.irda_attrib_name);
    switch (entry.irda_attrib_type) {
        case IAS_STRING:
        {
            QByteArray value(reinterpret_cast<const char *>(entry.attribute.irda_attrib_string.string),
                             entry.attribute.irda_attrib_string.len);
            int charset = entry.attribute.irda_attrib_string.charset;

            QString val;

            if (charset == CS_ASCII) {
                val = QString::fromAscii(value.constData());
            }
            else if (charset == CS_ISO_8859_1) {
                val = QString::fromLatin1(value.constData());
            }
            else {
                QByteArray codecName = convert_charset_to_string(charset);
                QTextCodec *codec = QTextCodec::codecForName(codecName);
                val = codec->toUnicode(value);
            }

            return QVariant::fromValue(val);
        }

        case IAS_INTEGER:
        {
            return QVariant::fromValue(static_cast<uint>(entry.attribute.irda_attrib_int));
        }

        case IAS_OCT_SEQ:
        {
            QByteArray value(reinterpret_cast<const char *>(entry.attribute.irda_attrib_octet_seq.octet_seq),
                             entry.attribute.irda_attrib_octet_seq.len);
            return QVariant::fromValue(value);
        }

        default:
            return QVariant();
    };
}

void convert_to_hints(QIr::DeviceClasses classes, unsigned char hints[])
{
    if (classes & QIr::PlugNPlay)
        hints[0] |= HINT_PNP;
    if (classes & QIr::PDA)
        hints[0] |= HINT_PDA;
    if (classes & QIr::Computer)
        hints[0] |= HINT_COMPUTER;
    if (classes & QIr::Printer)
        hints[0] |= HINT_PRINTER;
    if (classes & QIr::Modem)
        hints[0] |= HINT_MODEM;
    if (classes & QIr::Fax)
        hints[0] |= HINT_FAX;
    if (classes & QIr::LAN)
        hints[0] |= HINT_LAN;

    if (classes & QIr::Telephony) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_TELEPHONY;
    }
    if (classes & QIr::FileServer) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_FILE_SERVER;
    }
    if (classes & QIr::Communications) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_COMM;
    }
    if (classes & QIr::Message) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_MESSAGE;
    }
    if (classes & QIr::HTTP) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_HTTP;
    }
    if (classes & QIr::OBEX) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_OBEX;
    }
}
