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
#ifndef GOOGLEACCOUNT_H
#define GOOGLEACCOUNT_H

#include "ui_googleaccount.h"
#include <QDialog>
#include <QString>

#include <qtopia/pim/private/qgooglecontext_p.h>

class GoogleAccount : public QDialog, public Ui::GoogleAccount
{
    Q_OBJECT
public:
    GoogleAccount( QWidget *parent = 0 );
    ~GoogleAccount();

    void setEmail(const QString &);
    void setPassword(const QString &);
    void setName(const QString &);
    void setFeedType(QGoogleCalendarContext::FeedType);

    QString email() const;
    QString password() const;
    QGoogleCalendarContext::FeedType feedType() const;
};

#endif // GOOGLEACCOUNT_H
