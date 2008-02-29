/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef INSTALL_CONTROL_H
#define INSTALL_CONTROL_H

#include <QString>
#include <QDir>

class ErrorReporter
{
public:
    virtual ~ErrorReporter(){}
    void reportError( const QString &simpleError, const QString &detailedError = QString() ) 
    {
        doReportError( simpleError, detailedError );
    };
private:
    virtual void doReportError( const QString &simpleError, const QString &detailedError ) = 0;
};

class SimpleErrorReporter:public ErrorReporter
{
public:
    enum ReporterType{ Install, Uninstall, Other };
    SimpleErrorReporter( ReporterType type, const QString &pkgName = "" ); 
private:
    virtual void doReportError( const QString &simpleError, const QString &detailedError );
    QString packageName;   
    QString prefix;
    QString subject; 
};

class InstallControl
{
public:

    struct PackageInfo
    {
        enum Source
        {
            Control, //package information obtained from control file
            PkgList  //package information obtained from packages list or qpd descriptor
        };

        PackageInfo() { isEnabled = true; }
        ~PackageInfo() {}
        PackageInfo( const PackageInfo& d ) { *this = d; }
        QString name;
        QString description;
        QString fullDescription;
        QString size; //size of package to download
        QString section;
        QString domain;
        QString packageFile;
        QString trust;
        QString md5Sum;
        QStringList files;//this member is deprecated
        QString version;
        QString qtopiaVersion;
        QString url;
        QString devices;
        QString installedSize; //size after decompression
        QString type;
        bool isEnabled;
        bool isComplete( Source source = PackageInfo::PkgList, QString *reason = 0) const;
        bool isSystemPackage() const
        {
            return false;
        }
        PackageInfo &operator= ( const PackageInfo &d );
        bool operator< ( const PackageInfo &d ) const;
        bool operator==( const PackageInfo &d ) const;
    };

    InstallControl();
    ~InstallControl();

    bool installPackage( const PackageInfo&, const QString &md5Sum, ErrorReporter *reporter = 0 ) const;
    void uninstallPackage( const PackageInfo &, ErrorReporter *reporter = 0 ) const;
    bool verifyPackage( const QString &, const PackageInfo &, ErrorReporter *reporter = 0 ) const;
    void setInstallMedia( const QString &s ) { m_installMedia = s; }
    QString installMedia() const { return m_installMedia; }

    bool verifyCertificate( const QString & ) const;
    static QString downloadedFileLoc();

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
        return ( name.toLower() < other.name.toLower() );
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
  Return true if this is the same package as the other.

 The packages are considered the same if they either have the same
 md5sum or the same name
*/
inline bool InstallControl::PackageInfo::operator==( const PackageInfo &d ) const
{
      return ( md5Sum == d.md5Sum ) || ( name == d.name );
}

inline InstallControl::PackageInfo &InstallControl::PackageInfo::operator=( const InstallControl::PackageInfo &d )
{
    name = d.name;
    description = d.description;
    fullDescription = d.fullDescription;
    size = d.size;
    installedSize = d.installedSize;
    section = d.section;
    domain = d.domain;
    trust = d.trust;
    packageFile = d.packageFile;
    md5Sum = d.md5Sum;
    version = d.version;
    url = d.url;
    qtopiaVersion = d.qtopiaVersion;
    devices = d.devices;
    isEnabled = d.isEnabled;
    files = d.files; //files is deprecated
    installedSize = d.installedSize;
    type = d.type;
    return *this;
}

#endif
