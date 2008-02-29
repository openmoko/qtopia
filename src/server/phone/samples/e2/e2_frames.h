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

#ifndef _e2_FRAMES_H_
#define _e2_FRAMES_H_

#include <QWidget>
#include <QItemDelegate>
#include <QListWidget>

void e2Center(QWidget *);

class E2Bar;
class E2PopupFrame : public QWidget
{
Q_OBJECT
public:
    E2PopupFrame(QWidget *parent = 0, Qt::WFlags flags = 0);

protected:
    virtual void paintEvent(QPaintEvent *);
};

class E2TitleFrame : public QWidget
{
Q_OBJECT
public:
    enum TitleType { GradientTitle, NormalTitle, NoTitle };
    E2TitleFrame(TitleType title = GradientTitle,
                   QWidget *parent = 0,
                   Qt::WFlags flags = 0);

    void setTitleText(const QString &);
    E2Bar *bar() const;

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

private:
    void doLayout();

    TitleType m_titleType;
    QString m_title;
    E2Bar *m_bar;
    QPixmap m_titleFill;
};

class E2ListDelegate : public QItemDelegate
{
Q_OBJECT
public:
    E2ListDelegate(QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem & option,
                       const QModelIndex & index ) const;
};

class E2ListWidget : public QListWidget
{
Q_OBJECT
public:
    E2ListWidget(QWidget *parent = 0);
};

#endif // _e2_FRAMES_H_

