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

#ifndef _EXZ_POPUP_H_
#define _EXZ_POPUP_H_

#include <QPixmap>
#include <QWidget>
#include <QSize>
class QListWidget;
class QMouseEvent;
class QKeyEvent;

class E1Popup : public QWidget
{
Q_OBJECT
public:
    E1Popup();

    void addItem(const QString &);
    void addItem(const QPixmap &);
    void addItem(E1Popup *);

    virtual QSize sizeHint() const;

    void popup(const QPoint &);


signals:
    void closed();
    void selected(int);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void showEvent(QShowEvent *);

private slots:
    void doClose();

private:
    struct PopupItem {
        PopupItem() : popup(0) {}
        QString text;
        QPixmap pix;
        E1Popup *popup;

        QSize size;
    };
    QList<PopupItem> m_items;
    QSize sizeHint(const PopupItem &) const;
    void paint(QPainter *, const QRect &, const PopupItem &, bool);
    int findItem(const QPoint &);

    int m_selected;
};

#endif // _EXZ_POPUP_H_

