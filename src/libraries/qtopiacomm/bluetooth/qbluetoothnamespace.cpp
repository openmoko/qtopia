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

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <qtopianamespace.h>
#include <qbluetoothnamespace.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>

#include <QString>
#include <QStringList>
#include <QList>
#include <QDebug>

using namespace QBluetooth;

/*!
    \enum QBluetooth::SDPProfile
    Defines possible profiles in use by the system.

    \value SerialPortProfile Represents Serial Port profile.
    \value DialupNetworkingProfile Represents Dialup Networking profile.
    \value LanAccessProfile Represents LAN Access profile.
    \value FaxProfile Represents FAX profile.
    \value ObjectPushProfile Represents OBEX Object Push profile.
    \value FileTransferProfile Represents OBEX File Transfer profile.
    \value DirectPrintingProfile Represents the OBEX Simple Printing Profile (SPP), Direct Printing service.
    \value HeadsetProfile Represents the Headset profile.
    \value HandsFreeProfile Represents the Hands Free profile.
    \value SimAccessProfile Represents the SIM Access profile.
    \value NetworkAccessPointProfile Represents the PAN NAP profile.
    \value GroupAdHocNetworkProfile Represents the PAN GN profile.
    \value PersonalAreaNetworkUserProfile Represents the PAN PANU profile.
    \value HardCopyReplacementProfile Represents the HCRP profile.
    \value AdvancedAudioSourceProfile Represents the A2DP Source profile.
    \value AdvancedAudioSinkProfile Represents the A2DP Sink profile.
    \value AudioVideoRemoteControlProfile Represents the AVRCP Controller (CT) profile.
    \value AudioVideoTargetProfile Represents the AVRCP Target (TG) profile.
 */

/*!
    \enum QBluetooth::ServiceClass
    Defines the service classes

    \value Positioning Device has Positioning services (Location identification)
    \value Networking Device has Networking services (LAN, Ad hoc, ...)
    \value Rendering Device has Rendering services (Printing, Speaker, ...)
    \value Capturing Device has Capturing services (Scanner, Microphone, ...)
    \value ObjectTransfer Device has Object Transfer services (Object Push, FTP)
    \value Audio Device has Audio services (Speaker, Microphone, Headset service, ...)
    \value Telephony Device has Telephony services (Cordless telephony, Modem, Headset service, ...)
    \value Information Device has Information services (WEB-server, WAP-server, ...)
*/

/*!
    \enum QBluetooth::DeviceMajor
    Defines the major class of a Bluetooth device.

    \value Miscellaneous Miscellaneous device
    \value Computer Computer device
    \value Phone Phone device
    \value LANAccess Some form of a local area network router
    \value AudioVideo Audio / Video device
    \value Peripheral Mouse, Joystick, Keyboard
    \value Imaging Camera or Scanner device
    \value Wearable Wearable Device
    \value Toy Toy device
    \value Uncategorized Uncategorized device
*/

/*!
    \enum QBluetooth::ServiceError
    Defines the possible errors that occur during service start / stop.

    \value NoError No error has occurred.
    \value NotRunning The service is not running.
    \value AlreadyRunning The service is already running.
    \value SDPServerError Trouble registering / unregistering from the SDP server.
    \value NoAvailablePort No ports free.
    \value NoSuchAdapter The local adapter is not valid.
    \value UnknownService The name of the service is invalid.
    \value UnknownError An unknown error has occurred.
*/

// Device class consists of 24 bits
// 11 LSB bits are Major service class
// Followed by 5 MajorNumber bits
// Followed by 6 MinorNumber bits
// Followed by 2 format bits
// e.g.

// 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
// .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..

// Bits 23-13 Are reserved for service class
// Bits 12-8 Are reserved for major device number
// Bits 7-2 Are reserved for minor device number
// Bits 1-0 are unused?
//
// Stolen from hcid manual page, with comments by DK
// Major service class byte allocation (from LSB to MSB):
//       Bit 1 (16): Positioning (Location identification)
//       Bit 2 (17): Networking (LAN, Ad hoc, ...)
//       Bit 3 (18): Rendering (Printing, Speaker, ...)
//       Bit 4 (19): Capturing (Scanner, Microphone, ...)
//       Bit 5 (20): Object Transfer (v-Inbox, v-Folder, ...)
//       Bit 6 (21): Audio (Speaker, Microphone, Headset service, ...)
//       Bit 7 (22): Telephony (Cordless telephony, Modem, Headset service, ...)
//       Bit 8 (23): Information (WEB-server, WAP-server, ...)

