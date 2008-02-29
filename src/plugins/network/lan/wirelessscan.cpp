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

#include "wirelessscan.h"

#ifndef NO_WIRELESS_LAN

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <QAction>
#endif
#include <QApplication>
#include <QDialog>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

#include <qtopiaapplication.h>
#include <qtopialog.h>
#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qsoftmenubar.h>
#endif
#include <qnetworkdevice.h>

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

WirelessScan::WirelessScan( const QString& ifaceName, QObject* parent )
    : QObject( parent ), iface( ifaceName ), sockfd( -1 )
{
}

WirelessScan::~WirelessScan()
{
}

/*!
    This function obtains the range information of the wireless interface. The value of \a weVersion
    must be checked before using \a range. \a weVersion is 0 if the device is not a wireless device.


    This function
    returns 0 if the interface associated to this object is not a wireless interface. A
    return value >0 is equal to \a weVersion.

    Wireless extensions versioning was introduced by WE v11. If the driver for the wireless
    network device doesn't support versioning ( WE <= 10 ) we do a lucky guess and assume version 9.

    The structure of iw_range has been reshuffled and increased in WE version 16. For
    simplicity this function does not return a valid range parameter if  \a weVersion < 16.
    You must check \a weVersion before \a range is used.
*/
void WirelessScan::rangeInfo(iw_range* range, int* weVersion)
{
    *weVersion = 0;
    int socket = ::socket( AF_INET, SOCK_DGRAM, 0 );
    if ( socket < 0 )
        return;

    char buffer[sizeof(struct iw_range) * 2];
    memset( buffer, 0, sizeof(buffer) );

    struct iwreq wrq;
    wrq.u.data.flags = 0;
    wrq.u.data.pointer = 0;
    wrq.u.data.length = sizeof(buffer);
    wrq.u.data.pointer = buffer;
    strncpy( wrq.ifr_name, iface.toLatin1().constData(), IFNAMSIZ );

    if ( ioctl( socket, SIOCGIWRANGE, &wrq ) < 0 ) {
        //no Wireless Extension
        ::close( socket );
        return;
    }

    memcpy( range, buffer, sizeof(struct iw_range) );

    // struct iw_range changed but we_version_compiled is still at the same offset. Hence
    // we can access the version no matter what WE version we have
    if ( wrq.u.data.length >= 300 ) {
        *weVersion = range->we_version_compiled;
    } else {
        //everything up to version 10
        ::close( socket );
        *weVersion = 9;
    }

    ::close( socket );
    return;
}

//TODO document WirelessScan::ConnectionState

/*!
    Returns the connectivity state of the wireless interface.
*/
WirelessScan::ConnectionState WirelessScan::deviceState() const
{
    //TODO support for IPv4 only (PF_INET6)
    int inetfd = socket( PF_INET, SOCK_DGRAM, 0 );
    if ( inetfd == -1 )
        return InterfaceUnavailable;

    int flags = 0;
    struct ifreq ifreqst;
    ::strcpy( ifreqst.ifr_name, iface.toLatin1().constData() );
    int ret = ioctl( inetfd, SIOCGIFFLAGS, &ifreqst );
    if ( ret == -1 ) {
        ::close( inetfd );
        return InterfaceUnavailable;
    }

    ::close( inetfd );

    flags = ifreqst.ifr_flags;
    if ( ( flags & IFF_UP ) == IFF_UP  &&
            (flags & IFF_LOOPBACK) != IFF_LOOPBACK &&
            (flags & IFF_BROADCAST) == IFF_BROADCAST ) {
        return Connected;
    }

    return NotConnected;
}

/*!
  Returns the MAC address of the access point we are connected to. If the device is not connected to a
  wireless network the returned string is empty.

  \sa deviceState()
*/
QString WirelessScan::currentAccessPoint() const
{
    QString result;
    if ( deviceState() != Connected )
        return result;

    struct iwreq wrq;

    int fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 )
        return result;

    strncpy( wrq.ifr_name, iface.toLatin1().constData(), IFNAMSIZ );
    int retCode = ioctl( fd, SIOCGIWAP, &wrq );
    if ( retCode >= 0 ) {
        struct ether_addr* eth = (struct ether_addr*)(&(wrq.u.ap_addr.sa_data));
                    result.sprintf("%02X:%02X:%02X:%02X:%02X:%02X", eth->ether_addr_octet[0],
                            eth->ether_addr_octet[1], eth->ether_addr_octet[2],
                            eth->ether_addr_octet[3], eth->ether_addr_octet[4],
                            eth->ether_addr_octet[5]);

    }
    ::close( fd );
    return result;
}

/*!
  Returns the ESSID of the access point we are connected to. If the device is not connected to a
  wireless network the returned string is empty.

  \sa deviceState()
*/
QString WirelessScan::currentESSID() const
{
    QString result;
    if ( deviceState() != Connected )
        return result;

    struct iwreq wrq;

    int fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 )
        return result;

    char buffer[IW_ESSID_MAX_SIZE+1];

    wrq.u.essid.flags = 0;
    wrq.u.essid.length = IW_ESSID_MAX_SIZE+1;
    wrq.u.essid.pointer = buffer;
    strncpy( wrq.ifr_name, iface.toLatin1().constData(), IFNAMSIZ );
    int retCode = ioctl( fd, SIOCGIWESSID, &wrq );
    if ( retCode >= 0 ) {
        buffer[wrq.u.essid.length] = '\0';
        result = buffer;
    }
    ::close( fd );
    return result;
}

const QList<WirelessNetwork> WirelessScan::results() const
{
    return entries;
}

bool WirelessScan::startScanning()
{
    struct iw_range range;
    int weVersion;
    rangeInfo( &range, &weVersion );
    static bool showExtendedLog = true;
    if ( showExtendedLog && weVersion < 14 ) {
        qLog(Network) << "WE version 14+ is required for wireless network scanning on interface" << iface;
        return false;
    }

#if WIRELESS_EXT > 13
    if ( showExtendedLog && qtopiaLogEnabled("Network") ) {
        qLog(Network) << "driver on interface" << iface << "supports WE version" << range.we_version_source;
        qLog(Network) << "compiled with WE version" << range.we_version_compiled;
        showExtendedLog = false;
    }

    if (sockfd > 0) {
        qLog(Network) << "Scanning process active";
        return false; //we are in the process of scanning, dont start yet another scan
    }

    qLog(Network) << "Scanning for wireless networks...";
    //open socket
    sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( sockfd < 0 )
        return false;

    //initiate scanning for wireless networks
    struct iwreq wrq;
    wrq.u.data.flags = 0;
    wrq.u.data.length = 0;
    wrq.u.data.pointer = 0;
    strncpy( wrq.ifr_name, iface.toLatin1().constData(), IFNAMSIZ );

    if ( ioctl( sockfd, SIOCSIWSCAN, &wrq ) < 0 ) {
        if ( qLogEnabled(Network) )
            perror("wireless scan initiation");
        ::close( sockfd );
        sockfd = -1;
        return false;
    }
    QTimer::singleShot( 300, this, SLOT(checkResults()) );

    return true;
#else
    return false;
#endif
}

