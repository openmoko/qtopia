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

#ifndef SCANNERTHREAD_H
#define SCANNERTHREAD_H

#include <QThread>
#include <QFileInfo>

class PackageScanner;

class ScannerThread : public QThread
{
    Q_OBJECT
public:
    ScannerThread( PackageScanner *parent ) : QThread() { mParent = parent; }
    ~ScannerThread() {}
    void run();
private:
    void handleFile( const QString &, const QFileInfo & );
    PackageScanner *mParent;

    // current idea of how many files we have to scan thru
    static int numberOfFiles;
};

#endif
