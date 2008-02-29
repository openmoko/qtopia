/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifdef QTOPIA4_TODO
#ifndef QTOPIAPIMWINEXP_H
#define QTOPIAPIMWINEXP_H

#ifdef QTOPIAPIM_TEMPLATEDLL
//MOC_SKIP_BEGIN


#if defined(QVALUELIST_H) && defined (PIM_CONTACT_H) && !defined (HAVE_QVALUELIST_PIMCONTACT)
#define HAVE_QVALUELIST_PIMCONTACT
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QList<QContact>;
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QList<QContact::ContactFields>;
#endif

#if defined (QMAP_H) && defined (PIM_CONTACT_H) && !defined (HAVE_QMAP_INT_PIMCONTACT)
#define HAVE_QMAP_INT_PIMCONTACT
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QMap<int, QContact::ContactFields>;
#endif

#if defined (QMAP_H) && defined (QPIXMAP_H) && !defined (HAVE_QMAP_INT_QPIXMAP)
#define HAVE_QMAP_INT_QPIXMAP
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QMap<int, QPixmap>;
#endif

#if defined (QVALUELIST_H) && defined (QUUID_H) && !defined (HAVE_QVALUELIST_QUUID)
#define HAVE_QVALUELIST_QUUID
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QList<QUniqueId>;
#endif

#if defined (QMAP_H) && defined (PIM_CONTACT_H) && !defined (HAVE_QMAP_QLISTVIEW_PIMCONTACT)
#define HAVE_QMAP_QLISTVIEW_PIMCONTACT
    class QListViewItem;
    QTOPIAPIM_TEMPLATE_EXTERN template class QTOPIAPIM_EXPORT QMap<QListViewItem *, QContact::ContactFields>;
#endif


//MOC_SKIP_END
#endif // QTOPIAPIM_TEMPLATEDLL

#endif // QTOPIAPIMWINEXP_H
#endif

