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

#ifndef INSTALL_CONTROL_H
#define INSTALL_CONTROL_H

#include <QString>
#include <QDir>

class InstallControl
{
public:

    enum InstallStatus
    {
        Available,        // Available for install, but neither installed nor partly installed
        PartlyInstalled,  // Part-way thru install process, some - but not all - files have been written to store
        Installed,        // The install process has completed (but may still be in error)
        Error    = 0x08   // The install process has resulted in an error
    };
    struct PackageInfo
    {
        PackageInfo() {}
        ~PackageInfo() {}
        PackageInfo( const PackageInfo& d ) { *this = d; }
        InstallStatus status;
        QString name;
        QString description;
        QString fullDescription;
        QString size;
        QString section;
        QString domain;
        QString packageFile;
        QString trust;
        QString md5Sum;
        QStringList files;
        QString version;
        bool isComplete() const
        {
            return !(name.isEmpty() ||
                description.isEmpty() || size.isEmpty() ||
                (domain.isEmpty() && section.isEmpty()) ||
                packageFile.isEmpty() || md5Sum.isEmpty());
        }
        bool isSystemPackage() const
        {
            return !trust.isEmpty() && trust != "Untrusted";
        }
        PackageInfo &operator= ( const PackageInfo &d );
        bool operator< ( const PackageInfo &d ) const;
        bool operator==( const PackageInfo &d ) const;
    };

    InstallControl();
    ~InstallControl();

    void installPackage( const PackageInfo& ) const;
    bool verifyPackage( const QString &, const PackageInfo & ) const;
    void setInstallMedia( const QString &s ) { m_installMedia = s; }
    QString installMedia() const { return m_installMedia; }

    bool verifyCertificate( const QString & ) const;

private:

    void registerPackageFiles( const QDir &, const QString &, const QString & ) const;

    int m_numberInstalledPackages;
    QString m_installMedia;
};

/*!
  Return true if this package name is collated before the other, or for packages with
  the same name, return true if this package is an earlier version number than the other

  These version numbers could have all sorts of strange values in them:

  "V1.0.3"  "version 5.3" "1.0" and so on.

  If either has an empty version number, return false

  Otherwise extract a sequence of digits from each and turn into a number, then
  compare.

  If numbers are different, return true/false as appropriate.  If same number,
  extract and compare next numbers in majority order.

  Make at most 3 such comparisons.

  Sequences of digits are broken up by any non-digit character, typically
  a "." (decimal point).  If comparing eg "3.1.6" to "3.1" the second number is padded
  with "0" in the missing majority positions, eg "3.1" is compared as "3.1.0".
*/
inline bool InstallControl::PackageInfo::operator< ( const PackageInfo &other ) const
{
    // for different packages do a name collation
    if ( name != other.name )
        return ( name < other.name );
    int thisV = 0, otherV = 0, thisD, otherD;
    int thisPtr = 0, otherPtr = 0;
    int majority = 3;
    while ( majority > 0 )
    {
        while ( thisPtr < version.count() && ( thisD = version[thisPtr++].digitValue() ) != -1 )
            thisV = thisV * 10 + thisD;
        while ( otherPtr < other.version.count() && ( otherD = other.version[otherPtr++].digitValue() ) != -1 )
            otherV = otherV * 10 + otherD;
        if ( thisV != otherV )
            return ( thisV < otherV );
        thisV = otherV = 0;
        majority--;
    }
    return false;
}

/*!
  Return true is this is the same package as the other.

  It is the same package, if the name is the same and the version is the same.

  A simplifying assumption is made, that the same package provider will not issue
  a package where the version number is meant to reflect the same version, but uses
  a different format - ie  "version 1.0.3" and "V 1.0.3".  This seems like a safe
  assumption.
*/
inline bool InstallControl::PackageInfo::operator==( const PackageInfo &d ) const
{
    return ( name == d.name && version == d.version );
}

inline InstallControl::PackageInfo &InstallControl::PackageInfo::operator=( const InstallControl::PackageInfo &d )
{
    name = d.name;
    description = d.description;
    fullDescription = d.fullDescription;
    size = d.size;
    section = d.section;
    domain = d.domain;
    trust = d.trust;
    packageFile = d.packageFile;
    md5Sum = d.md5Sum;
    version = d.version;
    return *this;
}

#endif