void WirelessScan::checkResults()
{
    unsigned char* buffer = 0;
    int blength = IW_SCAN_MAX_DATA;
    struct iwreq wrq;

    struct iw_range range;
    int weVersion;
    rangeInfo( &range, &weVersion );
    if ( weVersion < 14 )
        return; //no scan support if WE version <14

#if WIRELESS_EXT > 13
    do {
        //we may have to allocate more memory later on
        unsigned char* temp = (unsigned char *)realloc( buffer, blength );
        if ( !temp ) {
            if ( buffer )
                free( buffer );
            return;
        }
        buffer = temp;

        wrq.u.data.flags = 0;
        wrq.u.data.length = blength;
        wrq.u.data.pointer = (char*)buffer;
        strncpy( wrq.ifr_name, iface.toLatin1().constData(), IFNAMSIZ );

        if ( ioctl( sockfd, SIOCGIWSCAN, &wrq ) < 0 ) {
            if ( errno == EAGAIN ) {
                // still busy scanning => try again later
                QTimer::singleShot( 200, this, SLOT(checkResults()) );
                free(buffer);
                return;
            } else if ( (weVersion > 16) && (errno == E2BIG) ) {
                //scan results can be quite large
                if ( wrq.u.data.length > blength )
                    blength = wrq.u.data.length;
                else
                    blength = blength + IW_SCAN_MAX_DATA;
                continue;
            } else {
                //hmm don't really know what this could be
                perror( "error:" );
                free( buffer );
                ::close( sockfd );
                sockfd = -1;
                return;
            }
        }
        break;
    } while ( true );

    if ( wrq.u.data.length > 0 ) {
        readData( buffer, wrq.u.data.length, weVersion, &range );
    }

    free( buffer );
    ::close( sockfd );
    sockfd = -1;
    emit scanningFinished();
#endif
}

static const char* operationMode[] =
{
        "Auto",
        "Ad-hoc",
        "Managed",
        "Master",
        "Repeater",
        "Secondary",
        "Monitor"
};

//compatibility with WE 19
#define IW_EV_POINT_OFF (((char *) &(((struct iw_point *) NULL)->length)) - (char *) NULL)

//compatibility with WE 16 and below
//these defines were added in WE 17
#ifndef IW_QUAL_QUAL_INVALID
#define IW_QUAL_QUAL_INVALID    0x10
#endif
#ifndef IW_QUAL_LEVEL_INVALID
#define IW_QUAL_LEVEL_INVALID   0x20
#endif
#ifndef IW_QUAL_NOISE_INVALID
#define IW_QUAL_NOISE_INVALID   0x40
#endif

