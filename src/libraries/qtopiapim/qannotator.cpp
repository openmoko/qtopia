/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include <qtopia/pim/private/qannotator_p.h>
#include <qtopianamespace.h>

QAnnotator::QAnnotator()
    : mIdGen("5c58d519-fd18-4063-847b-578df0be333e")
{
}

QAnnotator::~QAnnotator()
{
}

QUniqueId QAnnotator::add(const QByteArray &data, const QString &mimetype)
{
    QUniqueId id = mIdGen.createUniqueId();
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (!file.exists() && file.open(QIODevice::WriteOnly)) {
        QDataStream ds(&file);
        ds << mimetype;
        ds << data;
        file.close();
        return id;
    }
    return QUniqueId();
}

bool QAnnotator::set(const QUniqueId &id, const QByteArray &data, const QString &mimetype)
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (file.open(QIODevice::WriteOnly)) {
        QDataStream ds(&file);
        ds << mimetype;
        ds << data;
        file.close();
        return true;
    }
    return false;
}

void QAnnotator::remove(const QUniqueId &id)
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (file.exists())
        file.remove();
}

bool QAnnotator::contains(const QUniqueId &id) const
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    return file.exists();
}

QString QAnnotator::mimetype(const QUniqueId &id) const
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream ds(&file);
        QString mimetype;
        //QByteArray data;
        ds >> mimetype;
        //ds >> data;
        file.close();
        return mimetype;
    }
    return QString();
}

QByteArray QAnnotator::blob(const QUniqueId &id) const
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream ds(&file);
        QString mimetype;
        QByteArray data;
        ds >> mimetype;
        ds >> data;
        file.close();
        return data;
    }
    return QByteArray();
}