// These are bits 7, 6, 5, 4, 3, 2
const char *computer_minor_classes[] = {
    QT_TR_NOOP("Uncategorized"),
    QT_TR_NOOP("Desktop workstation"),
    QT_TR_NOOP("Server-class computer"),
    QT_TR_NOOP("Laptop"),
    QT_TR_NOOP("Handheld PC/PDA (clam shell)"),
    QT_TR_NOOP("Palm sized PC/PDA"),
    QT_TR_NOOP("Wearable computer (Watch sized)"),
    NULL
};

// These are bits 7, 6, 5, 4, 3, 2
const char *phone_minor_classes[] = {
    QT_TR_NOOP("Uncategorized"),
    QT_TR_NOOP("Cellular"),
    QT_TR_NOOP("Cordless"),
    QT_TR_NOOP("Smart phone"),
    QT_TR_NOOP("Wired modem or voice gateway"),
    QT_TR_NOOP("Common ISDN Access"),
    NULL
};

// These are bits 7, 6, 5
const char *lan_access_minor_classes[] = {
    QT_TR_NOOP("Fully available"),
    QT_TR_NOOP("17% utilized"),
    QT_TR_NOOP("17-33% utilized"),
    QT_TR_NOOP("33-50% utilized"),
    QT_TR_NOOP("50-67% utilized"),
    QT_TR_NOOP("67-83% utilized"),
    QT_TR_NOOP("83-99% utilized"),
    QT_TR_NOOP("No service available"),
    NULL
};

// These are bits 7, 6, 5, 4, 3, 2
const char *audio_video_minor_classes[] = {
    QT_TR_NOOP("Uncategorized, code not assigned"),
    QT_TR_NOOP("Wearable Headset Device"),
    QT_TR_NOOP("Hands-free Device"),
    QT_TR_NOOP("(Reserved)"),
    QT_TR_NOOP("Microphone"),
    QT_TR_NOOP("Loudspeaker"),
    QT_TR_NOOP("Headphones"),
    QT_TR_NOOP("Portable Audio"),
    QT_TR_NOOP("Car audio"),
    QT_TR_NOOP("Set-top box"),
    QT_TR_NOOP("HiFi Audio Device"),
    QT_TR_NOOP("VCR"),
    QT_TR_NOOP("Video Camera"),
    QT_TR_NOOP("Camcorder"),
    QT_TR_NOOP("Video Monitor"),
    QT_TR_NOOP("Video Display and Loudspeaker"),
    QT_TR_NOOP("Video Conferencing"),
    QT_TR_NOOP("(Reserved)"),
    QT_TR_NOOP("Gaming/Toy"),
    NULL
};

//Bits 7,6
const char *peripheral_minor_classes[] = {
    QT_TR_NOOP("Not Keyboard / Not Pointing Device"),
    QT_TR_NOOP("Keyboard"),
    QT_TR_NOOP("Pointing device"),
    QT_TR_NOOP("Combo keyboard/pointing device"),
    NULL
};

// Bits 5, 4, 3, 2 (combination with the above)
// E.g. can be Keyboard + Gamepad or something
const char *peripheral_device_field_minor_classes[] = {
    QT_TR_NOOP("Uncategorized device"),
    QT_TR_NOOP("Joystick"),
    QT_TR_NOOP("Gamepad"),
    QT_TR_NOOP("Remote control"),
    QT_TR_NOOP("Sensing device"),
    QT_TR_NOOP("Digitizer tablet"),
    QT_TR_NOOP("Card Reader"),
    NULL
};

