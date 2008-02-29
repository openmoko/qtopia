/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qmousedriverplugin_qws.h>
#include <qmousetslib_qws.h>

class TslibMouseDriver : public QMouseDriverPlugin
{
public:
    TslibMouseDriver();

    QStringList keys() const;
    QWSMouseHandler* create(const QString &name);
    QWSMouseHandler* create(const QString &driver, const QString &device);
};

TslibMouseDriver::TslibMouseDriver()
    : QMouseDriverPlugin()
{
}

QStringList TslibMouseDriver::keys() const
{
    return (QStringList() << "tslib");
}

QWSMouseHandler* TslibMouseDriver::create(const QString &name)
{
    return create(name, QString());
}

QWSMouseHandler* TslibMouseDriver::create(const QString &driver,
                                          const QString &device)
{
    if (driver.toLower() != "tslib")
        return 0;

    return new QWSTslibMouseHandler(driver, device);
}

Q_EXPORT_STATIC_PLUGIN(TslibMouseDriver)
Q_EXPORT_PLUGIN2(qwstslibmousehandler, TslibMouseDriver)
