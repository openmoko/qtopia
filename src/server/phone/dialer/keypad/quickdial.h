/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef HAVE_QUICKDIAL_H
#define HAVE_QUICKDIAL_H

#include <qtopiaservices.h>

#include "qabstractdialerscreen.h"
#include "callcontactlist.h"

class NumberDisplay;
class QuickDialModel;

class PhoneQuickDialerScreen : public QAbstractDialerScreen
{
    Q_OBJECT
public:
    PhoneQuickDialerScreen( QWidget *parent, Qt::WFlags fl = 0 );
    ~PhoneQuickDialerScreen();

    virtual void reset();
    virtual void setDigits(const QString &digits);
    virtual void appendDigits(const QString &digits);
    virtual QString digits() const;

protected:
    bool eventFilter( QObject *o, QEvent *e );

signals:
    void numberSelected(const QString&, const QUniqueId&);

protected slots:
    void rejectEmpty(const QString&);
    void selectedNumber( const QString &num );
    void selectedNumber( const QString &num, const QUniqueId &cnt );
    void showEvent( QShowEvent *e );

private:
    void appendDigits( const QString &digits, bool refresh,
                       bool speedDial = true );

    NumberDisplay *mNumberDS;
    CallContactListView *mDialList;
    QString mNumber;
    bool mSpeedDial;
    QuickDialModel *mDialModel;
};

#endif