// Bits 7, 6, 5, 4
const char *imaging_minor_classes[] = {
    QT_TR_NOOP("Uncategorized"),
    QT_TR_NOOP("Display"),
    QT_TR_NOOP("Camera"),
    QT_TR_NOOP("Camera"),
    QT_TR_NOOP("Scanner"),
    QT_TR_NOOP("Scanner"),
    QT_TR_NOOP("Scanner"),
    QT_TR_NOOP("Scanner"),
    QT_TR_NOOP("Printer"),
    QT_TR_NOOP("Printer"),
    QT_TR_NOOP("Printer"),
    QT_TR_NOOP("Printer"),
    QT_TR_NOOP("Printer"),
    QT_TR_NOOP("Printer"),
    QT_TR_NOOP("Printer"),
    QT_TR_NOOP("Printer"),
    NULL
};

// These are bits 7, 6, 5, 4, 3, 2
const char *wearable_minor_classes[] = {
    QT_TR_NOOP("Uncategorized"),
    QT_TR_NOOP("Wrist Watch"),
    QT_TR_NOOP("Pager"),
    QT_TR_NOOP("Jacket"),
    QT_TR_NOOP("Helmet"),
    QT_TR_NOOP("Glasses"),
    NULL
};

const char *toy_minor_classes[] = {
    QT_TR_NOOP("Uncategorized"),
    QT_TR_NOOP("Robot"),
    QT_TR_NOOP("Vehicle"),
    QT_TR_NOOP("Doll / Action Figure"),
    QT_TR_NOOP("Controller"),
    QT_TR_NOOP("Game"),
    NULL
};

static const char *minor_to_str(qint8 minor, const char *minors[])
{
    int i = 0;
    while (minors[i]) {
        i++;
    }

    if (minor < i) {
        return minors[minor];
    }

    return NULL;
}

/*!
    \internal
    Converts a device minor class to a String.  This is here since both
    QBluetoothLocalDevice and QBluetoothRemoteDevice use this.
*/
QString convertDeviceMinorToString(QBluetooth::DeviceMajor major, qint8 minor)
{
    const char *m;
    QString ret = QObject::tr("Uncategorized");

    switch(major) {
        case Miscellaneous:
            break;
        case Computer:
            m = minor_to_str(minor & 0x3F, computer_minor_classes);
            if (m != NULL) {
                ret = QObject::tr(m);
            }
            break;
        case Phone:
            m = minor_to_str(minor & 0x3F, phone_minor_classes);
            if (m != NULL) {
                ret = QObject::tr(m);
            }
            break;
        case LANAccess:
            m = minor_to_str((minor >> 3) & 0x7, lan_access_minor_classes);
            if (m != NULL) {
                ret = QObject::tr(m);
            }
            break;
        case AudioVideo:
            m = minor_to_str(minor & 0x3F, audio_video_minor_classes);
            if (m != NULL) {
                ret = QObject::tr(m);
            }
            break;
        case Peripheral:
            m = minor_to_str((minor >> 4) & 0x3, peripheral_minor_classes);
            if (m != NULL) {
                ret = QObject::tr(m);
            }
            break;
        case Imaging:
            m = minor_to_str((minor >> 2) & 0xF, imaging_minor_classes);
            if (m != NULL) {
                ret = QObject::tr(m);
            }
            break;
        case Wearable:
            m = minor_to_str(minor & 0x3F, wearable_minor_classes);
            if (m != NULL) {
                ret = QObject::tr(m);
            }
            break;
        case Toy:
            m = minor_to_str(minor & 0x3F, toy_minor_classes);
            if (m != NULL) {
                ret = QObject::tr(m);
            }
            break;
        case Uncategorized:
            ret = QObject::tr("Uncategorized");
            break;
        default:
            ret = QObject::tr("Invalid");
            break;
    };

    return ret;
}

/*! \internal
    Converts a device class to a String.  This is here since both QBluetoothLocalDevice
    and QBluetoothRemoteDevice use this.

    \ingroup qtopiabluetooth
 */
