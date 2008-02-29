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
#include <qscreenvnc_qws.h>
#include <qstringlist.h>

class GfxVncDriver : public QScreenDriverPlugin
{
public:
    GfxVncDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxVncDriver::GfxVncDriver()
: QScreenDriverPlugin()
{
}

QStringList GfxVncDriver::keys() const
{
    QStringList list;
    list << "VNC";
    return list;
}

QScreen* GfxVncDriver::create(const QString& driver, int displayId)
{
    if (driver.toLower() == "vnc")
        return new QVNCScreen(displayId);

    return 0;
}

Q_EXPORT_STATIC_PLUGIN(GfxVncDriver)
Q_EXPORT_PLUGIN2(qgfxvnc, GfxVncDriver)
