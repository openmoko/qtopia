/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CLEANUPWIZARD_H
#define CLEANUPWIZARD_H

#include <qwidget.h>
#include <qtopia/qwizard.h>

class PreselectionWidget;
class DocCleanWidget;
class DocResultWidget;
class DocSummaryWidget;
class MailCleanWidget;
class DatebookCleanWidget;
class FinalCleanupWidget;

class CleanupWizard : public QWizard
{
    Q_OBJECT
public:
    CleanupWizard(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~CleanupWizard();
protected:
    void showEvent(QShowEvent *);
    bool eventFilter(QObject *o, QEvent *e);
protected slots:
    virtual void next();
    virtual void back();
private slots:
    void addToFinalSummary(int noDeletedDocs);

private:
    void init();

    PreselectionWidget *m_PreselectionWidget;
    DocCleanWidget *m_DocCleanWidget;
    DocResultWidget *m_DocResultWidget;
    DocSummaryWidget * m_DocSummaryWidget;
    MailCleanWidget * m_MailCleanWidget;
    DatebookCleanWidget *m_DatebookCleanWidget;
    FinalCleanupWidget *m_FinalCleanupWidget;
};


#endif