QString convertDeviceMajorToString(QBluetooth::DeviceMajor dev_class)
{
    QString ret;

    switch(dev_class) {
        case Miscellaneous:
            ret = QObject::tr("Miscellaneous");
            break;
        case Computer:
            ret = QObject::tr("Computer");
            break;
        case Phone:
            ret = QObject::tr("Phone");
            break;
        case LANAccess:
            ret = QObject::tr("LAN Access");
            break;
        case AudioVideo:
            ret = QObject::tr("Audio / Video");
            break;
        case Peripheral:
            ret = QObject::tr("Peripheral");
            break;
        case Imaging:
            ret = QObject::tr("Imaging");
            break;
        case Wearable:
            ret = QObject::tr("Wearable");
            break;
        case Toy:
            ret = QObject::tr("Toy");
            break;
        case Uncategorized:
            ret = QObject::tr("Uncategorized");
            break;
        default:
            ret = QObject::tr("Invalid");
            break;
    };

    return ret;
}

/*! \internal
    Converts a union of service classes to a list of strings.  This service class
    is different from the Major class.  This is used as a rudimentary form of the
    service discovery protocol, it is meant to provide a rudimentary understanding
    of what services the device might provide.

    \ingroup qtopiabluetooth
 */
QStringList convertServiceClassesToString(QBluetooth::ServiceClasses classes)
{
    QStringList ret;

    if (classes & Positioning) {
        ret.append(QObject::tr("Positioning"));
    }

    if (classes & Networking) {
        ret.append(QObject::tr("Networking"));
    }

    if (classes & Rendering) {
        ret.append(QObject::tr("Rendering"));
    }

    if (classes & Capturing) {
        ret.append(QObject::tr("Capturing"));
    }

    if (classes & ObjectTransfer) {
        ret.append(QObject::tr("Object Transfer"));
    }

    if (classes & Audio) {
        ret.append(QObject::tr("Audio"));
    }

    if (classes & Telephony) {
        ret.append(QObject::tr("Telephony"));
    }

    if (classes & Information) {
        ret.append(QObject::tr("Information"));
    }

    return ret;
}

QBluetooth::DeviceMajor major_to_device_major(quint8 major)
{
    if (major > 9)
        return QBluetooth::Uncategorized;

    return static_cast<QBluetooth::DeviceMajor>(major);
}

qint128::qint128()
{
    for (unsigned int i = 0; i < 16; i++)
        data[i] = 0;
}

qint128::qint128(const quint8 indata[16])
{
    for (unsigned int i = 0; i < 16; i++)
        data[i] = indata[i];
}

quint128::quint128()
{
    for (unsigned int i = 0; i < 16; i++)
        data[i] = 0;
}

quint128::quint128(const quint8 indata[16])
{
    for (unsigned int i = 0; i < 16; i++)
        data[i] = indata[i];
}

/*! \internal
    Converts a QSDPProfile enum into something that can be used by the bluez hcitool command.
    Here since both QSDP and QSDAP use this functionality.

    \ingroup qtopiabluetooth
 */
QString map_sdp_profile_to_bluez_name(SDPProfile profile)
{
    QString ret;

    switch (profile) {
        case SerialPortProfile:
            ret = "SP";
            break;
        case DialupNetworkingProfile:
            ret = "DUN";
            break;
        case LanAccessProfile:
            ret = "LAN";
            break;
        case FaxProfile:
            ret = "FAX";
            break;
        case ObjectPushProfile:
            ret = "OPUSH";
            break;
        case FileTransferProfile:
            ret = "FTP";
            break;
        case DirectPrintingProfile:
            ret = "PRINT";
            break;
        case HeadsetProfile:
            ret = "HS";
            break;
        //TODO: THIS DOES NOT EXIST YET!!!
        case HeadsetAudioGatewayProfile:
            ret = "HSAG";
            break;
        case HandsFreeProfile:
            ret = "HF";
            break;
        case HandsFreeAudioGatewayProfile:
            ret = "HFAG";
            break;
        case SimAccessProfile:
            ret = "SAP";
            break;
        case NetworkAccessPointProfile:
            ret = "NAP";
            break;
        case GroupAdHocNetworkProfile:
            ret = "GN";
            break;
        case PersonalAreaNetworkUserProfile:
            ret = "PANU";
            break;
        case HardCopyReplacementProfile:
            ret = "HCRP";
            break;
        case AdvancedAudioSourceProfile:
            ret = "A2SRC";
            break;
        case AdvancedAudioSinkProfile:
            ret = "A2SNK";
            break;
        case AudioVideoRemoteControlProfile:
            ret = "AVRCT";
            break;
        case AudioVideoTargetProfile:
            ret = "AVRTG";
            break;
        default:
            break;
    }

    return ret;
}