void WirelessScan::readData( unsigned char* data, int length, int weVersion, iw_range* range )
{
#if WIRELESS_EXT > 13
    unsigned char* dataEnd = data + length;
    entries.clear();

    WirelessNetwork net;
    while ( data + IW_EV_LCP_LEN <= dataEnd ) {
        struct iw_event iwevent;
        char * iwp = (char*) (&iwevent);

        //copy event header
        memcpy( (char*) iwp, data, IW_EV_LCP_LEN );

        //an empty event?
        if ( iwevent.len <= IW_EV_LCP_LEN )
            return;

        unsigned char * value = data + IW_EV_LCP_LEN;
        iwp = (char*)iwp + IW_EV_LCP_LEN;
        switch ( iwevent.cmd ) {
            case  SIOCSIWCOMMIT:            /* Commit pending changes to driver */
                break;
            case  SIOCGIWNAME:              /* get name == wireless protocol */
                memcpy( iwp , value, IW_EV_CHAR_LEN - IW_EV_LCP_LEN);
                net.setData( WirelessNetwork::Protocol, QString( iwevent.u.name ) );
                break;
            case  SIOCSIWNWID:              /* set network id (pre-802.11) */
                break;
            case  SIOCGIWNWID:              /* get network id (the cell) */
                memcpy( iwp, value, IW_EV_PARAM_LEN-IW_EV_LCP_LEN );
                if ( iwevent.u.nwid.disabled )
                    net.setData( WirelessNetwork::NWID, tr("off") );
                else
                    net.setData( WirelessNetwork::NWID, QString::number( iwevent.u.nwid.value ) );
                break;
            case  SIOCSIWFREQ:              /* set channel/frequency (Hz) */
                break;
            case  SIOCGIWFREQ:              /* get channel/frequency (Hz) */
                {
                    memcpy( iwp, value, IW_EV_FREQ_LEN-IW_EV_LCP_LEN );
                    double val = ((double)iwevent.u.freq.m) * pow(10, iwevent.u.freq.e);
                    if ( val < 1e3 ) {
                        //we have a channel number
                        net.setData( WirelessNetwork::Channel, QString::number(val) );
                    } else {
                        //TODO match channel against frequency ->check iw_range
                        //we got the frequency
                        net.setData( WirelessNetwork::Frequency, QString::number(val) );
                    }
                }
                break;
            case  SIOCSIWMODE:              /* set operation mode */
                break;
            case  SIOCGIWMODE:              /* get operation mode */
                //IW_EV_UINT_LEN
                memcpy( iwp, value, IW_EV_UINT_LEN - IW_EV_LCP_LEN );
                net.setData( WirelessNetwork::Mode, operationMode[iwevent.u.mode] );
                break;
            //case  SIOCSIWSENS:              /* set sensitivity (dBm) */
            //case  SIOCGIWSENS:              /* get sensitivity (dBm) */
            //case  SIOCSIWRANGE:             /* Unused */
            //case  SIOCGIWRANGE:             /* Get range of parameters */
            //case  SIOCSIWPRIV:              /* Unused */
            //case  SIOCGIWPRIV:              /* get private ioctl interface info */
            //case  SIOCSIWSTATS:             /* Unused */
            //case  SIOCGIWSTATS:             /* Get /proc/net/wireless stats */
            //case  SIOCSIWSPY:               /* set spy addresses */
            //case  SIOCGIWSPY:               /* get spy info (quality of link) */
            //case  SIOCSIWTHRSPY:            /* set spy threshold (spy event) */
            //case  SIOCGIWTHRSPY:            /* get spy threshold */
            //case  SIOCSIWAP:                /* set access point MAC addresses */
            case  SIOCGIWAP:                /* get access point MAC addresses */
                {
                    //AP is the first parameter returned -> from here on we start a new WLAN network
                    if ( net.isValid() )
                        entries.append( net );
                    net = WirelessNetwork();
                    memcpy( iwp, value, IW_EV_ADDR_LEN-IW_EV_LCP_LEN );
                    const struct ether_addr* eth = (const struct ether_addr*)(&iwevent.u.ap_addr.sa_data);
                    QString tmp;
                    tmp.sprintf("%02X:%02X:%02X:%02X:%02X:%02X", eth->ether_addr_octet[0],
                            eth->ether_addr_octet[1], eth->ether_addr_octet[2],
                            eth->ether_addr_octet[3], eth->ether_addr_octet[4],
                            eth->ether_addr_octet[5]);
                    net.setData( WirelessNetwork::AP, tmp );
                }
                break;
            //case  SIOCSIWSCAN:              /* trigger scanning (list cells) */
            //case  SIOCGIWSCAN:              /* get scanning results */
            case  SIOCSIWESSID:             /* set ESSID (network name) */
                break;
            case  SIOCGIWESSID:             /* get ESSID */
                {
                    unsigned char * payload;
                    if ( weVersion > 18 ) {
                        //the pointer in iw_point is omitted
                        memcpy( iwp+IW_EV_POINT_OFF, value, sizeof(struct iw_point)-IW_EV_POINT_OFF );
                        payload = value + sizeof(struct iw_point) - IW_EV_POINT_OFF;
                    } else {
                        memcpy( iwp, value, sizeof(struct iw_point) );
                        payload = value + sizeof(struct iw_point);
                    }
                    if ( data + iwevent.len - payload > 0 && iwevent.u.essid.length  )  { //we have some payload
                        QString ssid;
                        if ( iwevent.u.essid.flags ) {
                            char essid[IW_ESSID_MAX_SIZE+1];
                            memcpy( essid, payload, iwevent.u.essid.length );
                            essid[iwevent.u.essid.length] = '\0';
                            ssid = QString(essid);
                            //qLog(Network) << "Discovered network on" << iface << ": " << ssid;
                        } else {
                            ssid = tr("off");
                        }
                        net.setData( WirelessNetwork::ESSID, ssid );
                    } else {
                        qLog(Network) << "ESSID event detected but no payload available";
                    }
                }
                break;
            //case  SIOCSIWNICKN:             /* set node name/nickname */
            //case  SIOCGIWNICKN:             /* get node name/nickname */
            case  SIOCSIWRATE:              /* set default bit rate (bps) */
                break;
            case  SIOCGIWRATE:              /* get default bit rate (bps) */
                memcpy( iwp, value, IW_EV_PARAM_LEN-IW_EV_LCP_LEN );
                net.setData( WirelessNetwork::BitRate, QString::number( iwevent.u.bitrate.value ) );
                break;
            //case  SIOCSIWRTS:               /* set RTS/CTS threshold (bytes) */
            //case  SIOCGIWRTS:               /* get RTS/CTS threshold (bytes) */
            //case  SIOCSIWFRAG:              /* set fragmentation thr (bytes) */
            //case  SIOCGIWFRAG:              /* get fragmentation thr (bytes) */
            //case  SIOCSIWTXPOW:             /* set transmit power (dBm) */
            //case  SIOCGIWTXPOW:             /* get transmit power (dBm) */
            //case  SIOCSIWRETRY:             /* set retry limits and lifetime */
            //case  SIOCGIWRETRY:             /* get retry limits and lifetime */
            case  SIOCSIWENCODE:            /* set encoding token & mode */
                break;
            case  SIOCGIWENCODE:            /* get encoding token & mode */
                {
                    unsigned char * payload;
                    if ( weVersion > 18 ) {
                        //the pointer in iw_point is omitted
                        memcpy( iwp+IW_EV_POINT_OFF, value, sizeof(struct iw_point)-IW_EV_POINT_OFF );
                        payload = value + sizeof(struct iw_point) - IW_EV_POINT_OFF;
                    } else {
                        memcpy( iwp, value, sizeof(struct iw_point) );
                        payload = value + sizeof(struct iw_point);
                    }
                    //evaluate security mode
                    QString tmp;
                    if ( (iwevent.u.data.flags & IW_ENCODE_OPEN) || (iwevent.u.data.flags & IW_ENCODE_RESTRICTED) ) {
                        if ( iwevent.u.data.flags & IW_ENCODE_OPEN )
                            tmp = tr("open", "open security");
                        else
                            tmp = tr("restricted", "restricted security");
                        net.setData( WirelessNetwork::Security, tmp );
                    }
                    if ( iwevent.u.data.flags & IW_ENCODE_DISABLED ) {
                        net.setData( WirelessNetwork::Encoding, tr("off") );
                    } else {
                        if ( iwevent.u.data.flags & IW_ENCODE_NOKEY )
                            net.setData( WirelessNetwork::Encoding, tr("on") );
                        else if ( data + iwevent.len - payload > 0 && iwevent.u.data.length  ) { //we have some payload
                            //TODO we have a key that we could show
                            //r.data = tr("whatever");
                        }
                    }
                }
                break;
            //case  SIOCSIWPOWER:             /* set Power Management settings */
            //case  SIOCGIWPOWER:             /* get Power Management settings */

            // WPA ioctls
            //case  SIOCSIWGENIE:             /* set generic IE */
            //case  SIOCGIWGENIE:             /* get generic IE */
            //case  SIOCSIWMLME:              /* request MLME operation */
            //case  SIOCSIWAUTH:              /* set authentication mode params */
            //case  SIOCGIWAUTH:              /* get authentication mode params */
            //case  SIOCSIWENCODEEXT:         /* set encoding token & mode */
            //case  SIOCGIWENCODEEXT:         /* get encoding token & mode */
            //case  SIOCSIWPMKSA:             /* PMKSA cache operation */

            //wireless events
            //case  IWEVTXDROP:
            case  IWEVQUAL:
                //we need range info WE version >15 to support quality parameter
                if ( weVersion > 15 ) {
                    memcpy( iwp, value, IW_EV_QUAL_LEN-IW_EV_LCP_LEN );
                    struct iw_quality* qual = &iwevent.u.qual;

                    //the quality field is 8 bit integer
                    //we need range to work out whether we have dbm or percent value
                    //by comparing quality.qual with range->max_qual.value
                    //Percent -> use iw_quality.qual as signed integer ( iw_quality.qual < range->max_qual.value )
                    //dbm -> use iw_quality.qual as negative integer ( iw_quality.qual > range->max_qual.value )
                    if ( qual->level <= range->max_qual.level ){
                        if ( !(qual->updated & IW_QUAL_QUAL_INVALID) ) {
                            int range_max = qMax( (int)range->max_qual.qual, 255 );
                            net.setData( WirelessNetwork::Quality,
                                    QString().setNum( ((double) qual->qual)/range_max, 'f', 2 ) );
                        }
                        //we have relative signal level
                        if ( !(qual->updated & IW_QUAL_LEVEL_INVALID) ) {
                            int range_max = qMax( (int)range->max_qual.level, 255 );
                            net.setData( WirelessNetwork::Signal,
                                    QString().setNum( ((double)qual->level) / range_max ) );
                        }
                        if ( !(qual->updated & IW_QUAL_NOISE_INVALID) ) {
                            int range_max = qMax( (int)range->max_qual.noise, 255 );
                            net.setData( WirelessNetwork::Noise,
                                    QString().setNum( ((double)qual->noise) / range_max, 'f', 2));
                        }
                    } else {
                        //we have dbm signal level
                        if ( !(qual->updated & IW_QUAL_QUAL_INVALID) ) {
                            int range_max = qMax( (int)range->max_qual.qual, 255 );
                            net.setData( WirelessNetwork::Quality,
                                    QString().setNum( ((double) qual->qual - 0x100)/range_max, 'f', 2 ) );
                        }
                        if ( !(qual->updated & IW_QUAL_LEVEL_INVALID) ) {
                            int range_max = qMax( (int)range->max_qual.level, 255 );
                            net.setData( WirelessNetwork::Signal,
                                    QString().setNum( ( (double)qual->level - 0x100)/range_max + 1, 'f', 2 ) );
                            //level.data = QString::number(qual->level - 0x100) + QChar(' ') + tr("dBm", "unit for signal strength");
                        }
                        if ( !(qual->updated & IW_QUAL_NOISE_INVALID) ) {
                            int range_max = qMax( (int)range->max_qual.noise, 255 );
                            net.setData( WirelessNetwork::Noise,
                                    QString().setNum( ( (double)qual->noise - 0x100)/range_max +1, 'f', 2 ) );
                            //noise.data = QString::number(qual->noise - 0x100) + QChar(' ') + tr("dBm", "unit for signal strength");
                        }
                    }
                }
                break;
#if WIRELESS_EXT > 14
            case  IWEVCUSTOM:
                {
                    unsigned char * payload;
                    if ( weVersion > 18 ) {
                        //the pointer in iw_point is omitted
                        memcpy( iwp+IW_EV_POINT_OFF, value, sizeof(struct iw_point)-IW_EV_POINT_OFF );
                        payload = value + sizeof(struct iw_point) - IW_EV_POINT_OFF;
                    } else {
                        memcpy( iwp, value, sizeof(struct iw_point) );
                        payload = value + sizeof(struct iw_point);
                    }

                    char buf[IW_CUSTOM_MAX+1];
                    if ( iwevent.u.data.length ) {
                        memcpy( buf, payload, iwevent.u.data.length );
                        buf[ iwevent.u.data.length ] = '\0';
                        net.addCustomData( QString( buf ).trimmed() );
                    }

                }
                break;
#endif
            //case  IWEVREGISTERED:
            //case  IWEVEXPIRED:
            //case  IWEVGENIE:
            //case  IWEVMICHAELMICFAILURE:
            //case  IWEVASSOCREQIE:
            //case  IWEVASSOCRESPIE:
            //case  IWEVPMKIDCAND:
            default:
                qLog(Network) << "Unknown iw_event type" << iwevent.cmd;
            }

        data += iwevent.len;
    }
    if ( net.isValid() )
        entries.append( net );

    if ( qLogEnabled(Network) ) {
    foreach(WirelessNetwork n, entries )
        qLog(Network) << "#### Found" << n.data(WirelessNetwork::ESSID) << n.data(WirelessNetwork::AP) << n.data(WirelessNetwork::Security).toString();
    }
#endif
}


