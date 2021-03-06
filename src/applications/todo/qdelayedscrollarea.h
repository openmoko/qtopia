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
#ifndef DELAYEDSCROLLAREA
#define DELAYEDSCROLLAREA
#include <QScrollArea>

class QDelayedScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    QDelayedScrollArea(int index, QWidget *parent = 0);
    QDelayedScrollArea(QWidget *parent = 0);
    ~QDelayedScrollArea();

    void setIndex(int index) { i = index; }
    int index() const { return i; }

signals:
    void aboutToShow(int);

protected:
    bool eventFilter(QObject *receiver, QEvent *event);

    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);
    void adjustWidget(int width);

private:
    int i;
};

#endif // DELAYEDSCROLLAREA
