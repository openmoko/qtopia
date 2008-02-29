/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef _e1_BAR_H_
#define _e1_BAR_H_

#include <QWidget>
#include <QList>
#include <QPair>
#include <QBitmap>
#include <qvaluespace.h>
class QColor;
class QLinearGradient;
class QPainter;
class QRect;
class E1BarItem;
class QMouseEvent;
class QMenu;
class E1Popup;

class E1Bar : public QWidget
{
Q_OBJECT
public:
    E1Bar(QWidget * = 0, Qt::WFlags = 0);
    virtual ~E1Bar();

    void addItem(E1BarItem *);
    void addSeparator();

    enum Border { BarBorder, ButtonBorder };
    void setBorder(Border);
    Border border() const;

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
private:
    friend class E1BarItem;
    QColor lighten(const QColor &, int) const;
    QColor darken(const QColor &, int) const;
    QColor pressed() const;
    QColor unpressed() const;
    QColor highlight() const;
    QLinearGradient gradient() const;
    QLinearGradient pressedGradient() const;
    void drawSeparator(QPainter *, int);
    void drawBorder(QPainter *);
    void updateRects();

    int m_clicked;
    QList<E1BarItem *> m_items;

    Border m_border;

    int leadin() const;
    int vertin() const;
    int leadout() const;
    int vertout() const;
};

class E1BarItem {
public:
    enum Flags { NoFlags = 0x00000000, Clickable = 0x00000001,
                 Clicked = 0x00000002, Enabled = 0x00000004,
                 Expanding = 0x00000008 };

    E1BarItem(Flags = Enabled);
    virtual ~E1BarItem();

    QRect rect() const;
    Flags flags() const;
    E1Bar * bar() const;

    void setFlag(Flags);
    void clearFlag(Flags);

    virtual QSize sizeHint() const;
    void setSizeHint(QSize);
protected:
    virtual void paint(QPainter *, const QRect &, const QColor &);
    virtual void clicked();

private:
    friend class E1Bar;
    QRect m_rect;
    Flags m_flags;
    E1Bar * m_bar;
    QSize m_sizeHint;
};

class E1Button : public QObject, public E1BarItem {
    Q_OBJECT
public:
    E1Button();

    void setPixmap(const QPixmap &, bool = true, bool = true);
    void setText(const QString &, bool = true);
    void setMargin(int);

    virtual QSize sizeHint() const;
protected:
    virtual void paint(QPainter *, const QRect &, const QColor &);

signals:
    void clicked();

private:
    bool m_stretch;
    int m_margin;
    QString m_text;
    QPixmap m_mask;
    bool m_tint;
    bool m_tintText;
};

class E1CloseButton : public E1Button
{
public:
    E1CloseButton();

protected:
    virtual void clicked();
};

class E1Menu : public E1Button
{
    Q_OBJECT
public:
    E1Menu();

    void setMenu(E1Popup *);
    E1Popup *menu();

    void activateMenu();

protected:
    virtual void paint(QPainter *, const QRect &, const QColor &);
    virtual void clicked();

private slots:
    void showMenu();

private:
    QRect m_lastDraw;
    E1Popup *m_menu;
};

class E1Spacer : public E1BarItem
{
public:
    E1Spacer(int);

protected:
    virtual QSize sizeHint() const;

private:
    int m_space;
};

class E1ContextMenu : public E1Menu
{
public:
};

// declare E1PhoneTelephonyBar
class E1PhoneTelephonyBar : public E1Bar
{
    Q_OBJECT
public:
    E1PhoneTelephonyBar(QWidget *);

private slots:
    void signalChanged();
    void timeChanged();
    void batteryChanged(int);

private:
    E1Button * signalBut;
    E1Button * batteryBut;
    E1Button * timeBut;

    QValueSpaceItem time;
    QValueSpaceItem signal;
    QPixmap batteryPixmap;
    QPixmap signalPixmap;

};


#endif // _e1_BAR_H_
