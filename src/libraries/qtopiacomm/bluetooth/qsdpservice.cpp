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

#include <qsdpservice.h>
#include <qsdpuuid.h>

#include <QString>
#include <QUrl>

/*!
    \class QSDPSequence
    \brief The QSDPSequence class is a convenience wrapper used in QSDPService.

    This class is a convenience class introduced for the purpose
    of distinguishing attributes that represent SDP Sequences
    from attributes that represent SDP Alternates.  Both SDP
    Alternate and Sequence types can be represented as
    sequences of \c QVariant objects.  The QSDPSequence and
    QSDPAlternative were introduced to make it easy to
    distinguish between these types.

    \sa QSDPAlternative
*/

/*!
    \class QSDPAlternative
    \brief The QSDPAlternative class is a convenience wrapper used in QSDPService.

    This class is a convenience class introduced for the
    purpose of distinguishing attributes that represent SDP
    Sequences from attributes that represent SDP Alternates.
    Both SDP Alternate and Sequence types can be represented
    as sequences of \c QVariant objects.  The QSDPSequence
    and QSDPAlternative were introduced to make it easy to
    distinguish between these types.

    \sa QSDPSequence
 */

/*!
    \class QSDPService
    \brief The QSDPService class represents a bluetooth SDP record.

    The QSDPService class encapsulates a Bluetooth SDP service record.
    The record is composed of Attributes, each attribute has a unique
    16 bit id.  The attributes can be of the following types:
    \list
        \o \bold{quint8} - Represents a SDP UINT8 type.
        \o \bold{qint8} - Represents a SDP INT8 type.
        \o \bold{quint16} - Represents a SDP UINT16 type.
        \o \bold{qint16} - Represents a SDP INT16 type.
        \o \bold{quint32} - Represents a SDP UINT32 type.
        \o \bold{qint32} - Represents a SDP INT32 type.
        \o \bold{quint64} - Represents a SDP UINT64 type.
        \o \bold{qint64} - Represents a SDP INT64 type.
        \o \bold{quint128} - Represents a SDP UINT128 type.
        \o \bold{qint128} - Represents a SDP INT128 type.
        \o \bold{bool} - Represents a SDP BOOL type.
        \o \bold{QSDPAlternative} - Represents a SDP Alternate type.
        \o \bold{QSDPSequence} - Represents a SDP Sequence type.
        \o \bold{QSDPUUID} - Represents a SDP UUID type.
        \o \bold{QUrl} - Represents a SDP URL type.
        \o \bold{QString} - Represents a SDP TEXT type.
        \o \bold{QByteArray} - Represents a SDP TEXT type that appears to contain binary data.
    \endlist

The attributes are stored as QVariants.

    \ingroup qtopiabluetooth
    \sa QVariant, QSDPSequence, QSDPAlternative
 */

static const int ServiceClassIDList = 0x0001;
static const int ProtocolDescriptorList = 0x0004;

static QSDPUUID OBEXObjectPushUUID(static_cast<quint16>(0x1105));
static QSDPUUID DirectPrintingUUID(static_cast<quint16>(0x1118));
static QSDPUUID RFCOMMProtocol(static_cast<quint16>(0x0003));
static QSDPUUID DialupNetworkingUUID(static_cast<quint16>(0x1103));

static const quint16 SVCNAME_PRIMARY =  0x0100;
static const quint16 SVCDESC_PRIMARY =  0x0101;
static const quint16 PROVNAME_PRIMARY = 0x0102;
static const quint16 DOC_URL = 0x000a;
static const quint16 CLNT_EXEC_URL = 0x000b;
static const quint16 ICON_URL = 0x000c;
static const quint16 SERVICE_ID = 0x0003;
static const quint16 GROUP_ID = 0x0200;
static const quint16 BROWSE_GRP_LIST = 0x0005;
static const quint16 RECORD_HANDLE = 0x0000;

/*!
    Construct a new empty SDP Service record.
 */
QSDPService::QSDPService()
{

}

/*!
    Deconstruct a SDP Service record.
 */
QSDPService::~QSDPService()
{

}

/*!
    Construct a SDP service record, copying contents from \a other.
*/
QSDPService::QSDPService(const QSDPService &other)
{
    operator=(other);
}