static const char* itemDescription[] = {
    QT_TRANSLATE_NOOP( "WSearchPage", "Protocol" ), //WRecord::Protocol
    QT_TRANSLATE_NOOP( "WSearchPage", "Access point" ), //WRecord::AP
    QT_TRANSLATE_NOOP( "WSearchPage", "ESSID" ), //WRecord::ESSID
    QT_TRANSLATE_NOOP( "WSearchPage", "Mode" ), //WRecord::Mode
    QT_TRANSLATE_NOOP( "WSearchPage", "Network ID" ), //WRecord::NWID
    QT_TRANSLATE_NOOP( "WSearchPage", "Bit rate" ), //WRecord::BitRate
    QT_TRANSLATE_NOOP( "WSearchPage", "Frequency" ), //WRecord::Frequency
    QT_TRANSLATE_NOOP( "WSearchPage", "Channel" ), //WRecord::Channel
    QT_TRANSLATE_NOOP( "WSearchPage", "Encryption" ), //WRecord::Encoding
    QT_TRANSLATE_NOOP( "WSearchPage", "Security" ), //WRecord::Security
    QT_TRANSLATE_NOOP( "WSearchPage", "Quality" ), //WRecord::Quality
    QT_TRANSLATE_NOOP( "WSearchPage", "Signal level" ), //WRecord::Signal
    QT_TRANSLATE_NOOP( "WSearchPage", "Noise level" ), //WRecord::Noise
    QT_TRANSLATE_NOOP( "WSearchPage", "More info" )  //WRecord::Custom
};

class ChooseNetworkUI : public QDialog {
    Q_OBJECT
public:
    ChooseNetworkUI( QWidget* parent = 0, Qt::WFlags flags = 0 );
    virtual ~ChooseNetworkUI();

    void setScanData( const QList<WirelessNetwork>& list);
    WirelessNetwork selectedWlan() const;

private:
    void init();

private slots:
    void wlanSelected();
    void updateView();

private:
    QListWidget* list;
    QList<WirelessNetwork> nets;

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QAction* filterHidden;
#else
    QCheckBox* filterHidden;
#endif
};

/**************************************************************************************/

static const int MacAddressRole = Qt::UserRole;
static const int OnlineStateRole = Qt::UserRole + 1;
static const int BitRateRole = Qt::UserRole + 2;
static const int ESSIDRole = Qt::UserRole + 3;
static const int ChannelRole = Qt::UserRole + 4;
static const int ModeRole = Qt::UserRole + 5;
static const int EncryptionRole = Qt::UserRole + 6;
static const int EncryptKeyLengthRole = Qt::UserRole + 7;
static const int EncryptPassphraseRole = Qt::UserRole + 8;
static const int EncryptKeyRole = Qt::UserRole + 9;
static const int SelectedEncryptKeyRole = Qt::UserRole + 10;
static const int NickNameRole = Qt::UserRole + 11;

WSearchPage::WSearchPage( const QString& c, QWidget* parent, Qt::WFlags flags )
    : QWidget( parent, flags ), config( c ), scanEngine( 0 ), state( QtopiaNetworkInterface::Unknown ),
        isShiftMode( false ), isRestart( false )
{
    initUI();
    loadKnownNetworks();

    devSpace = new QNetworkDevice( c, this );
    attachToInterface( devSpace->interfaceName() );
    state = devSpace->state();
    stateChanged( state, false );
    connect( devSpace, SIGNAL(stateChanged(QtopiaNetworkInterface::Status,bool)),
           this, SLOT(stateChanged(QtopiaNetworkInterface::Status,bool)) );

    setObjectName( "wireless-scan");
}

