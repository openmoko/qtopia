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

#include "printserver.h"

#include <qprinterinterface.h>
#include <qtopiaapplication.h>
#include <QDebug>
#include <QCopChannel>
#include <QPluginManager>
#include <QPrintEngine>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMap>
#include <QMessageBox>
#include <QQueue>

class PrintServerPrivate {
public:
    PrintServerPrivate() {
        m_pluginManager = new QPluginManager( "qtopiaprinting" );
        m_idle = true;
    }
    ~PrintServerPrivate() {
        delete m_pluginManager;
        m_pluginManager = 0;
    }

    QPluginManager *m_pluginManager;
    QQueue<PrintJobInfo> m_spooler;
    bool m_idle;
};

/*!
    \class PrintServer
    \brief The class PrintServer receives print jobs from applications and distributes the jobs to printer plugins.
    PrintServer displays available printing mechanisms to the user and pass the print job to a plugin that handles printing.
*/
PrintServer::PrintServer(QObject *parent)
    :QObject(parent)
{
    QtopiaApplication::instance()->registerRunningTask( "PrintServer", this );

    d = new PrintServerPrivate();

    // listen to the application channel messages
    connect( qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
            this, SLOT(receive(const QString&,const QByteArray&)) );

    // load printer plugins
    d->m_pluginManager = new QPluginManager( "qtopiaprinting" );

    // Print service
    new PrintService( this );
}

PrintServer::~PrintServer()
{
    delete d;
}

/*!
    \internal
    Receive and queue print job
*/
void PrintServer::receive(const QString &message, const QByteArray &data)
{
    QDataStream stream(data);
    if ( message == "print(QVariant)" ) {
        QVariant variant;
        stream >> variant;
        enqueuePPKPrintJob( variant );
    } else if ( message == "print(QString)" ) {
        QString fileName;
        stream >> fileName;
        enqueueFilePrintJob( fileName );
    } else if ( message == "Print::print(QString,QString)" ) {
        QString fileName, mimeType;
        stream >> fileName;
        stream >> mimeType;
        enqueueFilePrintJob( fileName, mimeType );
    } else if ( message == "printHtml(QString)" ) {
        QString html;
        stream >> html;
        enqueueHtmlPrintJob( html );
    } else if ( message == "done(bool)" ) {
        bool error;
        stream >> error;
        done( error );
    }
}

/*!
    \internal
    Createas and queues a print job and dispatchs it if printer is idle.
    \a properties contains QPrintEngine::PrintEnginePropertyKeys and associated values.
    To retreive values parse \a properties as following.

    \code
    QMap<QString,QVariant> PPK_values = properties.toMap();
    QString outputFileName =
        PPK_values.value( QString::number( QPrintEngine::PPK_OutputFileName ) ).toString();
    \endcode

*/
void PrintServer::enqueuePPKPrintJob(const QVariant &properties)
{
    // create printjob
    PrintJobInfo jobInfo;
    jobInfo.type = PPK;
    jobInfo.properties = properties;

    d->m_spooler.enqueue(jobInfo);

    if ( d->m_idle )
        selectPrinterPlugin();
}

/*!
    \internal
    Creates and queues a print job and dispatchs it if printer is idle.
    Properties of print job includes only \a fileName and marked as File type print job.
    Properties should be parsed accordingly when dispatched.
*/
void PrintServer::enqueueFilePrintJob(const QString &fileName)
{
    // create printJob
    QMap<QString,QVariant> properties;
    properties.insert("fileName", QVariant( fileName ));

    PrintJobInfo jobInfo;
    jobInfo.type = File;
    jobInfo.properties = QVariant( properties );

    d->m_spooler.enqueue(jobInfo);

    if ( d->m_idle )
        selectPrinterPlugin();
}

/*!
    \internal
    Creates and queues a print job and dispatchs it if printer is idle.
    Properties of print job includes only \a fileName and \a mimeType.
    This print job also marked as File type print job.
    Properties should be parsed accordingly when dispatched.
*/
void PrintServer::enqueueFilePrintJob(const QString &fileName, const QString &mimeType)
{
    // create printJob
    QMap<QString,QVariant> properties;
    properties.insert("fileName", QVariant( fileName ));
    properties.insert("mimeType", QVariant( mimeType ));

    PrintJobInfo jobInfo;
    jobInfo.type = File;
    jobInfo.properties = QVariant( properties );

    d->m_spooler.enqueue(jobInfo);

    if ( d->m_idle )
        selectPrinterPlugin();
}

