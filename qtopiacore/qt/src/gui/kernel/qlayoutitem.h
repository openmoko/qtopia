/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QLAYOUTITEM_H
#define QLAYOUTITEM_H

#include <QtGui/qsizepolicy.h>
#include <QtCore/qrect.h>

#include <limits.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

static const int QLAYOUTSIZE_MAX = INT_MAX/256/16;

class QLayout;
class QLayoutItem;
class QSpacerItem;
class QWidget;
class QSize;

class Q_GUI_EXPORT QLayoutItem
{
public:
    inline explicit QLayoutItem(Qt::Alignment alignment = 0);
    virtual ~QLayoutItem();
    virtual QSize sizeHint() const = 0;
    virtual QSize minimumSize() const = 0;
    virtual QSize maximumSize() const = 0;
    virtual Qt::Orientations expandingDirections() const = 0;
    virtual void setGeometry(const QRect&) = 0;
    virtual QRect geometry() const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth(int) const;
    virtual int minimumHeightForWidth(int) const;
    virtual void invalidate();

    virtual QWidget *widget();
    virtual QLayout *layout();
    virtual QSpacerItem *spacerItem();

    Qt::Alignment alignment() const { return align; }
    void setAlignment(Qt::Alignment a);

protected:
    Qt::Alignment align;
};

inline QLayoutItem::QLayoutItem(Qt::Alignment aalignment)
    : align(aalignment) { }

class Q_GUI_EXPORT QSpacerItem : public QLayoutItem
{
public:
    QSpacerItem(int w, int h,
                 QSizePolicy::Policy hData = QSizePolicy::Minimum,
                 QSizePolicy::Policy vData = QSizePolicy::Minimum)
        : width(w), height(h), sizeP(hData, vData) { }
    void changeSize(int w, int h,
                     QSizePolicy::Policy hData = QSizePolicy::Minimum,
                     QSizePolicy::Policy vData = QSizePolicy::Minimum);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    Qt::Orientations expandingDirections() const;
    bool isEmpty() const;
    void setGeometry(const QRect&);
    QRect geometry() const;
    QSpacerItem *spacerItem();

private:
    int width;
    int height;
    QSizePolicy sizeP;
    QRect rect;
};

class Q_GUI_EXPORT QWidgetItem : public QLayoutItem
{
public:
    explicit QWidgetItem(QWidget *w) : wid(w) { }
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    Qt::Orientations expandingDirections() const;
    bool isEmpty() const;
    void setGeometry(const QRect&);
    QRect geometry() const;
    virtual QWidget *widget();

    bool hasHeightForWidth() const;
    int heightForWidth(int) const;

private:
    QWidget *wid;
    Q_DISABLE_COPY(QWidgetItem)
};

QT_END_HEADER

#endif // QLAYOUTITEM_H
