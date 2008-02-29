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

#include <QByteArray>

#include <quniqueid.h>

class QAnnotator {
public:
    QAnnotator();
    ~QAnnotator();

    QUniqueId add(const QByteArray &, const QString &mimetype = QString());

    bool set(const QUniqueId &, const QByteArray &, const QString &mimetype = QString());
    void remove(const QUniqueId &);

    bool contains(const QUniqueId &) const;

    QString mimetype(const QUniqueId &) const;
    QByteArray blob(const QUniqueId &) const;

    /* add IO device functions later.
       Should be able to open an iodevice for append/create
       should be able to open an iodevice for read
   */
private:
    QUniqueIdGenerator mIdGen;
};