WSearchPage::~WSearchPage()
{
}

/*!
  \internal

  Initialise user interface.
  */
void WSearchPage::initUI()
{
    QVBoxLayout* vbox = new QVBoxLayout( this );
    vbox->setMargin( 2 );
    vbox->setSpacing( 2 );

    currentNetwork = new QLabel( this );
    currentNetwork->setWordWrap( true );
    currentNetwork->setTextFormat( Qt::RichText );
    currentNetwork->setText( tr("Connection state:\n<center><b>not connected</b></center>") );
    vbox->addWidget( currentNetwork );

    QFrame *seperator = new QFrame( this );
    seperator->setFrameShape( QFrame::HLine );
    vbox->addWidget( seperator );

    descr = new QLabel( tr("Network priority:"), this );
    descr->setWordWrap( true );
    vbox->addWidget( descr );

    knownNetworks = new QListWidget( this );
    knownNetworks->setAlternatingRowColors( true );
    knownNetworks->setSelectionBehavior( QAbstractItemView::SelectRows );
    knownNetworks->setEditTriggers( QAbstractItemView::NoEditTriggers );
    connect( knownNetworks, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateActions(QListWidgetItem*,QListWidgetItem*)) );
    connect( knownNetworks, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(changePriority(QListWidgetItem*)) );
    vbox->addWidget( knownNetworks );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu* menu = QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled ( this, true );
    scanAction = new QAction( QIcon(":icon/Network/lan/WLAN-demand"), tr("Rescan"), this ) ;
    scanAction->setVisible( false );
    menu->addAction( scanAction );

    environmentAction = new QAction( QIcon(":icon/new"), tr("Add new networks..."), this );
    menu->addAction( environmentAction );
    connect( environmentAction, SIGNAL(triggered()), this, SLOT(showAllNetworks()) );

    connectAction = new QAction( QIcon(":icon/Network/lan/WLAN-online"), tr("Connect"), this );
    menu->addAction( connectAction );
    connectAction->setVisible( false );
    connect( connectAction, SIGNAL(triggered()), this, SLOT(connectToNetwork()) );

    deleteAction = new QAction( QIcon(":icon/trash"), tr("Delete"), this );
    menu->addAction( deleteAction );
    deleteAction->setVisible( false );
    connect( deleteAction, SIGNAL(triggered()), this, SLOT(deleteNetwork()) );
#else
    //TODO PDA UI
    QHBoxLayout* hbox = new QHBoxLayout;
    vbox->addLayout( hbox );

    refreshPB = new QPushButton;
    refreshPB->setText( tr("Refresh") );
    hbox->addWidget( refreshPB );

    connectPB = new QPushButton;
    connectPB->setText( tr("Connect" ) );
    hbox->addWidget( connectPB );
    connect( connectPB, SIGNAL(clicked(bool)), this, SLOT(connectToNetwork()) );
#endif
}


/*!
  \internal

  Populates the "know networks" list by loading the details from config file
  */
void WSearchPage::loadKnownNetworks()
{
    knownNetworks->clear();
    QSettings cfg( config, QSettings::IniFormat );
    int size = cfg.beginReadArray( "WirelessNetworks" );

    if ( size <= 0 ) {
        QListWidgetItem* item = new QListWidgetItem( tr("<No known networks>") );
        item->setData( MacAddressRole, "INVALID" );
        item->setTextAlignment( Qt::AlignCenter );
        knownNetworks->addItem( item );
        knownNetworks->setSelectionMode( QAbstractItemView::NoSelection );
        cfg.endArray();
        return;
    }

    knownNetworks->setSelectionMode( QAbstractItemView::SingleSelection );

    QListWidgetItem* item;
    QString speed;
    QString essid;
    for ( int i= 0; i < size; i++ ) {
        cfg.setArrayIndex( i );
        QString essid = cfg.value("ESSID").toString();
        QString speed = cfg.value("BitRate").toString();
        QString itemText = essid;
        if ( !speed.isEmpty() && speed != "0" )
            itemText += " ("+speed+" "+ tr("Mb/s" , "Megabit per seconds")+")" ;
        item = new QListWidgetItem( itemText, knownNetworks );
        item->setData( MacAddressRole, cfg.value("AccessPoint").toString() );
        item->setIcon( QIcon(":icon/Network/lan/WLAN-notavail") );
        item->setData( OnlineStateRole, false );
        item->setData( BitRateRole, speed );
        item->setData( ESSIDRole, essid );
        item->setData( ChannelRole, cfg.value("CHANNEL", 0 ).toInt() );
        item->setData( EncryptionRole, cfg.value("Encryption", QString("open") ).toString() );
        item->setData( EncryptKeyLengthRole, cfg.value("KeyLength", 128 ).toInt() );
        item->setData( EncryptPassphraseRole, cfg.value("PRIV_GENSTR").toString() );
        QString key1 = cfg.value( "WirelessKey_1" ).toString();
        QString key2 = cfg.value( "WirelessKey_2" ).toString();
        QString key3 = cfg.value( "WirelessKey_3" ).toString();
        QString key4 = cfg.value( "WirelessKey_4" ).toString();
        item->setData( EncryptKeyRole, QString("%1@%2@%3@%4").arg(key1).arg(key2).arg(key3).arg(key4) );
        item->setData( SelectedEncryptKeyRole, cfg.value("SelectedKey", "PP").toString() );
        item->setData( NickNameRole, cfg.value("Nickname").toString() );
    }

    cfg.endArray();
}


void WSearchPage::stateChanged(QtopiaNetworkInterface::Status newState, bool /*error*/)
{
    if ( !scanEngine )
        return;
    if ( state != newState 
            && state == QtopiaNetworkInterface::Down 
            && (newState == QtopiaNetworkInterface::Demand || newState == QtopiaNetworkInterface::Pending
                || newState == QtopiaNetworkInterface::Up) )
    {
        QtopiaNetwork::extendInterfaceLifetime( config, true );
    }
    
    state = newState;
    switch (newState) {
        case QtopiaNetworkInterface::Pending:
        case QtopiaNetworkInterface::Demand:
            //don't do anything
            break;
        case QtopiaNetworkInterface::Up:
            {
                if ( scanEngine ) {
                    const QString mac = scanEngine->currentAccessPoint();
                    QString essid;
                    QListWidgetItem* lwi = 0;
                    for (int i = 0; i<knownNetworks->count() && essid.isEmpty(); i++) {
                        QListWidgetItem* item = knownNetworks->item( i );
                        if ( !item )
                            return;
                        if ( item->data( MacAddressRole ).toString() == mac ) {
                            essid = item->data( ESSIDRole ).toString();
                            lwi = item;
                        }
                    }

                    //this could return <hidden> which is not very useful -> hence we do it after camparing MACs
                    const QString curEssid = scanEngine->currentESSID();
                    if ( essid.isEmpty() ) {
                        essid = curEssid;
                    } else if ( curEssid != essid ) {
                        //same macs but different essids
                        //->we have a hidden essid but we are connected to it hence we know the real essid
                        essid = curEssid;
                        lwi->setData( ESSIDRole, essid );
                    }
                    //mask < and > because we are about the show it in richtext
                    essid.replace( QString("<"), QString("&lt;") );
                    essid.replace( QString(">"), QString("&gt;") );

                    currentNetwork->setText( QString(
                                    tr("Connection state:\n<center>Connected to <b>%1</b></center>", "1=network name") )
                            .arg( essid ));
                    updateActions( knownNetworks->currentItem(), 0 ); //update all actions
                }
            }
            break;
        case QtopiaNetworkInterface::Down:
            if ( isRestart ) {
                //we could have a reconnection attempt to a new network
                //if that's the case attempt to connect;
                isRestart = false;
                connectToNetwork();
            }
            break;
        default:
            currentNetwork->setText( tr("Connection state:\n<center><b>not connected</b></center>") );
            return;
    }
}

