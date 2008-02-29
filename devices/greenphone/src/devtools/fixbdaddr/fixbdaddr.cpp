/****************************************************************************
**
** Copyright (C) 2006-2007 TROLLTECH ASA. All rights reserved.
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
#include "fixbdaddr.h"

#include <qtopia/qtopiaapplication.h>
#include <qtopia/qsoftmenubar.h>
#include <qtopiaphone/qtelephonyconfiguration.h>
#include <qtopiabase/qlog.h>

#include <QWaitWidget>
#include <QMessageBox>

#include <QListWidget>
#include <QVBoxLayout>
#include <QMenu>

#include <string.h>
#include "../../../include/docparatable.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const QString option1 = QObject::tr("Set Address (Yuhuatel Dist)");
const QString option2 = QObject::tr("Set Address (IMEI Based, Unsafe)");

/* If you must change this, always APPEND!!! */
const char *imeis[] = {
    "348731009980330",
    "358731009980184",
    "358731009980309",
    "358731009980135",
    "358731009980796",
    "358731009980614",
    "358731009980176",
    "358731009980333",
    "358731009980598",
    "358731009980721",
    "358731009979970",
    "358731009980119",
    "358731009980739",
    "358731009979871",
    "358731009980143",
    "358731009979913",
    "358731009980861",
    "358731009979905",
    "358731009980390",
    "358731009980929",
    "358731009980267",
    "358731009980192",
    "358731009980903",
    "358731009980507",
    "358731009980036",
    "358731009980572",
    "358731009980523",
    "358731009980481",
    "358731009980648",
    "358731009980325",
    "358731009980077",
    "358731009980408",
    "358731009980754",
    "358731009979863",
    "358731009980242",
    "358731009979897",
    "358731009980150",
    "358731009979988",
    "358731009980606",
    "358731009980622",
    "358731009980747",
    "358731009980846",
    "358731009980564",
    "358731009979889",
    "358731009980283",
    "358731009980895",
    "358731009980697",
    "358731009980440",
    "358731009984905",
    NULL
};

int find_imei(const char *imei)
{
    const char **iter;

    iter = imeis;

    while (*iter) {
        if (!strcmp(*iter, imei))
            return iter-imeis;

        iter++;
    }

    return -1;
}

class FixBdaddr_Private
{
public:
    QListWidget *m_list;
    QString m_imei;
    QWaitWidget *m_waitWidget;
};

FixBdaddr::FixBdaddr( QWidget* parent, Qt::WFlags fl )
:   QDialog( parent, fl )
{
    setWindowTitle(tr("Fix BT Address"));

    m_data = new FixBdaddr_Private;
    m_data->m_waitWidget = 0;

    QVBoxLayout* vbLayout = new QVBoxLayout(this);
    vbLayout->setMargin(0);
    vbLayout->setSpacing(0);

    m_data->m_list = new QListWidget(this);

    connect(m_data->m_list, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(accept()));

    QListWidgetItem *item;
    item = new QListWidgetItem(option1, m_data->m_list);
    m_data->m_list->setCurrentItem(item);

    item = new QListWidgetItem(option2, m_data->m_list);

    m_data->m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_data->m_list->setFrameStyle(QFrame::NoFrame);
    vbLayout->addWidget(m_data->m_list);

    QTelephonyConfiguration *config
            = new QTelephonyConfiguration( "modem" );   // No tr
    connect( config, SIGNAL(notification(QString,QString)),
             this, SLOT(configValue(QString,QString)) );
    config->request( "serial" );            // No tr
}

FixBdaddr::~FixBdaddr()
{
    delete m_data;
}

void FixBdaddr::configValue(const QString &key, const QString &value)
{
    if ( key == "serial" ) {   // No tr
        m_data->m_imei = value;
        if (m_data->m_waitWidget)
            m_data->m_waitWidget->hide();
    }
}

int set_bdaddr(const char *value)
{
    char bdaddr[6];
    unsigned int tmp[6];

    sscanf(value, "%02x:%02x:%02x:%02x:%02x:%02x", &tmp[0], &tmp[1], &tmp[2],
           &tmp[3], &tmp[4], &tmp[5]);

    for (int i = 0; i < 6; i++) {
        bdaddr[i] = tmp[i];
    }

    int fd = open("/dev/docparatable", O_RDONLY);
    if (fd < 0) {
        return 1;
    }

    if (ioctl(fd, DOCPTSETBTMACADDR, bdaddr) < 0) {
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}

void FixBdaddr::accept()
{
    if (m_data->m_imei.isEmpty()) {
        m_data->m_waitWidget = new QWaitWidget(this);
        m_data->m_waitWidget->setExpiryTime( 5000 );
        m_data->m_waitWidget->setCancelEnabled(true);
        m_data->m_waitWidget->setText( tr("Obtaining IMEI") );
        m_data->m_waitWidget->show();

        if (m_data->m_imei.isEmpty()) {
            QMessageBox::critical(this, tr("Change Bluetooth Address"),
                                  tr("Could not obtain IMEI, Bluetooth Address was NOT changed."));
            return;
        }
    }

    qLog(Bluetooth) << "Changing Bluetooth Address - IMEI: " << m_data->m_imei;

    if (m_data->m_list->currentItem()->text() == option2) {
        char buf[] = "AC:DE:48:00:00:00";
        int serial = m_data->m_imei.mid(8, 6).toInt();
        char subserial[6];
        snprintf(subserial, 6, "%5X", serial);
        subserial[5] = '\0';

        buf[10] = subserial[0];
        buf[12] = subserial[1];
        buf[13] = subserial[2];
        buf[15] = subserial[3];
        buf[16] = subserial[4];

        qLog(Bluetooth) << "Changing address to: " << buf;

        if (set_bdaddr(buf) != 0) {
            QMessageBox::critical(this, tr("Change Bluetooth Address"),
                                  tr("Could not write new device address."));
            return;
        }

        QMessageBox::information(this, tr("Change Bluetooth Address"),
                                 tr("Bluetooth Address changed to: ") + QString(buf) + ".  " +
                                         tr("Please reboot the device for changes to take effect"));
    }
    else if (m_data->m_list->currentItem()->text() == option1) {
        // Yuhuatel gave us an address block for DVT3 devices of
        // 00:19:65:00:08:01-00:19:65:00:08:64
        // Which is exactly 100 numbers
        // Only the last byte is changing
        char buf[] = "00:19:65:00:08:00";
        int offset = 1;

        int position = find_imei(m_data->m_imei.toLatin1().constData());

        if (position == -1) {
            QMessageBox::critical(this, tr("Change Bluetooth Address"),
                                  tr("Could not find your IMEI in the database.  Please contact support@trolltech.com"));
            return;
        }

        char tmp[3];
        snprintf(tmp, 3, "%2X", offset+position);
        buf[15] = tmp[0];
        buf[16] = tmp[1];

        qLog(Bluetooth) << "Changing address to: " << buf;

        if (set_bdaddr(buf) != 0) {
            QMessageBox::critical(this, tr("Change Bluetooth Address"),
                                  tr("Could not write new device address."));
            return;
        }

        QMessageBox::information(this, tr("Change Bluetooth Address"),
                                 tr("Bluetooth Address changed to: ") + QString(buf) + ".  " +
                                         tr("Please reboot the device for changes to take effect"));
    }

    QDialog::accept();
}
