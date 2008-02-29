/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef REPEATENTRY_H
#define REPEATENTRY_H

#include <qdialog.h>
#include <qtopia/pim/event.h>

class QButtonGroup;
class QSpinBox;
class QCheckBox;
class WeekGroup;
class QLabel;
class RichLabel;
class QDate;
class QPEDateButton;
class QVButtonGroup;
class QWidgetStack;

class RepeatEntry : public QDialog
{
    Q_OBJECT
public:
    RepeatEntry( bool startOnMonday, const PimEvent &rp, QWidget *parent = 0, 
	    const char *name = 0, bool modal = TRUE, WFlags fl = 0 );
    ~RepeatEntry();

    PimEvent event() const;

private slots:
    void setRepeatType(int);
    void setSubRepeatType(int);
    void setFrequency(int);

    void setRepeatOnWeekDay(int, bool);

    void setStartOnMonday(bool);
private:

    void refreshLabels();

    QButtonGroup *typeSelector;
    QSpinBox *freqSelector;

    WeekGroup *weekGroup;
    QVButtonGroup *monthGroup;

    QLabel *typeLabel;
    RichLabel *descLabel;

    QWidgetStack *fStack, *subStack;

    PimEvent mEvent;

    QString strDate, strWeekDay, strEndWeekDay;
};
#endif
