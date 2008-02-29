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

#ifndef __FINDWIDGET_H__
#define __FINDWIDGET_H__

#include "ui_findwidgetbase_p.h"

#include <qdatetime.h>

class QtopiaCalendarWidget;

class FindWidget : public QWidget, public Ui::FindWidgetBase
{
    Q_OBJECT
public:
    FindWidget( const QString &appName, QWidget *parent = 0 );
    ~FindWidget();

    QString findText() const;
    void setUseDate( bool show );

public slots:
    void slotNotFound();
    void slotWrapAround();
    void setDate( const QDate &dt );

signals:
    void signalFindClicked( const QString &txt, bool caseSensitive,
                            bool backwards, const QCategoryFilter & category );
    void signalFindClicked( const QString &txt, const QDate &dt,
                            bool caseSensitive, bool backwards, const QCategoryFilter & category );

private slots:
    void slotFindClicked();

private:
    QString mStrApp;
    QtopiaCalendarWidget *dtPicker;
    QDate mDate;
};

#endif