/*!
    \internal
    Creates and queues a print job and dispatchs it if printer is idle.
    Properties of print job includes only \a html and marked as Html type print job.
    Properties should be parsed accordingly when dispatched.
*/
void PrintServer::enqueueHtmlPrintJob(const QString &html)
{
    // create printJob
    PrintJobInfo jobInfo;
    jobInfo.type = Html;
    jobInfo.properties = QVariant( html );

    d->m_spooler.enqueue(jobInfo);

    if ( d->m_idle )
        selectPrinterPlugin();
}



/*!
    \internal
    Load plugins
*/
void PrintServer::selectPrinterPlugin()
{
    QDialog dlg;
    dlg.setWindowTitle( tr( "Printer Type" ) );
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QtopiaApplication::setMenuLike( &dlg, true );
#endif
    QVBoxLayout layout( &dlg );
    QListWidget list( &dlg );
    layout.addWidget( &list );
    connect( &list, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(pluginSelected(QListWidgetItem*)) );
    connect( &list, SIGNAL(itemActivated(QListWidgetItem*)),
            &dlg, SLOT(accept()) );

    QStringList pluginList = d->m_pluginManager->list();
    pluginList.sort();

    QObject *instance;
    QtopiaPrinterInterface *iface;

    if ( pluginList.count() > 0 ) {
        foreach ( QString plugin, pluginList ) {
            instance = d->m_pluginManager->instance( plugin );
            iface = qobject_cast<QtopiaPrinterPlugin *>(instance);
            if ( iface->isAvailable() ) {
                QListWidgetItem *item = new QListWidgetItem( iface->name(), &list );
                item->setData( Qt::WhatsThisRole, plugin );
            }
        }
        list.setCurrentRow( 0 );
    }

    if ( list.count() == 0 ) {
        (void) new QListWidgetItem( tr( "Printing is not available" ), &list );
        list.setCurrentRow( 0 );
        QtopiaApplication::instance()->unregisterRunningTask( this );
    }

    QtopiaApplication::execDialog( &dlg );
}

void PrintServer::pluginSelected( QListWidgetItem *item )
{
    QString pluginName = item->data( Qt::WhatsThisRole ).toString();
    QObject *instance = d->m_pluginManager->instance( pluginName );
    if ( instance )
        dispatchPrintJob( qobject_cast<QtopiaPrinterPlugin *>(instance) );
}

/*!
    \internal
    Distribute the job to the selected printing machanism.
*/
void PrintServer::dispatchPrintJob( QtopiaPrinterInterface *plugin )
{
    if ( !plugin->isAvailable() )
        return;

    PrintJobInfo jobInfo = d->m_spooler.head();
    QMap<QString, QVariant> properties = jobInfo.properties.toMap();

    QString fileName, mimeType, html;

    // send print job to printer plugin
    // properties of print job should be parsed accordingly based on job type.
    switch ( jobInfo.type ) {
    case PPK:
        d->m_idle = false;
        plugin->print( properties );
        break;
    case File:
        fileName = properties.value( "fileName" ).toString();
        mimeType = properties.value( "mimeType", QVariant(QString()) ).toString();
        d->m_idle = false;
        plugin->printFile( fileName, mimeType );
        break;
    case Html:
        html = jobInfo.properties.toString();
        d->m_idle = false;
        plugin->printHtml( html );
        break;
    default:
        break;
    }
}

void PrintServer::done( bool error )
{
    if ( !error ) {
        // remove print job from spooler
        d->m_spooler.dequeue();
        d->m_idle = true;
    } else {
        int result = QMessageBox::question( 0, tr( "Printing" ),
                        tr( "Printing failed. Try again?" ),
                        QMessageBox::Yes, QMessageBox::No );
        if ( result == (int) QMessageBox::Yes ) {
            selectPrinterPlugin();
        } else {
            d->m_spooler.dequeue();
            d->m_idle = true;
        }
    }
    if ( d->m_spooler.count() )
        selectPrinterPlugin();
    else
        QtopiaApplication::instance()->unregisterRunningTask( this );

}

/*!
    \service PrintService PrintServer
    \brief The class PrintService provides the Qtopia Printing service.
*/
PrintService::~PrintService()
{
}

/*!
    Prints a file specified by \a fileName.
    This slot corresponds to the QCop message
    \c{Print::print(QString)}.
*/
void PrintService::print(QString fileName)
{
    parent->enqueueFilePrintJob( fileName );
}

/*!
    Prints a file specified by \a fileName.
    \a mimeType is optional.
    This slot corresponds to the QCop message
    \c{Print::print(QString,QString)}.
*/
void PrintService::printFile(QString fileName, QString mimeType)
{
    parent->enqueueFilePrintJob( fileName, mimeType );
}

/*!
    Prints html document specified by \a html.
    This slot corresponds to the QCop message
    \c{Print::printHtml(QString)}.
*/
void PrintService::printHtml(QString html)
{
    parent->enqueueHtmlPrintJob( html );
}

