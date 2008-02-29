/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <QAbstractListModel>
#include <QFont>
#include <QItemDelegate>

struct FontedItem
{
    FontedItem(const QString &lang, QFont &f, bool hasDictionary, bool isCurrentLanguage)
        : langName(lang), fnt(f), hasDict(hasDictionary), direction( Qt::LeftToRight ),
          isCurrentLang(isCurrentLanguage)
    {
    }

    FontedItem&operator=(const FontedItem &other)
    {
        langName = other.langName;
        fnt = other.fnt;
        hasDict = other.hasDict;
        isCurrentLang = other.isCurrentLang;
        return *this;
    }

    QString langName;
    QFont fnt;
    bool hasDict;
    Qt::LayoutDirection direction;
    bool isCurrentLang;
};

class LanguageModel : public QAbstractListModel
{
public:
    LanguageModel(QObject *parent, const QList<FontedItem> &l)
        :QAbstractListModel(parent), list(l) {}

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
    QList<FontedItem> list;

};

/*class LanguageDelegate : public QItemDelegate {

public:
    LanguageDelegate( QObject* parent )
        :QItemDelegate( parent )
    {
    }

    void paint( QPainter * p, const QStyleOptionViewItem& opt, const QModelIndex& index ) const
    {
        QItemDelegate::paint( p, option, index );
    }
};*/
