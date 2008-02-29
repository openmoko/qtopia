/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
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

#if defined(Q_DEFINED_QMAP) && defined (Q_DEFINED_QSTRING) && !defined(QTOPIA_EXPORTED_CONFIG_TEMPLATES)
#define QTOPIA_EXPORTED_CONFIG_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QMapIterator<QString, QMap<QString,QString> >;
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QMap<QString, QMap<QString,QString> >;
#endif

#if defined(QTOPIA_DEFINED_CATEGORYGROUP) && defined (Q_DEFINED_QSTRING) && defined(Q_DEFINED_QMAP) && !defined(QTOPIA_EXPORTED_QTPALMTOP_CATEGORY_TEMPLATES)
#define QTOPIA_EXPORTED_QTPALMTOP_CATEGORY_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QMap<QString, CategoryGroup>;
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
#if defined(Q_DEFINED_QDICT) && defined(Q_DEFINED_QLIBRARY) && (QT_VERSION < 0x030000) && !defined(QTOPIA_EXPORTED_QLIBRAY_TEMPLATES) 
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

#if defined(Q_DEFINED_QLIST) && defined(QPIXMAP_H) && !defined(QTOPIA_EXPORTED_ABEDITOR_TEMPLATES)
#define QTOPIA_EXPORTED_ABEDITOR_TEMPLATES
QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QList<QPixmap>;
#endif

//MOC_SKIP_END
#endif //QTOPIA_TEMPLATEDLL

