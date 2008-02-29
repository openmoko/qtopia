/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qstylefactory.h"
#include "qstyleplugin.h"
#include "private/qfactoryloader_p.h"
#include "qmutex.h"

#include "qapplication.h"
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#include "qcdestyle.h"
#ifndef QT_NO_STYLE_PLASTIQUE
#include "qplastiquestyle.h"
#endif
#ifndef QT_NO_STYLE_CLEANLOOKS
#include "qcleanlooksstyle.h"
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
#include "qwindowsxpstyle.h"
#endif

#if !defined(QT_NO_STYLE_MAC) && defined(Q_WS_MAC)
#  include <private/qt_mac_p.h>
#  include "qmacstyle_mac.h"
QString qt_mac_get_style_name()
{
    QString ret;
    Collection c = NewCollection();
    if (c) {
        GetTheme(c);
        Str255 str;
        long int s = 256;
        if(!GetCollectionItem(c, kThemeNameTag, 0, &s, &str)) {
            extern QString qt_mac_from_pascal_string(const Str255); //qglobal.cpp
            ret = qt_mac_from_pascal_string(str);
        }
    }
    DisposeCollection(c);
    return ret;
}
#endif

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QStyleFactoryInterface_iid, QCoreApplication::libraryPaths(), QLatin1String("/styles"), Qt::CaseInsensitive))
#endif

/*!
    \class QStyleFactory
    \brief The QStyleFactory class creates QStyle objects.

    \ingroup appearance

    The QStyle class is an abstract base class that encapsulates the
    look and feel of a GUI. QStyleFactory creates a QStyle object
    using the create() function and a key identifying the style. The
    styles are either built-in or dynamically loaded from a style
    plugin (see QStylePlugin).

    The valid keys can be retrieved using the keys()
    function. Typically they include "windows", "motif", "cde", and
    "plastique" and "cleanlooks".  Depending on the platform,
    "windowsxp" and "macintosh" may be available. Note that keys are
    case insensitive.

    \sa QStyle
*/

/*!
    Creates a QStyle object that matches the given \a key. This is either a
    built-in style, or a style from a style plugin.

    Note that keys are case insensitive.

    \sa keys()
*/
QStyle *QStyleFactory::create(const QString& key)
{
    QStyle *ret = 0;
    QString style = key.toLower();
#ifndef QT_NO_STYLE_WINDOWS
    if (style == QLatin1String("windows"))
        ret = new QWindowsStyle;
    else
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
    if (style == QLatin1String("windowsxp"))
        ret = new QWindowsXPStyle;
    else
#endif
#ifndef QT_NO_STYLE_MOTIF
    if (style == QLatin1String("motif"))
        ret = new QMotifStyle;
    else
#endif
#ifndef QT_NO_STYLE_CDE
    if (style == QLatin1String("cde"))
        ret = new QCDEStyle;
    else
#endif
#ifndef QT_NO_STYLE_PLASTIQUE
    if (style == QLatin1String("plastique"))
        ret = new QPlastiqueStyle;
    else
#endif
#ifndef QT_NO_STYLE_CLEANLOOKS
    if (style == QLatin1String("cleanlooks"))
        ret = new QCleanlooksStyle;
    else
#endif
#ifndef QT_NO_STYLE_MAC
    if (style.left(9) == QLatin1String("macintosh")) {
        ret = new QMacStyle;
#  ifdef Q_WS_MAC
        if (style == QLatin1String("macintosh"))
            style += QLatin1String(" (") + qt_mac_get_style_name() + QLatin1Char(')');
#  endif
    } else
#endif
    { } // Keep these here - they make the #ifdefery above work
#ifndef QT_NO_LIBRARY
    if(!ret) {
        if (QStyleFactoryInterface *factory = qobject_cast<QStyleFactoryInterface*>(loader()->instance(style)))
            ret = factory->create(style);
    }
#endif
    if(ret)
        ret->setObjectName(style);
    return ret;
}

/*!
    Returns the list of valid keys, i.e. the leys this factory can
    create styles for.

    \sa create()
*/
QStringList QStyleFactory::keys()
{
#ifndef QT_NO_LIBRARY
    QStringList list = loader()->keys();
#else
    QStringList list;
#endif
#ifndef QT_NO_STYLE_WINDOWS
    if (!list.contains(QLatin1String("Windows")))
        list << QLatin1String("Windows");
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
    if (!list.contains(QLatin1String("WindowsXP")))
        list << QLatin1String("WindowsXP");
#endif
#ifndef QT_NO_STYLE_MOTIF
    if (!list.contains(QLatin1String("Motif")))
        list << QLatin1String("Motif");
#endif
#ifndef QT_NO_STYLE_CDE
    if (!list.contains(QLatin1String("CDE")))
        list << QLatin1String("CDE");
#endif
#ifndef QT_NO_STYLE_PLASTIQUE
    if (!list.contains(QLatin1String("Plastique")))
        list << QLatin1String("Plastique");
#endif
#ifndef QT_NO_STYLE_CLEANLOOKS
    if (!list.contains(QLatin1String("Cleanlooks")))
        list << QLatin1String("Cleanlooks");
#endif
#ifndef QT_NO_STYLE_MAC
    QString mstyle = QLatin1String("Macintosh");
# ifdef Q_WS_MAC
    mstyle += " (" + qt_mac_get_style_name() + ")";
# endif
    if (!list.contains(mstyle))
        list << mstyle;
#endif
    return list;
}