/*!
    \internal

    Converts a QString representation of a Bluetooth address to a bdaddr_t structure
    Used by BlueZ
 */
void str2bdaddr(const QString &addr, bdaddr_t *bdaddr)
{
    unsigned char b[6];

    b[5] = addr.mid(0, 2).toUShort(0, 16);
    b[4] = addr.mid(3, 2).toUShort(0, 16);
    b[3] = addr.mid(6, 2).toUShort(0, 16);
    b[2] = addr.mid(9, 2).toUShort(0, 16);
    b[1] = addr.mid(12, 2).toUShort(0, 16);
    b[0] = addr.mid(15, 2).toUShort(0, 16);

    memcpy(bdaddr, b, 6);
}

static const char hex_table[]={'0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'};

#define CONVERT(dest, offset, byte)\
    dest[offset+1] = hex_table[byte & 0x0f]; \
    dest[offset] = hex_table[(byte >> 4) & 0x0f]; \

/*!
    \internal

    Converts a bdaddr_t structure used by BlueZ to a string representation.
 */
QString bdaddr2str(const bdaddr_t *bdaddr)
{
    QByteArray arr("00:00:00:00:00:00");
    unsigned char b[6];
    memcpy(b, bdaddr, 6);

    CONVERT(arr, 15, b[0])
    CONVERT(arr, 12, b[1])
    CONVERT(arr, 9, b[2])
    CONVERT(arr, 6, b[3])
    CONVERT(arr, 3, b[4])
    CONVERT(arr, 0, b[5])

    return arr;
}

QString find_sdptool()
{

    QStringList paths = Qtopia::installPaths();
    for(int i = 0; i < paths.count(); ++i) {
        QString sdptool;
        sdptool.append(paths.at(i) + "bin/" + "sdptool");
        if (QFile::exists(sdptool)) {
            return sdptool;
        }
    }

    return QString();
}

/*!
    \internal

    Get the security options.
*/
bool _q_getSecurityOptions(int sockfd, QBluetooth::SecurityOptions &options)
{
    int lm = 0;
    socklen_t len;

    if (getsockopt(sockfd, SOL_RFCOMM, RFCOMM_LM, &lm, &len) < 0) {
        options = 0;
        return false;
    }

    if (lm & RFCOMM_LM_AUTH)
        options |= QBluetooth::Authenticated;
    if (lm & RFCOMM_LM_ENCRYPT)
        options |= QBluetooth::Encrypted;
    if (lm & RFCOMM_LM_SECURE)
        options |= QBluetooth::Secure;

    return options;
}

bool _q_setSecurityOptions(int sockfd, QBluetooth::SecurityOptions options)
{
    int lm = 0;
    if (options & QBluetooth::Authenticated)
        lm |= RFCOMM_LM_AUTH;
    if (options & QBluetooth::Encrypted)
        lm |= RFCOMM_LM_ENCRYPT;
    if (options & QBluetooth::Secure);
        lm |= RFCOMM_LM_SECURE;

    if (lm && setsockopt(sockfd, SOL_RFCOMM, RFCOMM_LM, &lm, sizeof(lm)) < 0)
    {
        return false;
    }

    return true;
}

class RegisterMetaTypes {
public:
    RegisterMetaTypes();
};

RegisterMetaTypes::RegisterMetaTypes()
{
//    qRegisterMetaType<QSDPUUID>("QSDPUUID");
    qRegisterMetaType<qint128>("qint128");
    qRegisterMetaType<quint128>("quint128");
    qRegisterMetaType<qint8>("qint8");
}

RegisterMetaTypes sdpMetaTypes;

Q_IMPLEMENT_USER_METATYPE_ENUM(QBluetooth::SDPProfile);
Q_IMPLEMENT_USER_METATYPE_ENUM(QBluetooth::SecurityOptions);
Q_IMPLEMENT_USER_METATYPE_ENUM(QBluetooth::ServiceError);