void WSearchPage::saveScanResults()
{
    saveKnownNetworks();
}

/*!
  Saves all scan results to config file.
  */
void WSearchPage::saveKnownNetworks() {
    QSettings cfg( config, QSettings::IniFormat );
    const QVariant timeout = cfg.value( "WirelessNetworks/Timeout" );
    const QVariant autoConnect = cfg.value( "WirelessNetworks/AutoConnect" );
    cfg.beginGroup( "WirelessNetworks" );
    cfg.remove(""); //delete all "known networks" information
    cfg.endGroup();

    if ( knownNetworks->count() ) {
        QListWidgetItem* item;
        cfg.beginWriteArray( "WirelessNetworks" );
        for ( int i = 0; i<knownNetworks->count(); i++ ) {
            item = knownNetworks->item( i );
            if (!item)
                continue;
            QString mac = item->data( MacAddressRole ).toString();
            if ( mac == "INVALID" ) //don't save the "no known network" item
                continue;
            cfg.setArrayIndex( i );
            cfg.setValue( "AccessPoint", mac );
            cfg.setValue( "ESSID" , item->data( ESSIDRole ).toString() );
            cfg.setValue( "BitRate", item->data( BitRateRole ).toString() );
            int channel = 0;
            QVariant v = item->data( ChannelRole );
            if ( v.isValid() )
                channel = v.toInt();
            cfg.setValue( "CHANNEL", channel );
            v = item->data( ModeRole );
            if ( v.isValid() )
                cfg.setValue("WirelessMode", v.toString());
            else
                cfg.setValue("WirelessMode", "Managed");

            v = item->data( EncryptionRole );
            if ( v.isValid() )
                cfg.setValue("Encryption", item->data( EncryptionRole ).toString() );
            else
                cfg.setValue("Encryption", "open" );

            v = item->data( EncryptKeyLengthRole );
            if ( v.isValid() )
                cfg.setValue("KeyLength", v.toInt() );
            else
                cfg.setValue("KeyLength", 128 );

            cfg.setValue("PRIV_GENSTR", item->data( EncryptPassphraseRole ).toString() );

            v = item->data( SelectedEncryptKeyRole );
            if ( v.isValid() )
                cfg.setValue( "SelectedKey", v.toString() );
            else
                cfg.setValue( "SelectedKey", "PP" ); //use passphrase as default

            v = item->data( EncryptKeyRole );
            QStringList keys;
            if ( v.isValid() )
                keys = item->data( EncryptKeyRole ).toString().split( QChar('@'), QString::KeepEmptyParts );
            else
                keys << "" << "" << "" << ""; //add 4 empty keys
            for (int j = keys.count() -1; j >= 0; j-- )
                cfg.setValue("WirelessKey_"+QString::number(j+1), keys[j] );

            cfg.setValue("Nickname", item->data( NickNameRole ).toString() );
        }
        cfg.endArray();
    }
    cfg.setValue( "Info/WriteToSystem", true );
    cfg.setValue( "WirelessNetworks/AutoConnect", autoConnect );
    cfg.setValue( "WirelessNetworks/Timeout", timeout );
    cfg.sync();

}

/*!
  The search for networks should be performed on \ifaceName.
  */
void WSearchPage::attachToInterface( const QString& ifaceName )
{
    if ( !scanEngine ) {
        qLog(Network) << "Using network scanner on interface" << ifaceName;
        scanEngine = new WirelessScan( ifaceName, this );
        connect( scanEngine, SIGNAL(scanningFinished()), this, SLOT(updateConnectivity()) );
#ifdef QTOPIA_KEYPAD_NAVIGATION
        connect( scanAction, SIGNAL(triggered()), scanEngine, SLOT(startScanning()) );
#else
        connect( refreshPB, SIGNAL(clicked(bool)), scanEngine, SLOT(startScanning()) );
#endif

        //do we support network scanning?
        struct iw_range range;
        int weVersion = 0;
        scanEngine->rangeInfo( &range, &weVersion);
        qLog(Network) << "Wireless extension version" << weVersion << "detected";
        if ( weVersion >= 14 ) { //WE v14 introduced SIOCSIWSCAN and friends
#ifdef QTOPIA_KEYPAD_NAVIGATION
            scanAction->setVisible( true );
#else
            connectPB->setEnabled( true );
            refreshPB->setEnabled( true );
#endif
            QTimer::singleShot( 1, scanEngine, SLOT(startScanning()) );
        } else {
#ifdef QTOPIA_KEYPAD_NAVIGATION
            scanAction->setVisible( false );
#else
            connectPB->setEnabled( false );
            refreshPB->setEnabled( false );
#endif
        }
    }
}

/*!
  \internal

  Update actions and/or pushbuttons.
  */
void WSearchPage::updateActions(QListWidgetItem* cur, QListWidgetItem* /*prev*/)
{
    if ( isShiftMode )
        return;
    //update all actions/buttons
#ifdef QTOPIA_KEYPAD_NAVIGATION
    deleteAction->setVisible( cur );
#endif

    if ( !cur || !scanEngine)
        return;

    const bool isOnline = cur->data(OnlineStateRole).toBool();

    QString curAP = scanEngine->currentAccessPoint();
    bool connectEnabled = isOnline && !(curAP == cur->data( MacAddressRole ).toString()) ;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    connectAction->setVisible( connectEnabled );
#else
    //TODO
#endif
}

/*!
  \internal

  Attempts to connect to the currently selected network in the "known network" list.
  */
