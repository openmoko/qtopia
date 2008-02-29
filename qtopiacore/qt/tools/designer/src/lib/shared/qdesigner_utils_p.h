/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_UTILS_H
#define QDESIGNER_UTILS_H

#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtGui/QMainWindow>
#include "abstractformwindow.h"

namespace qdesigner_internal {

class EnumType
{
public:
    QVariant value;
    QMap<QString, QVariant> items;
    QStringList names;
};


class FlagType
{
public:
    QVariant value;
    QMap<QString, QVariant> items;
};

} // namespace qdesigner_internal

Q_DECLARE_METATYPE(qdesigner_internal::EnumType)
Q_DECLARE_METATYPE(qdesigner_internal::FlagType)

namespace qdesigner_internal { namespace Utils {

inline int valueOf(const QVariant &value, bool *ok = 0)
{
    if (qVariantCanConvert<EnumType>(value))
        return qVariantValue<EnumType>(value).value.toInt(ok);
    else if (qVariantCanConvert<FlagType>(value))
        return qVariantValue<FlagType>(value).value.toInt(ok);

    return value.toInt(ok);
}

inline bool isObjectAncestorOf(QObject *ancestor, QObject *child)
{
    QObject *obj = child;
    while (obj != 0) {
        if (obj == ancestor)
            return true;
        obj = obj->parent();
    }
    return false;
}

inline bool isCentralWidget(QDesignerFormWindowInterface *fw, QWidget *widget)
{
    if (! fw || ! widget)
        return false;

    if (widget == fw->mainContainer())
        return true;

    // ### generalize for other containers
    if (QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer())) {
        return mw->centralWidget() == widget;
    }

    return false;
}

} // namespace Utils

} // namespace qdesigner_internal

#endif // QDESIGNER_UTILS_H
