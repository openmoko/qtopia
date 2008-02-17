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

#include "configure.h"
#include "deviceupdater.h"
#include "configuredata.h"

#include <QMutex>
#include <QDir>
#include <QFile>
#include <QFileDialog>

Configure::Configure( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
{
    setupUi( this );
    mData = new ConfigureData();
    mData->load();
    serverNameLineEdit->setText( mData->server() );
    portLineEdit->setText( QString::number( mData->port() ));
    packageRootLineEdit->setText( mData->docRoot() );
    commandLineEdit->setText( mData->command() );

    connect( descriptorPushButton, SIGNAL(clicked()),
            this, SLOT(fileDialog()) );
    connect( packageRootDirPushButton, SIGNAL(clicked()),
            this, SLOT(fileDialog()) );
    connect( serverNameLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(textChanged(QString)) );
    connect( portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(textChanged(QString)) );
    connect( packageRootLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(textChanged(QString)) );
    connect( commandLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(textChanged(QString)) );
}

Configure::~Configure()
{
    mData->save();
    delete mData;
}

/*!
  Singleton instance / factory to create and return the one instance of the
  configure dialog.
*/
Configure *Configure::dialog( QWidget *parent, Qt::WFlags f )
{
    static Configure *theDialog = 0;
    static QMutex m;

    m.lock();
    if ( theDialog == 0 )
        theDialog = new Configure( parent, f );
    m.unlock();
    return theDialog;
}

void Configure::fileDialog()
{
    QString startDir = QDir::rootPath();
    if ( sender() == descriptorPushButton )
    {
        // choose the command for creating the descriptors
        QString command = commandLineEdit->text().trimmed();
        while ( command.endsWith( QDir::separator() ))
            command.chop( 1 );
        if ( !command.isEmpty() && QFile::exists( command ))
            startDir = command;
        QString fileName = QFileDialog::getOpenFileName( this, tr("Select command"), startDir );
        commandLineEdit->setText( fileName );
    }
    else
    {
        // choose the directory for the packages to be served from
        QString docRoot = packageRootLineEdit->text().trimmed();
        while ( docRoot.endsWith( QDir::separator() ))
            docRoot.chop( 1 );
        if ( !docRoot.isEmpty() && QDir( docRoot ).exists() )
            startDir = docRoot;
        QString dirName = QFileDialog::getExistingDirectory( this, tr("Select package serve directory"), startDir );
        packageRootLineEdit->setText( dirName );
    }
}

void Configure::textChanged( const QString &text )
{
    if ( sender() == serverNameLineEdit &&
            serverNameLineEdit->text().trimmed() != mData->server() )
    {
        mData->setServer( serverNameLineEdit->text().trimmed() );
    }
    else if ( sender() == portLineEdit )
    {
        bool ok;
        quint16 val = portLineEdit->text().toInt( &ok );
        if ( ok && val != mData->port() )
            mData->setPort( val );
    }
    else if ( sender() == packageRootLineEdit &&
            packageRootLineEdit->text().trimmed() != mData->docRoot() )
    {
        mData->setDocRoot( packageRootLineEdit->text().trimmed() );
    }
    else if ( sender() == commandLineEdit &&
            commandLineEdit->text().trimmed() != mData->command() )
    {
        mData->setCommand( commandLineEdit->text().trimmed() );
    }
}