void WSearchPage::connectToNetwork()
{
    if ( !scanEngine )
        return;
    QListWidgetItem* item = knownNetworks->currentItem();
    if ( !item )
        return;

    saveKnownNetworks();
    QString newESSID = item->data(ESSIDRole).toString();
    if ( newESSID.isEmpty() )
        return;

    switch (state) {
        case QtopiaNetworkInterface::Down:
            break;
        case QtopiaNetworkInterface::Pending:
        case QtopiaNetworkInterface::Demand:
        case QtopiaNetworkInterface::Up:
            {
                if ( item->data( MacAddressRole ).toString() == scanEngine->currentAccessPoint() ) {
                    qLog(Network) << "Already connected to" << item->data( ESSIDRole );
                    return;
                }
                QString currentESSID = scanEngine->currentESSID();
                qLog(Network) << "Connecting from" << currentESSID << "to"
                    << newESSID;

                //force shutdown. This means the app that loads this plugin
                //needs SXE netsetup privileges
                QtopiaNetwork::privilegedInterfaceStop( config );
                //stopping will take some time. we have to wait till the plugin
                //signals no connection;
                isRestart = true;
                return;
            }
            break;
        default:
            return;
    }

    qLog(Network) << "Connecting to" << newESSID;
    currentNetwork->setText( QString(
                    tr("Connection state:\n<center>Connecting to <b>%1</b></center>", "1=network name") )
            .arg( item->data( ESSIDRole ).toString() ));

    QtopiaNetwork::startInterface( config, QVariant( newESSID ) );
}

/*!
  \internal

  Deletes the selected network.
  */
void WSearchPage::deleteNetwork()
{
    int row = knownNetworks->currentRow();
    if ( row < 0 || row >= knownNetworks->count() )
        return;

    QListWidgetItem* item = knownNetworks->takeItem( row );
    delete item;
}


QString qualityToImage( const QString& quality, bool secure )
{
    bool ok = false;
    float qual = quality.toFloat( &ok );
    if ( !ok )
        return QString();
    if ( qual > 0.75f )
        if ( secure )
            return QString(":image/Network/lan/wlan-excellent-secure");
        else
            return QString(":image/Network/lan/wlan-excellent");
    else if ( qual > 0.4f )
        if ( secure )
            return QString(":image/Network/lan/wlan-avg-secure");
        else
            return QString(":image/Network/lan/wlan-avg");
    else
        if ( secure )
            return QString(":image/Network/lan/wlan-bad-secure");
        else
            return QString(":image/Network/lan/wlan-bad");
    return QString();
}

/*!
  \internal

  This function is a helper function for updateConnectivity(). It updates the information
  of a single item in the known list.

  If \a itemToUpdate is null it is assumed that we want to add a new entry.
  */
void WSearchPage::updateKnownNetworkList( const WirelessNetwork& record, QListWidgetItem* itemToUpdate )
{
    if ( !record.isValid() )
        return;
    QListWidgetItem* item = itemToUpdate;
    if ( !item )
        item = new QListWidgetItem( knownNetworks );
    else
        item->setText("");

    QString enc = record.data(WirelessNetwork::Encoding).toString();
    bool securedNet = false;
    if ( enc !=  WirelessScan::tr("off") )
        securedNet = true;

    const QVariant tmp = record.data( WirelessNetwork::Quality );
    if ( tmp.isValid() )
        item->setIcon( QIcon(qualityToImage( tmp.toString(), securedNet )) );
    else
        item->setIcon( QIcon(qualityToImage( record.data(WirelessNetwork::Signal).toString(), securedNet )) );

    bool ok;
    int rate = record.data(WirelessNetwork::BitRate).toString().toInt( &ok );
    if ( ok ) {
        item->setData( BitRateRole, QString::number(rate/1e6) );
    }

    //don't override existing essid
    //if the essid is hidden we don't want to override the name set by the user
    if ( item->data( ESSIDRole ).toString().isEmpty() )
        item->setData( ESSIDRole, record.data(WirelessNetwork::ESSID) );

    item->setData( MacAddressRole, record.data(WirelessNetwork::AP) );
    if ( record.data(WirelessNetwork::Mode).toString() == "Ad-hoc" )
        item->setData( ModeRole, "Ad-hoc" );
    else
        item->setData( ModeRole, "Managed" );

    item->setData( OnlineStateRole, true );
    item->setText( item->data(ESSIDRole).toString() + " (" +
            item->data(BitRateRole).toString() + " " +
            tr("Mb/s" , "Megabit per seconds")+")" );
    knownNetworks->setSelectionMode( QAbstractItemView::SingleSelection );
}

/*!
  \internal

  This function initiates/shows the dialog which presents all WLAN networks
  in the local area. It can be used to add new networks to the "Known" list.
  */
void WSearchPage::showAllNetworks()
{
    if ( !scanEngine )
        return;

    QList<WirelessNetwork> list = scanEngine->results();

    ChooseNetworkUI dlg( this );
    dlg.setScanData( list );
    dlg.showMaximized();
    if ( QtopiaApplication::execDialog( &dlg ) ) {
        WirelessNetwork net = dlg.selectedWlan();
        if ( !net.isValid() )
            return;

        const QString selectedMac = net.data( WirelessNetwork::AP).toString();
        const QString selectedEssid = net.data( WirelessNetwork::ESSID).toString();

        //delete "no known network item"
        if ( knownNetworks->count() == 1 &&
                knownNetworks->item(0) && knownNetworks->item(0)->data( MacAddressRole ).toString() == "INVALID" ) {
            knownNetworks->clear();
        }

        //select current item if it's among known Networks already (matching MAC and ESSID)
        const bool hiddenEssid = (selectedEssid == "<hidden>");
        int matching = -1;
        for (int i = 0; i<knownNetworks->count(); i++) {
            QListWidgetItem* item = knownNetworks->item( i );
            if ( !item )
                continue;
            if ( !hiddenEssid && item->data(ESSIDRole).toString() == selectedEssid ) {
                if ( matching < 0 )
                    matching = i;
                if ( item->data(MacAddressRole).toString() == selectedMac ) { //exact match
                    knownNetworks->setCurrentItem( item );
                    return;
                }
            } else if ( !hiddenEssid ) {
                continue;
            } else if ( item->data(MacAddressRole).toString() == selectedMac ) {
                //we have to deal with a hidden essid -> just compare MACs
                matching = i;
            }
        }
        if ( matching >= 0 ) { //we had at least one network with the same essid
            knownNetworks->setCurrentItem( knownNetworks->item( matching ) );
            return;
        }

        //the selected network is not in our list yet
        updateKnownNetworkList( net );

        int row = knownNetworks->count() - 1;
        if ( row >= 0 )
            knownNetworks->setCurrentRow( row );
    }
}

/*!
  \internal

  Updates the state of the known networks and displays the network we are currently connected to.
  It uses updateKnownNetworkList to update each individual item.
  */
