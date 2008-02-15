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

#ifndef _SLIDEINMESSAGEBOX_H_
#define _SLIDEINMESSAGEBOX_H_

#include <QWidget>
#include "qabstractmessagebox.h"
class QString;
class SlideInMessageBoxPrivate;

class SlideInMessageBox : public QAbstractMessageBox
{
Q_OBJECT
public:
    SlideInMessageBox(QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~SlideInMessageBox();

    virtual void setButtons(Button button1, Button button2);
    virtual void setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text,
            int defaultButtonNumber, int escapeButtonNumber);

    virtual Icon icon() const;
    virtual void setIcon(Icon);

    virtual QString title() const;
    virtual void setTitle(const QString &);

    virtual QString text() const;
    virtual void setText(const QString &);

    virtual QPixmap pixmap() const;
    virtual void setPixmap(const QPixmap &);

protected:
    virtual void showEvent(QShowEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void valueChanged(qreal);

private:
    void renderBox();

    void animate();
    void drawFrame(QPainter *painter, const QRect &r, const QString &title);

    SlideInMessageBoxPrivate *d;
};

#endif // _SLIDEINMESSAGEBOX_H_