/*!
    Assign the contents of \a other to the current SDP service record.
 */
QSDPService &QSDPService::operator=(const QSDPService &other)
{
    if (this == &other)
        return *this;

    m_attrs = other.m_attrs;
    return *this;
}

/*!
    Returns the service name attribute.
*/
QString QSDPService::serviceName() const
{
    return m_attrs.value(SVCNAME_PRIMARY).value<QString>();
}

/*!
    Sets the service name attribute to \a serviceName.
*/
void QSDPService::setServiceName(const QString &serviceName)
{
    m_attrs.insert(SVCNAME_PRIMARY, QVariant::fromValue(serviceName));
}

/*!
    Returns the service description attribute.
*/
QString QSDPService::serviceDescription() const
{
    return m_attrs.value(SVCDESC_PRIMARY).value<QString>();
}

/*!
    Sets the service description attribute to \a serviceDesc.
*/
void QSDPService::setServiceDescription(const QString &serviceDesc)
{
    m_attrs.insert(SVCDESC_PRIMARY, QVariant::fromValue(serviceDesc));
}

/*!
    Returns the provider name attribute.
*/
QString QSDPService::providerName() const
{
    return m_attrs.value(PROVNAME_PRIMARY).value<QString>();
}

/*!
    Sets the provider name attribute to \a providerName
*/
void QSDPService::setProviderName(const QString &providerName)
{
    m_attrs.insert(PROVNAME_PRIMARY, QVariant::fromValue(providerName));
}

/*!
    Returns the Doc URL attribute.
*/
QUrl QSDPService::docURL() const
{
    return m_attrs.value(DOC_URL).value<QUrl>();
}

/*!
    Sets the Doc URL attribute to \a docURL.
*/
void QSDPService::setDocURL(const QUrl &docURL)
{
    m_attrs.insert(DOC_URL, QVariant::fromValue(docURL));
}

/*!
    Returns the Exec URL attribute.
 */
QUrl QSDPService::execURL() const
{
    return m_attrs.value(CLNT_EXEC_URL).value<QUrl>();
}

/*!
    Sets the Exec URL attribute to \a execURL.
 */
void QSDPService::setExecURL(const QUrl &execURL)
{
    m_attrs.insert(CLNT_EXEC_URL, QVariant::fromValue(execURL));
}

/*!
    Returns the Icon URL attribute.
 */
QUrl QSDPService::iconURL() const
{
    return m_attrs.value(ICON_URL).value<QUrl>();
}

/*!
    Sets the Icon URL attribute to \a iconURL.
 */
void QSDPService::setIconURL(const QUrl &iconURL)
{
    m_attrs.insert(ICON_URL, QVariant::fromValue(iconURL));
}

/*!
   Gets the ServiceID attribute.  Each service on the SDP Server is
   uniquely identified using this uuid.
*/
QSDPUUID QSDPService::id() const
{
    return m_attrs.value(SERVICE_ID).value<QSDPUUID>();
}

/*!
   Sets the ServiceID attribute to \a id.  The \a id argument should be unique
   identifier of the service.
*/
void QSDPService::setId(const QSDPUUID &id)
{
    m_attrs.insert(SERVICE_ID, QVariant::fromValue(id));
}

/*!
    Returns the group id attribute
*/
QSDPUUID QSDPService::group() const
{
    return m_attrs.value(GROUP_ID).value<QSDPUUID>();
}

/*!
   Sets the GroupID attribute to \a group.  All services which belong to
   a Group Service Class will require this attribute.  All other services
   can be a part of one or more groups.  This is set through the
   browse group list attribute.
*/
void QSDPService::setGroup(const QSDPUUID &group)
{
    m_attrs.insert(GROUP_ID, QVariant::fromValue(group));
}

/*!
    Returns a list of unique identifiers of all browse groups this service is a part of.
*/
QList<QSDPUUID> QSDPService::browseGroups() const
{
    QList<QSDPUUID> ret;

    QVariant browseAttr = m_attrs.value(BROWSE_GRP_LIST);
    if (!browseAttr.canConvert<QSDPSequence>())
        return ret;

    QSDPSequence list = browseAttr.value<QSDPSequence>();

    foreach (QVariant attr, list) {
        if (!browseAttr.canConvert<QSDPUUID>()) {
            qWarning("browseGroup list contains non-UUID elements");
            return QList<QSDPUUID>();
        }

        ret.push_back(attr.value<QSDPUUID>());
    }

    return ret;
}

