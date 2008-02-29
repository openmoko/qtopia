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

#include <qtopiasendvia.h>
#include <qcontent.h>
#include <qmimetype.h>
#include <qtopiaservices.h>

#include <QMap>
#include <QDialog>
#include <QVBoxLayout>
#include <qtopiaapplication.h>

#include <QListWidget>
#include <QDesktopWidget>

#include <QDSServices>
#include <QDSServiceInfo>
#include <QDSData>
#include <QDSServiceInfo>
#include <QDSAction>

#if defined(QTOPIA_INFRARED)
#include <qvaluespace.h>
#endif

class QtopiaSendFileDialog : public QDialog
{
    Q_OBJECT

public:
    QtopiaSendFileDialog(QWidget *parent,
                         const QString &filename,
                         const QString &mimetype,
                         bool autodelete);
    ~QtopiaSendFileDialog();

protected slots:
    void serverClicked( QListWidgetItem* );

private:
    QString m_filename;
    QString m_mimetype;
    bool m_autodelete;
};

QtopiaSendFileDialog::QtopiaSendFileDialog(QWidget *parent,
                                           const QString &filename,
                                           const QString &mimetype,
                                           bool autodelete)
    : QDialog(parent)
{
    m_filename = filename;
    m_mimetype = mimetype;
    m_autodelete = autodelete;

    setModal(true);
    setWindowTitle( tr ( "Send via..."));
    QVBoxLayout* l = new QVBoxLayout( this );
    QListWidget* lb = new QListWidget( this );
    l->addWidget( lb );

    connect( lb,
                SIGNAL( itemActivated( QListWidgetItem* ) ),
                this,
                SLOT( serverClicked( QListWidgetItem* ) ) );

#ifdef QTOPIA_BLUETOOTH
    {
        QListWidgetItem *item = new QListWidgetItem( tr("Bluetooth"), lb );
        item->setIcon( QPixmap(":icon/bluetooth/bluetooth-online") );
        item->setData( Qt::UserRole, QByteArray("Bluetooth") );
    }
#endif

#ifdef QTOPIA_BLUETOOTH
    {
        QListWidgetItem *item = new QListWidgetItem( tr("Infrared"), lb );
        item->setIcon( QPixmap(":icon/beam") );
        item->setData( Qt::UserRole, QByteArray("Infrared") );
    }
#endif

    lb->sortItems();
    lb->setCurrentRow(0);
}

QtopiaSendFileDialog::~QtopiaSendFileDialog()
{

}

void QtopiaSendFileDialog::serverClicked(QListWidgetItem *item)
{
    if (item->data( Qt::UserRole ).toByteArray() == "Bluetooth") {
        QtopiaServiceRequest req("BluetoothPush", "pushFile(QString,QString,bool)");
        req << m_filename << m_mimetype << m_autodelete;
        req.send();
    }

    else if (item->data( Qt::UserRole ).toByteArray()  == "Infrared") {
        QtopiaServiceRequest req("InfraredBeaming", "beamFile(QString,QString,bool)");
        req << m_filename << m_mimetype << m_autodelete;
        req.send();
    }

    accept();
}

class QtopiaSendViaDialog : public QDialog
{
    Q_OBJECT

public:
    QtopiaSendViaDialog( const QByteArray &data,
                         const QString &mimetype, QWidget* parent );
    ~QtopiaSendViaDialog();

protected slots:
    void serverClicked( QListWidgetItem* );

private:
    QDSData m_data;
    QMap< QListWidgetItem*, QDSServiceInfo > mItemToService;
};

QtopiaSendViaDialog::QtopiaSendViaDialog(const QByteArray &data,
                                         const QString &mimetype,
                                         QWidget *parent) : QDialog(parent)
{
    QDSServices services(mimetype,
                         QString( "" ),
                         QStringList( QString ( "send" ) ) );

    m_data = QDSData( data, QMimeType( mimetype ) );

    if ( services.count() > 0) {
        setModal(true);
        setWindowTitle( tr ( "Send via..."));
        QVBoxLayout* l = new QVBoxLayout( this );
        QListWidget* lb = new QListWidget( this );
        l->addWidget( lb );

        connect( lb,
                 SIGNAL( itemActivated( QListWidgetItem* ) ),
                 this,
                 SLOT( serverClicked( QListWidgetItem* ) ) );

        foreach ( QDSServiceInfo serviceinfo, services ) {
            QListWidgetItem* item
                    = new QListWidgetItem( serviceinfo.description(), lb );
            item->setIcon( QPixmap( ":icon/" + serviceinfo.icon() ) );
            mItemToService.insert( item, serviceinfo );
        }

        lb->sortItems();
        lb->setCurrentRow(0);
    }
}

QtopiaSendViaDialog::~QtopiaSendViaDialog()
{

}

void QtopiaSendViaDialog::serverClicked(QListWidgetItem *item)
{
    // Create QDS action and make request
    QDSAction action( mItemToService[item] );
    action.invoke(m_data);
    accept();
}

QtopiaSendVia::QtopiaSendVia()
{
}

QtopiaSendVia::~QtopiaSendVia()
{
}

bool QtopiaSendVia::isDataSupported(const QString &mimetype)
{
    QDSServices services(mimetype,
                    QString( "" ),
                    QStringList( QString ( "send" ) ) );

    return services.count() > 0;
}

bool QtopiaSendVia::isFileSupported()
{
#if defined(QTOPIA_BLUETOOTH)
    return true;
#endif

#if defined(QTOPIA_INFRARED)
    QValueSpaceItem item("/Communications/Infrared/");
    bool ret = item.value("Available").toBool();

    return ret;
#endif
    return false;
}

bool QtopiaSendVia::sendData(QWidget *parent, const QByteArray &data, const QString &mimetype)
{
    QtopiaSendViaDialog *dlg = new QtopiaSendViaDialog(data, mimetype, parent);

# ifdef QTOPIA_PHONE
    QtopiaApplication::setMenuLike( dlg, true );
# endif
    QtopiaApplication::execDialog( dlg );
    delete dlg;

    return true;
}

bool QtopiaSendVia::sendFile(QWidget *parent, const QString &filename, const QString &mimetype,
                             bool autodelete)
{
#if defined(QTOPIA_BLUETOOTH) || defined(QTOPIA_INFRARED)
    QtopiaSendFileDialog *dlg = new QtopiaSendFileDialog(parent, filename, mimetype, autodelete);

#ifdef QTOPIA_PHONE
    QtopiaApplication::setMenuLike( dlg, true );
#endif
    QtopiaApplication::execDialog( dlg );
    delete dlg;
#else
    Q_UNUSED(parent);
    Q_UNUSED(filename);
    Q_UNUSED(mimetype);
    Q_UNUSED(autodelete);
#endif

    return true;
}

bool QtopiaSendVia::sendFile(QWidget *parent, const QContent &content, bool autodelete)
{
    QMimeType mime(content);
    return sendFile(parent, content.file(), mime.id(), autodelete);
}

#include "qtopiasendvia.moc"
