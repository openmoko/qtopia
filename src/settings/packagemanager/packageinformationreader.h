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

#ifndef PACKAGEINFORMATIONREADER_H
#define PACKAGEINFORMATIONREADER_H

#include <QString>
#include <QTextStream>

#include "installcontrol.h"

class PackageInformationReader : public QObject
{
    Q_OBJECT
public:

    PackageInformationReader(const QString& fileName, InstallControl::PackageInfo::Source source = InstallControl::PackageInfo::PkgList);
    PackageInformationReader(QTextStream &ts, InstallControl::PackageInfo::Source source = InstallControl::PackageInfo::PkgList); 
    PackageInformationReader(InstallControl::PackageInfo::Source source = InstallControl::PackageInfo::PkgList);

    void readLine( const QString & );
    void reset();

    QString name() const { return pkg.name; }
    QString description() const { return pkg.description; }
    QString section() const { return pkg.section; }
    QString domain() const { return pkg.domain; }
    QString trust() const { return pkg.trust; }
    QString url() const { return pkg.url; }
    QString md5Sum() const { return pkg.md5Sum; }
    QString qtopiaVersion() const { return pkg.qtopiaVersion; }
    QString devices() const { return pkg.devices; }
    QString installedSize() const { return pkg.installedSize; }
    QString type() const { return pkg.type; }


    const InstallControl::PackageInfo &package() const { return pkg; }
    bool getIsError() const { return isError; }
    QString getError() const { return error; }

signals:
    void packageComplete();

private:
    QString error;
    bool isError;
    bool hasContent;

    bool accumulatingFullDesc;
    InstallControl::PackageInfo pkg;
    InstallControl::PackageInfo::Source source;

    void checkCompleted();
};

#endif
