/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qmime.h"

/*!
    \class QMimeSource
    \brief The QMimeSource class is an abstraction of objects that
           provided formatted data of a certain MIME type.

    \obsolete

    The preferred approach to drag and drop is to use QDrag in
    conjunction with QMimeData. See \l{Drag and Drop} for details.

    \sa QMimeData, QDrag
*/

/*!
    Destroys the MIME source.
*/
QMimeSource::~QMimeSource()
{
}

/*!
    \fn const char *QMimeSource::format(int i) const

    Returns the (\a i - 1)-th supported MIME format, or 0.
*/

/*!
    \fn QByteArray QMimeSource::encodedData(const char *format) const

    Returns the encoded data of this object in the specified MIME
    \a format.
*/

/*!
    Returns true if the object can provide the data in format \a
    mimeType; otherwise returns false.

    If you inherit from QMimeSource, for consistency reasons it is
    better to implement the more abstract canDecode() functions such
    as QTextDrag::canDecode() and QImageDrag::canDecode().
*/
bool QMimeSource::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
        if (!qstricmp(mimeType,fmt))
            return true;
    }
    return false;
}
