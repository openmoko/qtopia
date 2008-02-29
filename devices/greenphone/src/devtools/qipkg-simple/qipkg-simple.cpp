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

#include <unistd.h>

#include <QTimer>
#include <QDebug>
#include <qcontent.h>

#include "qipkg-simple.h"
#include <qpushbutton.h>

QipkgBase::QipkgBase( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    setupUi( this );
}

QipkgBase::~QipkgBase()
{
}

Qipkg::Qipkg( QWidget *parent, Qt::WFlags f )
    : QipkgBase( parent, f )
{
    TextLabel1->setText("Please wait...");
    done->setEnabled(false);
    showMaximized();
    connect(done, SIGNAL(clicked()), this, SLOT(finished()));
}

Qipkg::~Qipkg()
{
}

void Qipkg::start()
{
    QString cmd,package;

    QContent nf(filename);
    cmd = filename;
    package = "Installing Package ";
    package = package + nf.name();
    TextLabel1->setText(package);

    package = "pkg.sh ";
    package = package + cmd;

    TextLabel1->setText(TextLabel1->text() + "\n\nrunning: " + package);

    int i = system(package.toLocal8Bit().constData());
    if(i == 0) {
        TextLabel1->setText(TextLabel1->text() + "\n\nsuccessfully installed");
    } else {
        TextLabel1->setText(TextLabel1->text() + "\n\ninstallation failed");
    } 
    done->setEnabled(true);
}

void Qipkg::finished()
{ 
    exit(0);
}

void Qipkg::setDocument( const QString& ipkg )
{
    filename = ipkg; 
    start();
}

QipkgService::~QipkgService()
{
}

void QipkgService::setDocument( const QString& ipkg )
{
    parent->setDocument( ipkg );
}
