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

#include <qbluetoothsdprecord.h>
#include <qbluetoothsdpuuid.h>

#include <QString>
#include <QUrl>

/*!
    \class QBluetoothSdpSequence
    \brief The QBluetoothSdpSequence class is a convenience wrapper used in QBluetoothSdpRecord.

    This class is a convenience class introduced for the purpose
    of distinguishing attributes that represent SDP Sequences
    from attributes that represent SDP Alternates.  Both SDP
    Alternate and Sequence types can be represented as
    sequences of \c QVariant objects.  The QBluetoothSdpSequence and
    QBluetoothSdpAlternative were introduced to make it easy to
    distinguish between these types.

    \sa QBluetoothSdpAlternative
*/

/*!
    \class QBluetoothSdpAlternative
    \brief The QBluetoothSdpAlternative class is a convenience wrapper used in QBluetoothSdpRecord.

    This class is a convenience class introduced for the
    purpose of distinguishing attributes that represent SDP
    Sequences from attributes that represent SDP Alternates.
    Both SDP Alternate and Sequence types can be represented
    as sequences of \c QVariant objects.  The QBluetoothSdpSequence
    and QBluetoothSdpAlternative were introduced to make it easy to
    distinguish between these types.

    \sa QBluetoothSdpSequence
 */

/*!
    \class QBluetoothSdpRecord
    \brief The QBluetoothSdpRecord class represents a bluetooth SDP record.

    The QBluetoothSdpRecord class encapsulates a Bluetooth SDP service record.
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
        \o \bold{QBluetoothSdpAlternative} - Represents a SDP Alternate type.
        \o \bold{QBluetoothSdpSequence} - Represents a SDP Sequence type.
        \o \bold{QBluetoothSdpUuid} - Represents a SDP UUID type.
        \o \bold{QUrl} - Represents a SDP URL type.
        \o \bold{QString} - Represents a SDP TEXT type.
        \o \bold{QByteArray} - Represents a SDP TEXT type that appears to contain binary data.
    \endlist

The attributes are stored as QVariants.

    \ingroup qtopiabluetooth
    \sa QVariant, QBluetoothSdpSequence, QBluetoothSdpAlternative
 */

static const int ServiceClassIDList = 0x0001;
static const int ProtocolDescriptorList = 0x0004;

static QBluetoothSdpUuid OBEXObjectPushUUID(static_cast<quint16>(0x1105));
static QBluetoothSdpUuid DirectPrintingUUID(static_cast<quint16>(0x1118));
static QBluetoothSdpUuid RFCOMMProtocol(static_cast<quint16>(0x0003));
static QBluetoothSdpUuid DialupNetworkingUUID(static_cast<quint16>(0x1103));

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
QBluetoothSdpRecord::QBluetoothSdpRecord()
{

}

/*!
    Deconstruct a SDP Service record.
 */
QBluetoothSdpRecord::~QBluetoothSdpRecord()
{

}

/*!
    Construct a SDP service record, copying contents from \a other.
*/
QBluetoothSdpRecord::QBluetoothSdpRecord(const QBluetoothSdpRecord &other)
{
    operator=(other);
}

/*!
    Assign the contents of \a other to the current SDP service record.
 */
QBluetoothSdpRecord &QBluetoothSdpRecord::operator=(const QBluetoothSdpRecord &other)
{
    if (this == &other)
        return *this;

    m_attrs = other.m_attrs;
    return *this;
}

/*!
    Returns the service name attribute.
*/
QString QBluetoothSdpRecord::serviceName() const
{
    return m_attrs.value(SVCNAME_PRIMARY).value<QString>();
}

/*!
    Sets the service name attribute to \a serviceName.
*/
void QBluetoothSdpRecord::setServiceName(const QString &serviceName)
{
    m_attrs.insert(SVCNAME_PRIMARY, QVariant::fromValue(serviceName));
}

/*!
    Returns the service description attribute.
*/
QString QBluetoothSdpRecord::serviceDescription() const
{
    return m_attrs.value(SVCDESC_PRIMARY).value<QString>();
}

