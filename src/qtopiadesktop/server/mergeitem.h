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
#ifndef __MERGE_ITEM_H__
#define __MERGE_ITEM_H__

#include <QByteArray>
#include <QString>

/*
   Because the structures for pim data types are taken from XML this is a tree like structure.

   There are a number of differences though.  Any list of any kind is assumed to be unordered and
   will be sorted internally in the structure.  Also identifier elements are extracted and stored separately.
*/

class MergeElement;
class QXmlStreamWriter;
class QSyncMerge;
class MergeItem
{
public:
    enum ChangeSource {
        Server,
        Client,
        IdentifierOnly,
        DataOnly,
    };

    MergeItem(QSyncMerge *);
    ~MergeItem();

    /* future intent
       reading server/client as if from storage stub/client device.
       writing server/client as if to send to storage stub/client device.
       writing dataonly suitable for finding identical item clashs
       writing identifieronly includes both server and client ids.

       intent, read dataonly followed by identifier only for whole
       merge item.  Currently identifieronly overwrites data only,
       however architecture will allow memory reduction later
       without changing the API
       */
    QByteArray write(ChangeSource) const;
    bool read(const QByteArray &,ChangeSource);

    void restrictTo(const MergeItem &);

    QString serverIdentifier() const;
    QString clientIdentifier() const;

    QString dump() const;

    // may need to do mappings....
private:
    // attempt for now is to disable copying.  Can implement later
    MergeItem(const MergeItem &) {}
    MergeElement *rootElement;
    QSyncMerge *mMerge; // for id mapping.

    void writeElement(QXmlStreamWriter &, MergeElement *item, ChangeSource) const;
};

#endif// __MERGE_ITEM_H__
