
/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/qpeglobal.h>

#ifdef QTOPIA_TEMPLATEDLL
//MOC_SKIP_BEGIN

#if ( defined(Q_DEFINED_QLIST) || defined(Q_DEFINED_QPTRLIST) )&& defined(QTOPIA_DEFINED_APPLNK) && !defined(QTOPIA_EXPORTED_APPLNK_TEMPLATES)
#define QTOPIA_EXPORTED_APPLNK_TEMPLATES 
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QList<AppLnk>;
#endif

#if defined(QTOPIA_DEFINED_CATEGORYGROUP) && defined(Q_DEFINED_QMAP) && !defined(QTOPIA_EXPORTED_QTPALMTOP_CATEGORY_TEMPLATES)
#define QTOPIA_EXPORTED_QTPALMTOP_CATEGORY_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QMap<QString, CategoryGroup>;
#endif

#if  defined(Q_DEFINED_QMAP) && defined(Q_DEFINED_QSTRING) && !defined(QTOPIA_EXPORTED_CONFIG_TEMPLATES)
#define QTOPIA_EXPORTED_CONFIG_TEMPLATES 
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QMapIterator<QString, QMap<QString, QString> >;
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QMap<QString, QMap<QString, QString> >;
#endif

#if ( defined(Q_DEFINED_QLIST) || defined(Q_DEFINED_QPTRLIST) ) && defined(QTOPIA_DEFINED_FILESYSTEM) && !defined(QTOPIA_EXPORTED_STORAGE_TEMPLATES)
#define QTOPIA_EXPORTED_STORAGE_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QList<FileSystem>;
#endif

#if defined(QTOPIA_DEFINED_EVENT) && defined (Q_DEFINED_QVALUELIST) && !defined(QTOPIA_EXPORTED_EVENT_TEMPLATES)
#define QTOPIA_EXPORTED_EVENT_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QValueList<Event>;
#endif

// Special case QLibrary is will be defined in Qtopia under Qt2.3
#if defined(Q_DEFINED_QDICT) && defined(Q_DEFINED_QLIBRARY) && (QT_VERSION < 300) && !defined(QTOPIA_EXPORTED_QLIBRAY_TEMPLATES) 
#define QTOPIA_EXPORTED_QLIBRAY_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QDict<QLibrary>;
#endif

#if defined (QTOPIA_DEFINED_DEVICEBUTTON) && defined(Q_DEFINED_QVALUELIST) && !defined(QTOPIA_EXPORTED_BUTTON_MANAGER_TEMPLATES)
#define QTOPIA_EXPORTED_BUTTON_MANAGER_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QValueList<DeviceButton>;
#endif

#if defined(Q_DEFINED_QGUARDEDPTR) && defined(Q_DEFINED_QVALUELIST) && defined(Q_DEFINED_QWIDGET) && !defined(QTOPIA_EXPORTED_QMENUBAR_TEMPLATES)
#define QTOPIA_EXPORTED_QMENUBAR_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QValueList<QGuardedPtr< QWidget> >;
#endif

//MOC_SKIP_END
#endif //QTOPIA_TEMPLATEDLL
