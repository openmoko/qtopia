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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "hybridscreen.h"

#include <QScreenDriverPlugin>
#include <QStringList>

class HybridPlugin : public QScreenDriverPlugin
{
public:
    HybridPlugin();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

HybridPlugin::HybridPlugin()
    : QScreenDriverPlugin()
{
}

QStringList HybridPlugin::keys() const
{
    return (QStringList() << "hybrid");
}

QScreen* HybridPlugin::create(const QString &driver, int displayId)
{
    if (driver.toLower() != "hybrid")
        return 0;

    return new HybridScreen(displayId);
}

Q_EXPORT_STATIC_PLUGIN(Hybrid)
Q_EXPORT_PLUGIN2(hybridscreendriver, HybridPlugin)
