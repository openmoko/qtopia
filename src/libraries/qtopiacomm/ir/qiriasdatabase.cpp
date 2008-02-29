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

#include <qtopiacomm/private/qirnamespace_p.h>
#include <qiriasdatabase.h>
#include <qtopialog.h>

#include <sys/socket.h>
#include <linux/types.h>
#include <linux/irda.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>

#include <errno.h>
#include <stdio.h>

#include <QStringList>
#include <QTextCodec>
#include <QVariant>

/*!
    \class QIrIasDatabase
    \brief The QIrIasDatabase class provides access to the local IAS Database.

    IAS stands for Information Access Service.  The IAS database provides a way
    for applications to register information about the infrared services they
    provide.  Applications can also query remote IAS databases to find out
    what services the remote device supports.

    QIrIasDatabase class provides access to the local IAS Database, and gives
    the ability to add, query and delete attributes.  This class also allows
    to modify the advertised major classes this device supports.  These are
    generally referred to as service hints.

    \ingroup qtopiair
 */

/*!
    Sets a new attribute into the IAS Database.  The \a attr parameter
    holds the attribute to be set.  Attr can be of the following types:

    \list
        \o \bold{QString} - Represents a String IAS attribute.
        \o \bold{uint} - Represents an Integer IAS attribute.
        \o \bold{QByteArray} - Represents an octet sequence IAS attribute.
    \endlist

    Any other type will be treated as invalid.  The \a className parameter
    holds the class name of the IAS attribute.  The \a attribName parameter
    holds the attribute name of the attribute.

    NOTE: Under linux this function requires administrator privileges.
 */
bool QIrIasDatabase::setAttribute(const QString &className,
                                  const QString &attribName,
                                  const QVariant &attr)
{
    if (className.size() > IAS_MAX_CLASSNAME)
        return false;

    if (attribName.size() > IAS_MAX_ATTRIBNAME)
        return false;

    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1 )
        return false;

    qLog(Infrared) << "Successfully opened Infrared socket";

    struct irda_ias_set entry;
    strcpy(entry.irda_class_name, className.toAscii().constData());
    strcpy(entry.irda_attrib_name, attribName.toAscii().constData());
    switch (attr.type()) {
        case QVariant::String:
        {
            QByteArray value;

            qLog(Infrared) << "Attribute is a string...";
            entry.irda_attrib_type = IAS_STRING;

            value = attr.value<QString>().toAscii();
            // Linux only supports the ASCII charset right now
            entry.attribute.irda_attrib_string.charset =
                    CS_ASCII;
            int len = value.length() < IAS_MAX_STRING ?
                    value.length() : IAS_MAX_STRING;
            entry.attribute.irda_attrib_string.len = len;
            strncpy(reinterpret_cast<char *>(entry.attribute.irda_attrib_string.string),
                    value.constData(), IAS_MAX_STRING);
            break;
        }
        case QVariant::UInt:
        {
            entry.irda_attrib_type = IAS_INTEGER;
            entry.attribute.irda_attrib_int = attr.value<uint>();
            break;
        }
        case QVariant::ByteArray:
        {
            entry.irda_attrib_type = IAS_OCT_SEQ;
            int len = attr.value<QByteArray>().length() < IAS_MAX_OCTET_STRING ?
                    attr.value<QByteArray>().length() : IAS_MAX_OCTET_STRING;
            entry.attribute.irda_attrib_octet_seq.len = len;
            strncpy(reinterpret_cast<char *>(entry.attribute.irda_attrib_octet_seq.octet_seq),
                    attr.value<QByteArray>().constData(), IAS_MAX_OCTET_STRING);
            break;
        }
        default:
            return false;
    }

    int status = setsockopt(fd, SOL_IRLMP, IRLMP_IAS_SET, &entry, sizeof(entry));
    if (status != 0) {
        qLog(Infrared) << "Error is: " << strerror(errno);
    }

    close(fd);

    return status == 0;

}

/*!
    Gets an attribute from the local IAS Database.  The \a className
    parameter specifies the class name of the attribute.  The
    \a attribName parameter specifies the attribute name.  Returns an
    invalid QVariant in case of failure, and valid otherwise.
    The value of QVariant can be one of:
    \list
        \o \bold{QString} - Represents a String IAS attribute.
        \o \bold{uint} - Represents an Integer IAS attribute.
        \o \bold{QByteArray} - Represents an octet sequence IAS attribute.
    \endlist

    NOTE: Under linux this function requires administrator privileges.
 */
QVariant QIrIasDatabase::attribute(const QString &className, const QString &attribName)
{
    if (className.size() > IAS_MAX_CLASSNAME)
        return QVariant();

    if (attribName.size() > IAS_MAX_ATTRIBNAME)
        return QVariant();

    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1 )
        return QVariant();

    qLog(Infrared) << "Successfully opened Infrared socket";

    struct irda_ias_set entry;
    strcpy(entry.irda_class_name, className.toAscii().constData());
    strcpy(entry.irda_attrib_name, attribName.toAscii().constData());

    socklen_t len = sizeof(entry);

    int status = getsockopt(fd, SOL_IRLMP, IRLMP_IAS_GET, &entry, &len);
    close(fd);

    if (status != 0)
        return QVariant();

    return convert_ias_entry(entry);
}

/*!
    Attempts to remove the attribute from the local IAS Database.
    The \a className parameter specifies the class name of the attribute.
    The \a attribName parameter specifies the attribute name.
    Returns true if the attribute could be removed, false otherwise.

    NOTE: Under linux this function requires administrator privileges.
*/
bool QIrIasDatabase::removeAttribute(const QString &className,
                                     const QString &attribName)
{
    if (className.size() > IAS_MAX_CLASSNAME)
        return false;

    if (attribName.size() > IAS_MAX_ATTRIBNAME)
        return false;

    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1 )
        return false;

    qLog(Infrared) << "Successfully opened Infrared socket";

    struct irda_ias_set entry;
    strcpy(entry.irda_class_name, className.toAscii().constData());
    strcpy(entry.irda_attrib_name, attribName.toAscii().constData());

    int status = setsockopt(fd, SOL_IRLMP, IRLMP_IAS_DEL, &entry, sizeof(entry));
    close(fd);

    return status == 0;
}
