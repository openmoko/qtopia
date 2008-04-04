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

#ifdef QT_QWS_FICGTA01

#include "fichardware.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>

#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>

#include <qtopiaserverapplication.h>
#include <standarddevicefeatures.h>
#include <ui/standarddialogs.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>

QTOPIA_TASK(Ficgta01Hardware, Ficgta01Hardware);

FicLinuxInputEventHandler::FicLinuxInputEventHandler(QObject* parent)
    : QObject(parent)
    , m_fd(-1)
    , m_notifier(0)
{
}

bool FicLinuxInputEventHandler::openByPhysicalBus(const QByteArray& physical)
{
    return internalOpen(EVIOCGPHYS(4096), 4096, physical);
}

bool FicLinuxInputEventHandler::openByName(const QByteArray& name)
{
    return internalOpen(EVIOCGNAME(4096), 4096, name);
}

bool FicLinuxInputEventHandler::internalOpen(int request, int length, const QByteArray& match)
{
    if (m_fd >= 0) {
        ::close(m_fd);
        delete m_notifier;
        m_notifier = 0;
        m_fd = -1;
    }

    QByteArray deviceData(length, 0);

    // Find a suitable device, might want to add caching
    QDir dir(QLatin1String("/dev/input/"), QLatin1String("event*"));
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files|QDir::System)) {
        m_fd = ::open(QFile::encodeName(fileInfo.filePath()), O_RDONLY|O_NDELAY);
        if (m_fd < 0)
            continue;

        int ret = ioctl(m_fd, request, deviceData.data());
        if (ret < 0)
            continue;

        // match the string we got with what we wanted
        if (strcmp(deviceData.constData(), match.constData()) == 0) {
            break;
        } else {
            close(m_fd);
            m_fd = -1;
        }
    }

    if (m_fd >= 0) {
        m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
        connect(m_notifier, SIGNAL(activated(int)), this, SLOT(readData()));
    }

    return m_fd >= 0;
}

void FicLinuxInputEventHandler::readData()
{
    struct input_event event;

    int n = read(m_fd, &event, sizeof(struct input_event));
    if(n != (int)sizeof(struct input_event))
        return;

    emit inputEvent(event);
}

Ficgta01Hardware::Ficgta01Hardware()
    : m_vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree")
{
    qWarning() << "ficgta01hardware plugin";

    m_vsoPortableHandsfree.setAttribute("Present", false);

    m_handler = new FicLinuxInputEventHandler(this);
    if (m_handler->openByPhysicalBus("neo1973kbd/input0")) {
        connect(m_handler, SIGNAL(inputEvent(struct input_event&)),
                SLOT(inputEvent(struct input_event&)));
    } else {
        qWarning("Cannot open a device for the neo1973kbd");
        delete m_handler;
        m_handler = 0;
    }
}

Ficgta01Hardware::~Ficgta01Hardware()
{
}

void Ficgta01Hardware::inputEvent(struct input_event& event)
{
    if(event.type != EV_SW)
        return;

    qWarning("keypressed: type=%03d, code=%03d, value=%03d (%s)",
              event.type, event.code,event.value,((event.value)!=0) ? "Down":"Up");

    // Only handle the headphone insert. value=0 (Up,Insert), value=1 (Down,Remove)
    switch(event.code) {
    case SW_HEADPHONE_INSERT:
        m_vsoPortableHandsfree.setAttribute("Present", event.value != 0x01);
        m_vsoPortableHandsfree.sync();
        break;
    default:
        break;
    };
}

#endif // QT_QWS_Ficgta01

