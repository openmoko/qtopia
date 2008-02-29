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

#ifndef CLEANUPWIZARD_H
#define CLEANUPWIZARD_H

#include <qwidget.h>

class QWidgetStack;
class PreselectionWidget;
class DocCleanWidget;
class DocResultWidget;
class DocSummaryWidget;
class MailCleanWidget;
class DatebookCleanWidget;
class FinalCleanupWidget;

class CleanupWizard : public QWidget
{
    Q_OBJECT
public:
    CleanupWizard(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~CleanupWizard();
protected:
    void keyPressEvent(QKeyEvent *keyEvent);
    void showEvent(QShowEvent *);
    bool eventFilter(QObject *o, QEvent *e);
private slots:
    void addToFinalSummary(int noDeletedDocs);
    void init();

private:
    enum WizardStyle { Default, NoBack, NoForward};
    void setContextBar(WizardStyle style);

    QWidgetStack * wStack;
    PreselectionWidget *m_PreselectionWidget;
    DocCleanWidget *m_DocCleanWidget;
    DocResultWidget *m_DocResultWidget;
    DocSummaryWidget * m_DocSummaryWidget;
    MailCleanWidget * m_MailCleanWidget;
    DatebookCleanWidget *m_DatebookCleanWidget;
    FinalCleanupWidget *m_FinalCleanupWidget;
};

#endif
