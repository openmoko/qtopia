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
#include <qdwin32>
using namespace QDWIN32;

#include <QString>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>

int main( int /*argc*/, char **argv )
{
    QString mso;
    QString msoutl;

    {   // Use mso.dll and msoutl.olb from src/qtopiadesktop/dist if they exist
        // This is useful when you want to target a version of Outlook older
        // than what you have installed.
        QString dist = QDir::cleanPath( QFileInfo(argv[0]).absoluteDir().absoluteFilePath("../src/qtopiadesktop/dist") );
        QString _mso = QString("%1/mso.dll").arg(dist);
        //qDebug() << "mso" << _mso;
        if ( QFile::exists(_mso) )
            mso = QDir::toNativeSeparators( _mso );
        QString _msoutl = QString("%1/msoutl.olb").arg(dist);
        if ( QFile::exists(_msoutl) )
            msoutl = QDir::toNativeSeparators( _msoutl );
    }

    if ( mso.isEmpty() ) {
        QString clsid = readRegKey( HKEY_CLASSES_ROOT, "OfficeCompatible.Application\\CLSID" );
        //qDebug() << "CLSID" << clsid;
        if ( clsid.isEmpty() )
            return 1;

        mso = readRegKey( HKEY_CLASSES_ROOT, QString("CLSID\\%1\\InprocServer32").arg(clsid) );
        //qDebug() << "mso" << mso;
    }

    if ( msoutl.isEmpty() ) {
        QString clsid = readRegKey( HKEY_CLASSES_ROOT, "Outlook.Application\\CLSID" );
        //qDebug() << "CLSID" << clsid;
        if ( clsid.isEmpty() )
            return 1;

        QString typelib = readRegKey( HKEY_CLASSES_ROOT, QString("CLSID\\%1\\Typelib").arg(clsid) );
        //qDebug() << "typelib" << typelib;
        if ( typelib.isEmpty() )
            return 1;

        QString sub = findSubKey( HKEY_CLASSES_ROOT, QString("Typelib\\%1").arg(typelib) );
        //qDebug() << "sub" << sub;
        if ( sub.isEmpty() )
            return 1;

        msoutl = readRegKey( HKEY_CLASSES_ROOT, QString("Typelib\\%1\\%2\\0\\win32").arg(typelib).arg(sub) );
        //qDebug() << "msoutl" << msoutl;
    }

    if ( mso.isEmpty() )
        qDebug() << "Could not find mso.dll";

    if ( msoutl.isEmpty() )
        qDebug() << "Could not find msoutl.olb";

    if ( mso.isEmpty() || msoutl.isEmpty() ) {
        qDebug() << "Do you have Outlook installed?";
        return 1;
    }

    {
        QFile f( "qoutlook_detect.h" );
        f.open( QIODevice::WriteOnly );
        QTextStream stream( &f );
        stream << "#import \"" << mso << "\" \\" << endl
               << "    no_namespace, no_implementation, rename(\"DocumentProperties\", \"DocProps\")" << endl
               << "#import \"" << msoutl << "\" \\" << endl
               << "    rename_namespace(\"Outlook\")" << endl;
        f.close();
    }

    {
        QFile f( "qoutlook_detect.cpp" );
        f.open( QIODevice::WriteOnly );
        QTextStream stream( &f );
        stream << "#import \"" << mso << "\" \\" << endl
               << "    no_namespace, implementation_only, rename(\"DocumentProperties\", \"DocProps\")" << endl;
               //<< "#import \"" << msoutl << "\" \\" << endl
               //<< "    rename_namespace(\"Outlook\")" << endl;
        f.close();
    }

    return 0;
}