/*!
    Sets the service description attribute to \a serviceDesc.
*/
void QBluetoothSdpRecord::setServiceDescription(const QString &serviceDesc)
{
    m_attrs.insert(SVCDESC_PRIMARY, QVariant::fromValue(serviceDesc));
}

/*!
    Returns the provider name attribute.
*/
QString QBluetoothSdpRecord::providerName() const
{
    return m_attrs.value(PROVNAME_PRIMARY).value<QString>();
}

/*!
    Sets the provider name attribute to \a providerName
*/
void QBluetoothSdpRecord::setProviderName(const QString &providerName)
{
    m_attrs.insert(PROVNAME_PRIMARY, QVariant::fromValue(providerName));
}

/*!
    Returns the Doc URL attribute.
*/
QUrl QBluetoothSdpRecord::docURL() const
{
    return m_attrs.value(DOC_URL).value<QUrl>();
}

/*!
    Sets the Doc URL attribute to \a docURL.
*/
void QBluetoothSdpRecord::setDocURL(const QUrl &docURL)
{
    m_attrs.insert(DOC_URL, QVariant::fromValue(docURL));
}

/*!
    Returns the Exec URL attribute.
 */
QUrl QBluetoothSdpRecord::execURL() const
{
    return m_attrs.value(CLNT_EXEC_URL).value<QUrl>();
}

/*!
    Sets the Exec URL attribute to \a execURL.
 */
void QBluetoothSdpRecord::setExecURL(const QUrl &execURL)
{
    m_attrs.insert(CLNT_EXEC_URL, QVariant::fromValue(execURL));
}

/*!
    Returns the Icon URL attribute.
 */
QUrl QBluetoothSdpRecord::iconURL() const
{
    return m_attrs.value(ICON_URL).value<QUrl>();
}

/*!
    Sets the Icon URL attribute to \a iconURL.
 */
void QBluetoothSdpRecord::setIconURL(const QUrl &iconURL)
{
    m_attrs.insert(ICON_URL, QVariant::fromValue(iconURL));
}

/*!
   Gets the ServiceID attribute.  Each service on the SDP Server is
   uniquely identified using this uuid.
*/
QBluetoothSdpUuid QBluetoothSdpRecord::id() const
{
    return m_attrs.value(SERVICE_ID).value<QBluetoothSdpUuid>();
}

/*!
   Sets the ServiceID attribute to \a id.  The \a id argument should be unique
   identifier of the service.
*/
void QBluetoothSdpRecord::setId(const QBluetoothSdpUuid &id)
{
    m_attrs.insert(SERVICE_ID, QVariant::fromValue(id));
}

/*!
    Returns the group id attribute.
*/
QBluetoothSdpUuid QBluetoothSdpRecord::group() const
{
    return m_attrs.value(GROUP_ID).value<QBluetoothSdpUuid>();
}

/*!
   Sets the GroupID attribute to \a group.  All services which belong to
   a Group Service Class will require this attribute.  All other services
   can be a part of one or more groups.  This is set through the
   browse group list attribute.
*/
void QBluetoothSdpRecord::setGroup(const QBluetoothSdpUuid &group)
{
    m_attrs.insert(GROUP_ID, QVariant::fromValue(group));
}

/*!
    Returns a list of unique identifiers of all browse groups this service is a part of.
*/
QList<QBluetoothSdpUuid> QBluetoothSdpRecord::browseGroups() const
{
    QList<QBluetoothSdpUuid> ret;

    QVariant browseAttr = m_attrs.value(BROWSE_GRP_LIST);
    if (!browseAttr.canConvert<QBluetoothSdpSequence>())
        return ret;

    QBluetoothSdpSequence list = browseAttr.value<QBluetoothSdpSequence>();

    foreach (QVariant attr, list) {
        if (!browseAttr.canConvert<QBluetoothSdpUuid>()) {
            qWarning("browseGroup list contains non-UUID elements");
            return QList<QBluetoothSdpUuid>();
        }

        ret.push_back(attr.value<QBluetoothSdpUuid>());
    }

    return ret;
}

