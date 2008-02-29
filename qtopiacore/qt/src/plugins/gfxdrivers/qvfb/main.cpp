/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qscreendriverplugin_qws.h>
#include <qscreenvfb_qws.h>
#include <qstringlist.h>

class ScreenVfbDriver : public QScreenDriverPlugin
{
public:
    ScreenVfbDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

ScreenVfbDriver::ScreenVfbDriver()
: QScreenDriverPlugin()
{
}

QStringList ScreenVfbDriver::keys() const
{
    QStringList list;
    list << "QVFb";
    return list;
}

QScreen* ScreenVfbDriver::create(const QString& driver, int displayId)
{
    if (driver.toLower() == "qvfb")
        return new QVFbScreen(displayId);

    return 0;
}

Q_EXPORT_STATIC_PLUGIN(ScreenVfbDriver)
Q_EXPORT_PLUGIN2(qscreenvfb, ScreenVfbDriver)
