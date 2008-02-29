/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __FINDWIDGET_H__
#define __FINDWIDGET_H__

#include "findwidgetbase_p.h"

#include <qdatetime.h>

class Categories;
class DateBookMonth;

class FindWidgetPrivate;
class FindWidget : public FindWidgetBase
{
    Q_OBJECT
public:
    FindWidget( const QString &appName,
		QWidget *parent = 0, const char *name = 0 );
    ~FindWidget();

    QString findText() const;
    void setUseDate( bool show );
    void setDate( const QDate &dt );

public slots:
    void slotNotFound();
    void slotWrapAround();
    void slotDateChanged( int year, int month, int day );

signals:
    void signalFindClicked( const QString &txt, bool caseSensitive,
			    bool backwards, int category );
    void signalFindClicked( const QString &txt, const QDate &dt,
			    bool caseSensitive, bool backwards, int category );

private slots:
    void slotFindClicked();

private:
    QString mStrApp;
    Categories *mpCat;
    DateBookMonth *dtPicker;
    QDate mDate;
};

#endif