/*!
    Sets a list of unique identifiers of all browse groups this service is a part of
    to \a groups.
*/
void QSDPService::setBrowseGroups(const QList<QSDPUUID> &groups)
{
    QSDPSequence list;

    foreach (QSDPUUID group, groups) {
        QVariant attr = QVariant::fromValue(group);
        list.push_back(attr);
    }

    m_attrs.insert(BROWSE_GRP_LIST, QVariant::fromValue(list));
}

/*!
    Returns a server specific record handle.
*/
quint32 QSDPService::recordHandle() const
{
    return m_attrs.value(RECORD_HANDLE).value<quint32>();
}

/*!
    Sets a server specific record handle to \a handle.
*/
void QSDPService::setRecordHandle(quint32 handle)
{
    m_attrs.insert(RECORD_HANDLE, QVariant::fromValue(handle));
}

/*!
   Returns a list of all attribute identifiers this service contains.
*/
QList<quint16> QSDPService::attributeIds() const
{
    return m_attrs.keys();
}

/*!
    Tries to add an attribute \a attr with id \a id to the service.
    Returns false if the attribute already exists.
*/
bool QSDPService::addAttribute(quint16 id, const QVariant &attr)
{
    if (m_attrs.contains(id))
        return false;

    m_attrs.insert(id, attr);

    return true;
}

/*!
    Removes the attribute with the specified id \a id from the service record.
    Returns true on success.  If the attribute is not found,
    nothing is done and false is returned.
*/
bool QSDPService::removeAttribute(quint16 id)
{
    return m_attrs.remove(id) > 0;
}

/*!
    Gets the attribute with id \a id from the service.  If the attribute is not
    found, a null QSDPAttribute is returned.  For extra error information,
    you can pass in the \a ok flag, which specifies whether an error occurred,
    or an actual NULL attribute was returned.
*/
QVariant QSDPService::attribute(quint16 id, bool *ok) const
{
    QVariant ret;

    bool retOk = false;

    if (m_attrs.contains(id)) {
        retOk = true;
        ret = m_attrs.value(id);
    }

    if (ok)
        *ok = retOk;

    return ret;
}

/*!
    This method can be used to find out whether a SDP record
    represented by \a service
    is an implementation of a particular service class,
    given by \a profile parameter.

    This method returns true if the service class matches,
    false otherwise.
 */
bool QSDPService::isInstance(const QSDPService &service,
                             QBluetooth::SDPProfile profile)
{
    QVariant var = service.attribute(ServiceClassIDList);
    if (!var.canConvert<QSDPSequence>())
        return false;

    QSDPSequence attrList = var.value<QSDPSequence>();
    foreach (QVariant seqAttr, attrList) {
        if (seqAttr.canConvert<QSDPUUID>()) {
            QSDPUUID uuid = seqAttr.value<QSDPUUID>();
            if (uuid == QSDPUUID::fromProfile(profile))
                return true;
        }
    }

    return false;
}

/*!
    For a family of services that work over the RFCOMM protocol,
    this method returns the RFCOMM channel the service is
    running on.  The \a service parameter specifies the
    service record to search.

    Returns the channel number on success, -1 if no channel
    number was found.
 */
int QSDPService::rfcommChannel(const QSDPService &service)
{
    QVariant var = service.attribute(ProtocolDescriptorList);
    if (!var.canConvert<QSDPSequence>())
        return -1;

    QSDPSequence attrList = var.value<QSDPSequence>();
    foreach (QVariant i, attrList) {
        if (i.canConvert<QSDPSequence>()) {
            QSDPSequence seq = i.value<QSDPSequence>();
            if ((seq[0].value<QSDPUUID>() == RFCOMMProtocol) &&
                (seq.size() >= 2)) {
                if (!seq[1].canConvert<quint8>())
                    return -1;
                return seq[1].value<quint8>();
            }
        }
    }

    return -1;
}
