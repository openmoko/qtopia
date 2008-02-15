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
#include "localservicesdialog.h"

#include <qbluetoothservicecontroller.h>
#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qsoftmenubar.h>

#include <QMessageBox>
#include <QListView>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QtopiaItemDelegate>
#include <QVBoxLayout>
#include <QMenu>


class ServicesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ServicesModel(QBluetoothServiceController *serviceController, QObject *parent);

    void setSecurityEnabled(const QModelIndex &index, bool enable);
    QString serviceFromIndex(const QModelIndex &index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private slots:
    void serviceStarted(const QString &name, bool error, const QString &desc);
    void serviceStopped(const QString &name);

private:
    QBluetoothServiceController *m_serviceController;
    QStringList m_serviceIds;
    QIcon m_lockIcon;
    QHash<QString, QBluetooth::SecurityOptions> m_cachedSecurityOptions;
};

ServicesModel::ServicesModel(QBluetoothServiceController *serviceController, QObject *parent)
    : QAbstractListModel(parent),
      m_serviceController(serviceController)
{
    connect(m_serviceController, SIGNAL(started(QString,bool,QString)),
            SLOT(serviceStarted(QString,bool,QString)));
    connect(m_serviceController, SIGNAL(stopped(QString)),
            SLOT(serviceStopped(QString)));

    m_serviceIds = m_serviceController->services();
    m_lockIcon = QIcon(":image/icons/padlock");
}

void ServicesModel::setSecurityEnabled(const QModelIndex &index, bool enable)
{
    QString name = m_serviceIds.value(index.row());
    if (name.isEmpty())
        return;

    if (enable) {
        m_serviceController->setSecurityOptions(name,
                QBluetooth::Authenticated | QBluetooth::Encrypted);
        m_cachedSecurityOptions.insert(name,
                QBluetooth::Authenticated | QBluetooth::Encrypted);
    } else {
        m_serviceController->setSecurityOptions(name, 0);
        m_cachedSecurityOptions.insert(name, 0);
    }

    emit dataChanged(index, index);
}

QString ServicesModel::serviceFromIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return QString();
    return m_serviceIds.value(index.row());
}

int ServicesModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_serviceIds.size();
}

int ServicesModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant ServicesModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (!index.isValid() || row < 0 || row >= m_serviceIds.count() )
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            return m_serviceController->displayName(m_serviceIds[row]);
        case Qt::CheckStateRole:
            if (m_serviceController->state(m_serviceIds[row]) ==
                    QBluetoothServiceController::NotRunning) {
                return Qt::Unchecked;
            } else {    // Running / Starting
                return Qt::Checked;
            }
        case Qtopia::AdditionalDecorationRole:
        {
            // check the cache in case the security options have just
            // changed, and the bluetoothservicemanager hasn't got the
            // IPC message yet and thus calling securityOptions() on
            // the controller would still return the old options
            QBluetooth::SecurityOptions options;
            if (m_cachedSecurityOptions.contains(m_serviceIds[row]))
                options = m_cachedSecurityOptions[m_serviceIds[row]];
            else
                options = m_serviceController->securityOptions(m_serviceIds[row]);
            if ( (options & QBluetooth::Authenticated) &&
                    (options & QBluetooth::Encrypted) ) {
                return m_lockIcon;
            }
        }
        default:
            break;
    }

    return QVariant();
}

Qt::ItemFlags ServicesModel::flags(const QModelIndex &index) const
{
    if (m_serviceController->state(m_serviceIds[index.row()]) ==
            QBluetoothServiceController::Starting) {
        return ( Qt::ItemIsSelectable | Qt::ItemIsUserCheckable );
    } else {
        return ( Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
    }
}

void ServicesModel::serviceStarted(const QString &name, bool error, const QString &)
{
    if (!error) {
        int row = m_serviceIds.indexOf(name);
        if (row != -1)
            emit dataChanged(index(row, 0), index(row, columnCount()-1));
    }
}

void ServicesModel::serviceStopped(const QString &name)
{
    int row = m_serviceIds.indexOf(name);
    if (row != -1) {
        emit dataChanged(index(row, 0), index(row, columnCount()-1));
    }
}

//========================================================

/*!
    \class LocalServicesDialog

    This displays a list of local services, and allows the user to perform
    operations enabled registered local services.

    When this dialog starts up, it displays all registered services (i.e. those
    created using QBluetoothAbstractService).
 */

LocalServicesDialog::LocalServicesDialog(QWidget *parent, Qt::WFlags)
    : QDialog(parent),
      m_serviceController(new QBluetoothServiceController(this)),
      m_model(new ServicesModel(m_serviceController, this)),
      m_view(new QListView(this))
{
    connect(m_view, SIGNAL(activated(QModelIndex)),
            SLOT(activated(QModelIndex)));

    m_securityAction = new QAction(tr("Use secure connections"), this);
    m_securityAction->setCheckable(true);
    connect(m_securityAction, SIGNAL(triggered(bool)),
            this, SLOT(toggleSecurity(bool)));
    QSoftMenuBar::menuFor(this)->addAction(m_securityAction);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_view);
    setLayout(layout);

    setWindowTitle(tr("My Services"));
    setObjectName("local-services");
}

LocalServicesDialog::~LocalServicesDialog()
{
}

void LocalServicesDialog::start()
{
    m_view->setModel(m_model);
    m_view->setItemDelegate(new QtopiaItemDelegate(this));
    connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(currentChanged(QModelIndex,QModelIndex)));

    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setAlternatingRowColors(true);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setUniformItemSizes(true);
    m_view->setTextElideMode(Qt::ElideRight);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    if (m_model->rowCount() > 0)
        m_view->setCurrentIndex(m_model->index(0, 0));

    this->setModal( true );
    QtopiaApplication::execDialog( this );
}

void LocalServicesDialog::activated(const QModelIndex &index)
{
    QString name = m_model->serviceFromIndex(index);
    if (name.isEmpty())
        return;
    switch (m_serviceController->state(name)) {
        case QBluetoothServiceController::NotRunning:
            m_lastStartedService = name;
            m_serviceController->start(name);
            break;
        case QBluetoothServiceController::Starting:
            QMessageBox::warning(this, tr("Service Busy"),
                    QString(tr("<P>Service is starting, please wait...")));
            break;
        case QBluetoothServiceController::Running:
            m_serviceController->stop(name);
            break;
    }
}

void LocalServicesDialog::serviceStarted(const QString &name, bool error, const QString &desc)
{
    if (error && name == m_lastStartedService) {
        m_lastStartedService.clear();
        QMessageBox::warning(this, tr("Service Error"),
                QString(tr("<P>Unable to start service:") + QString("\r\n"))
                        + desc);
    }
}

void LocalServicesDialog::toggleSecurity(bool checked)
{
    m_model->setSecurityEnabled(m_view->currentIndex(), checked);
}

void LocalServicesDialog::currentChanged(const QModelIndex &current, const QModelIndex &/*previous*/)
{
    QString name = m_model->serviceFromIndex(current);
    QBluetooth::SecurityOptions security = m_serviceController->securityOptions(name);
    if ( (security & QBluetooth::Authenticated ) &&
         (security & QBluetooth::Encrypted) ) {
        m_securityAction->setChecked(true);
    } else {
        m_securityAction->setChecked(false);
    }
}

#include "localservicesdialog.moc"