void WSearchPage::updateConnectivity()
{
    if ( !scanEngine )
        return;

    QList<WirelessNetwork> results = scanEngine->results();
    QStringList foundMacs;
    QStringList foundEssids;
    foreach ( WirelessNetwork net, results ) {
        foundMacs.append( net.data( WirelessNetwork::AP ).toString() );
        foundEssids.append( net.data( WirelessNetwork::ESSID ).toString() );
    }

    // network is a match if
    // a) essid is not hidden and mac and essid match or
    // b) essid is not hidden and essid matches or
    // c) essid is hidden and mac matches

    for ( int i = 0; i< knownNetworks->count(); ++i ) {
        QListWidgetItem* item = knownNetworks->item( i );
        if ( !item )
            continue;
        const QString macAddress = item->data(MacAddressRole).toString();
        if ( macAddress == "INVALID" )  //the empty list place holder
            continue;
        const QString essid = item->data(ESSIDRole).toString();

        int idx = -1;
        if ( (idx = foundEssids.indexOf( essid ))>=0  && essid != "<hidden>") {
            updateKnownNetworkList( results.at(idx), item );
        } else if ( (idx=foundMacs.indexOf(macAddress)) >= 0 ) {
            if ( foundEssids.at(idx) == "<hidden>" )
                updateKnownNetworkList( results.at(idx), item );
        } else {
            item->setIcon( QIcon(":icon/Network/lan/WLAN-notavail") );
            item->setData( OnlineStateRole, false );
        }
    }

    updateActions( knownNetworks->currentItem(), 0 ); //update all actions
}

void WSearchPage::changePriority( QListWidgetItem* item )
{
    if ( !item  || knownNetworks->count()<=1 )
        return;

    static QListWidgetItem* marker = 0;
    if ( !isShiftMode ) {
        marker = item;
        descr->setText( tr("Moving %1").arg(marker->text()) );
    } else if ( marker ) {
        int oldRow = knownNetworks->row( marker );
        int newRow = knownNetworks->row( item );
        if ( oldRow > newRow ) {
            knownNetworks->takeItem( oldRow );
            knownNetworks->insertItem( newRow, marker );
        } else if ( oldRow < newRow ) {
            knownNetworks->takeItem( oldRow );
            knownNetworks->insertItem( newRow+1, marker );
        }
        knownNetworks->setCurrentItem( marker );
        descr->setText( tr("Network priority:") );
    }

    if ( marker ) {
        QFont f = marker->font();
        f.setBold( !isShiftMode );
        marker->setFont( f );
    }

#ifdef QTOPIA_KEYPAD_NAVIGATION
    connectAction->setVisible( isShiftMode );
    scanAction->setVisible( isShiftMode );
    environmentAction->setVisible( isShiftMode );
    deleteAction->setVisible( isShiftMode );
#else
    connectPB->setEnabled( isShiftMode );
    refreshPB->setEnabled( isShiftMode );
#endif


    if ( isShiftMode ) {
        marker = 0;
        updateActions( knownNetworks->currentItem(), 0 );
    }
    isShiftMode = !isShiftMode;
}
/**************************************************************************************/

ChooseNetworkUI::ChooseNetworkUI( QWidget* parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    setModal( true );
    init();
}

ChooseNetworkUI::~ChooseNetworkUI()
{
}

void ChooseNetworkUI::init()
{
    setWindowTitle( tr("New WLAN's") );

    QVBoxLayout* vbox = new QVBoxLayout( this );
    vbox->setMargin( 2 );
    vbox->setSpacing( 2 );

    QLabel* label = new QLabel( tr("The following networks were detected in the local area:"), this );
    label->setWordWrap( true );
    vbox->addWidget( label );

    list = new QListWidget( this );
    list->setAlternatingRowColors( true );
    vbox->addWidget( list );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu* menu = QSoftMenuBar::menuFor( this );

    filterHidden = new QAction( tr("Show hidden networks"), this );
    filterHidden->setCheckable( true );
    filterHidden->setChecked( false );
    menu->addAction( filterHidden );
    connect( filterHidden, SIGNAL(toggled(bool)), this, SLOT(updateView()) );
#else
    filterHidden = new QCheckBox( tr("Show hidden networks"), this );
    filterHidden->setCheckState( Qt::Unchecked );
    vBox->addWidget( filterHidden );
    connect( filterHidden, SIGNAL(toggled(bool)), this, SLOT(updateView()) );
#endif

    connect( list, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(wlanSelected()) );
}

WirelessNetwork ChooseNetworkUI::selectedWlan() const
{
    if ( !list->count() || !list->currentItem() )
        return WirelessNetwork();

    const QString mac = list->currentItem()->data( MacAddressRole ).toString();
    const QString essid = list->currentItem()->data( ESSIDRole ).toString();
    for( int i=0; i<nets.count(); ++i) {
        if ( nets[i].data(WirelessNetwork::AP).toString() == mac
            && nets[i].data( WirelessNetwork::ESSID).toString() == essid )
            return nets[i];
    }

    return WirelessNetwork();
}

void ChooseNetworkUI::setScanData( const QList<WirelessNetwork>& records)
{
    nets = records;
    updateView();
}

void ChooseNetworkUI::updateView()
{
    list->clear();

#ifdef QTOPIA_KEYPAD_NAVIGATION
    const bool showHidden = filterHidden->isChecked();
#else
    const bool showHidden = (filterHidden->checkState() == Qt::Checked);
#endif

    QListWidgetItem* item;
    if ( !nets.count() )
    {
        item = new QListWidgetItem( list );
        list->setSelectionMode( QAbstractItemView::NoSelection );
        item->setText( tr("<No WLAN found>") );
        item->setTextAlignment( Qt::AlignCenter );
        return;
    }

    QVariant tmp;
    QString essid;
    QHash<QString,int> essidExist;
    foreach( WirelessNetwork net, nets ) {
        essid = net.data(WirelessNetwork::ESSID).toString();

        if ( !showHidden && essid == "<hidden>" )
            continue;

        if ( essid != "<hidden>" ) {
            if ( essidExist[essid] < 1 )
                essidExist[essid]++;
            else
                continue; //don't show several APs with same essid
        }

        item = new QListWidgetItem( list );
        item->setData( ESSIDRole, essid );

        tmp = net.data(WirelessNetwork::Encoding).toString();
        bool securedNet = false;
        if ( tmp.toString() !=  WirelessScan::tr("off") )
            securedNet = true;

        tmp = net.data( WirelessNetwork::Quality );
        if ( tmp.isValid() )
            item->setIcon( QIcon(qualityToImage( tmp.toString(), securedNet )) );
        else
            item->setIcon( QIcon(qualityToImage( net.data(WirelessNetwork::Signal).toString(), securedNet )) );

        bool ok;
        int rate = net.data(WirelessNetwork::BitRate).toString().toInt( &ok );
        if ( ok ) {
            essid += QLatin1String("    ");
            essid += QString::number(rate/1e6) + QLatin1String(" ") + tr("Mb/s" , "Megabit per seconds");
        }
        item->setText( essid );
        item->setData( MacAddressRole, net.data(WirelessNetwork::AP) );

    }

    list->sortItems();
}

void ChooseNetworkUI::wlanSelected()
{
    QDialog::done(1);
}
#include "wirelessscan.moc"

#endif
