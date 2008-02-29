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

#ifndef QTOPIAPIMWINEXP_H
#define QTOPIAPIMWINEXP_H

#ifdef QTOPIAPIM_TEMPLATEDLL
//MOC_SKIP_BEGIN


#if defined(QVALUELIST_H) && defined (PIM_CONTACT_H) && !defined (HAVE_QVALUELIST_PIMCONTACT)
#define HAVE_QVALUELIST_PIMCONTACT
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QValueList<PimContact>;
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QValueList<PimContact::ContactFields>;
#endif

#if defined (QMAP_H) && defined (PIM_CONTACT_H) && !defined (HAVE_QMAP_INT_PIMCONTACT)
#define HAVE_QMAP_INT_PIMCONTACT
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QMap<int, PimContact::ContactFields>;
#endif

#if defined (QMAP_H) && defined (QPIXMAP_H) && !defined (HAVE_QMAP_INT_QPIXMAP)
#define HAVE_QMAP_INT_QPIXMAP
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QMap<int, QPixmap>;
#endif

#if defined (QVALUELIST_H) && defined (QUUID_H) && !defined (HAVE_QVALUELIST_QUUID)
#define HAVE_QVALUELIST_QUUID
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QValueList<QUuid>;
#endif

#if defined (QMAP_H) && defined (PIM_CONTACT_H) && !defined (HAVE_QMAP_QLISTVIEW_PIMCONTACT)
#define HAVE_QMAP_QLISTVIEW_PIMCONTACT
    class QListViewItem;
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QMap<QListViewItem *, PimContact::ContactFields>;
#endif


//MOC_SKIP_END
#endif // QTOPIAPIM_TEMPLATEDLL

#endif // QTOPIAPIMWINEXP_H