/*!
    Sets a list of unique identifiers of all browse groups this service is a part of
    to \a groups.
*/
void QBluetoothSdpRecord::setBrowseGroups(const QList<QBluetoothSdpUuid> &groups)
{
    QBluetoothSdpSequence list;

    foreach (QBluetoothSdpUuid group, groups) {
        QVariant attr = QVariant::fromValue(group);
        list.push_back(attr);
    }

    m_attrs.insert(BROWSE_GRP_LIST, QVariant::fromValue(list));
}

/*!
    Returns a server specific record handle.
*/
quint32 QBluetoothSdpRecord::recordHandle() const
{
    return m_attrs.value(RECORD_HANDLE).value<quint32>();
}

/*!
    Sets a server specific record handle to \a handle.
*/
void QBluetoothSdpRecord::setRecordHandle(quint32 handle)
{
    m_attrs.insert(RECORD_HANDLE, QVariant::fromValue(handle));
}

/*!
   Returns a list of all attribute identifiers this service contains.
*/
QList<quint16> QBluetoothSdpRecord::attributeIds() const
{
    return m_attrs.keys();
}

/*!
    Tries to add an attribute \a attr with id \a id to the service.
    Returns false if the attribute already exists.
*/
bool QBluetoothSdpRecord::addAttribute(quint16 id, const QVariant &attr)
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
bool QBluetoothSdpRecord::removeAttribute(quint16 id)
{
    return m_attrs.remove(id) > 0;
}

/*!
    Gets the attribute with id \a id from the service.  If the attribute is not
    found, a null QSDPAttribute is returned.  For extra error information,
    you can pass in the \a ok flag, which specifies whether an error occurred,
    or an actual NULL attribute was returned.
*/
QVariant QBluetoothSdpRecord::attribute(quint16 id, bool *ok) const
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
    Clears all attributes.
*/
void QBluetoothSdpRecord::clearAttributes()
{
    m_attrs.clear();
}

/*!
    This method can be used to find out whether a SDP record
    is an implementation of a particular service class,
    given by \a serviceUuid parameter.

    This method returns true if the service class matches,
    false otherwise.
 */
bool QBluetoothSdpRecord::isInstance(const QBluetoothSdpUuid &serviceUuid) const
{
    QVariant var = attribute(ServiceClassIDList);
    if (!var.canConvert<QBluetoothSdpSequence>())
        return false;

    QBluetoothSdpSequence attrList = var.value<QBluetoothSdpSequence>();
    foreach (QVariant seqAttr, attrList) {
        if (seqAttr.canConvert<QBluetoothSdpUuid>()) {
            QBluetoothSdpUuid uuid = seqAttr.value<QBluetoothSdpUuid>();
            if (uuid == serviceUuid)
                return true;
        }
    }

    return false;
}

/*!
    This method can be used to find out whether a SDP record
    is an implementation of a particular service class,
    given by \a profile parameter.

    This method returns true if the service class matches,
    false otherwise.
 */
bool QBluetoothSdpRecord::isInstance(QBluetooth::SDPProfile profile) const
{
    return isInstance(QBluetoothSdpUuid::fromProfile(profile));
}

/*!
    For a family of services that work over the RFCOMM protocol,
    this method returns the RFCOMM channel the service is
    running on.  The \a service parameter specifies the
    service record to search.

    Returns the channel number on success, -1 if no channel
    number was found.
 */
int QBluetoothSdpRecord::rfcommChannel(const QBluetoothSdpRecord &service)
{
    QVariant var = service.attribute(ProtocolDescriptorList);
    if (!var.canConvert<QBluetoothSdpSequence>())
        return -1;

    QBluetoothSdpSequence attrList = var.value<QBluetoothSdpSequence>();
    foreach (QVariant i, attrList) {
        if (i.canConvert<QBluetoothSdpSequence>()) {
            QBluetoothSdpSequence seq = i.value<QBluetoothSdpSequence>();
            if ((seq[0].value<QBluetoothSdpUuid>() == RFCOMMProtocol) &&
                (seq.size() >= 2)) {
                if (!seq[1].canConvert<quint8>())
                    return -1;
                return seq[1].value<quint8>();
            }
        }
    }

    return -1;
}
