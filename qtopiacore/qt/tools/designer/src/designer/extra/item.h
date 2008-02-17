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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ITEM_H
#define ITEM_H

#include <QtCore/QString>
#include <QtGui/QPixmap>

class Item
{
public:
    enum ItemType { BusinessCard, Weapon, Armor };
    Item(ItemType type, const QString &pixName, const QString &name, const QString &description)
        : m_name(name), m_description(description), m_type(type), m_pic(pixName) {}
    virtual ~Item() {}
    inline QString name() const { return m_name; }
    inline QString description() const { return m_description; }
    inline ItemType type() const { return m_type; }
    inline QString pixmapName() const { return m_pic; }

private:
    QString m_name;
    QString m_description;
    ItemType m_type;
    QString m_pic;
};

class BusinessCard : public Item
{
public:
    BusinessCard(const QString &name, const QString &description, const QString &bigpic)
        : Item(Item::BusinessCard, QLatin1String(":/qthack/images/bcard.png"),
               name, description), m_bigPic(bigpic)
    {}
    inline QString bigPicture() const { return m_bigPic; }
private:
    QString m_bigPic;
};
#endif
