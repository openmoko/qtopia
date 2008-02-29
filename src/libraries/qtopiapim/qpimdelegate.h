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

#ifndef PIMDELEGATE_H_
#define PIMDELEGATE_H_

#include <qtopiaglobal.h>

#include <QAbstractItemDelegate>
#include <QFont>
#include <QMetaType>
#include <QPair>
#include <QString>
#include <QPixmap>
#include <QHash>

class QPimDelegateData;
typedef QPair<QString, QString> StringPair;
class QTOPIAPIM_EXPORT QPimDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    QPimDelegate(QObject * = 0);
    ~QPimDelegate();

    // base delegate functionality
    void paint(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    enum SubTextAlignment {Independent, CuddledPerItem};
    enum BackgroundStyle {None, SelectedOnly, Gradient};

    // customizability (these should be overridden as needed)

    // Background/foreground
    virtual BackgroundStyle backgroundStyle(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual void drawBackground(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual void drawForeground(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex& index) const;

    // Decorations
    virtual void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    virtual QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSizeHint) const;

    // Main line of text
    virtual QString mainText(const QStyleOptionViewItem &option, const QModelIndex& index) const;

    // Subsidiary lines of text
    virtual QList<StringPair> subTexts(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual int subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual SubTextAlignment subTextAlignment(const QStyleOptionViewItem &option, const QModelIndex& index) const;

    // text line placement
    virtual QRect textRectangle(const QRect& entireRect, const QList<QRect>& leftFloats, const QList<QRect>& rightFloats, int top, int height) const;

    // Font selection
    virtual QFont mainFont(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual QFont secondaryHeaderFont(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual QFont secondaryFont(const QStyleOptionViewItem &option, const QModelIndex& index) const;

    // Helper function
    QFont differentFont(const QFont& start, int step) const;

private:
    // Data
    QPimDelegateData *d;
};

#endif // PIMDELEGATE_H_
