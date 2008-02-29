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

#ifndef _QABSTRACTDIALERSCREEN_H_
#define _QABSTRACTDIALERSCREEN_H_

#include <QWidget>
class QString;
class QUniqueId;

class QAbstractDialerScreen : public QWidget
{
Q_OBJECT
public:
    QAbstractDialerScreen(QWidget *parent = 0, Qt::WFlags f = 0)
    : QWidget(parent, f) {}

    virtual QString digits() const = 0;

public slots:
    virtual void reset() = 0;
    virtual void appendDigits(const QString &digits) = 0;
    virtual void setDigits(const QString &digits) = 0;

signals:
    void requestDial(const QString &, const QUniqueId &);
    void speedDial(const QString&);
    void filterKeys(const QString&, bool&);
    void filterSelect(const QString&, bool&);
};


#endif // _QABSTRACTDIALERSCREEN_H_

