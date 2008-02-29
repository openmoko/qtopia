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

#ifndef VERSION_H
#define VERSION_H

#include <qstring.h>

#include <custom-qtopia.h>
#ifndef QTOPIA_COMPATIBLE_DEVICES
#warning  QTOPIA_COMPATIBLE_DEVICES not defined in device specific custom.h \
           setting value to "Unknown" 
#define QTOPIA_COMPATIBLE_DEVICES "Unknown"
#endif


class Version
{
    public:
        Version( const QString & );

        int maj() const{ return m_version / MAJOR_MAG; };
        int min() const{ return (m_version % MAJOR_MAG) / MINOR_MAG; };
        int patch() const{ return (m_version % MINOR_MAG); };

        //TODO: methods to modify maj, min and patch

        bool operator< ( const Version &other ) const;
        bool operator== ( const Version &other ) const;
        bool operator<= ( const Version &other ) const;
        bool operator > ( const Version &other ) const;
        bool operator >= ( const Version &other ) const;
        QString toString() const;

    private:
        enum { NUM_VERSION_PARTS = 3 };
        static const long MAJOR_MAG;
        static const long MINOR_MAG;
        static const long PATCH_MAG;
        long m_version;
        bool bothValid( const Version &other ) const;
};

class Single;
class Range;
class VersionItem
{
    public:
        VersionItem( const QString &str);
        virtual ~VersionItem();
        virtual bool isCompatibleWith( const VersionItem &verItem ) const = 0;
        virtual bool isCompatibleWith( const Single &other ) const = 0;
        virtual bool isCompatibleWith( const Range &other ) const = 0;
        virtual QString toString() const = 0;

        static VersionItem *createVersionItem( const QString &str );
        static QList<VersionItem *> createVersionItemList ( const QString &verList );
};

class Single: public VersionItem
{
    public:
        Single( const QString &str );
        Version ver() const { return m_version; };
        bool isCompatibleWith( const VersionItem &other ) const;
        bool isCompatibleWith( const Single &other ) const;
        bool isCompatibleWith( const Range &other ) const;
        QString toString() const { return m_version.toString(); };
    private:
        Version m_version;
};

class Range: public VersionItem
{
    public:
        Range( const QString &str );
        Version maxVer() const { return m_max; };
        Version minVer() const { return m_min; };
        bool isCompatibleWith( const VersionItem &other ) const;
        bool isCompatibleWith( const Single &other ) const;
        bool isCompatibleWith( const Range &other ) const;
        QString toString() const { return m_min.toString() + "-" + m_max.toString(); };
    private:
        Version m_min;
        Version m_max;
};

class VersionUtil
{
public:
    static bool checkVersionLists( const QString &verList1, const QString &verList2 );
private:
    static bool checkVersionItems( const QString &verItem1, const QString &verItem2 );
};

class DeviceUtil
{
    public:
        static bool checkDeviceLists( const QString &devList1, const QString &devList2 );
        static QString compatibleDevices();

};

#endif
