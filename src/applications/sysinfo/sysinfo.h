/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <QWidget>
#include <QScrollArea>
#include <qtopiaabstractservice.h>

#ifdef QTOPIA4_TODO

#ifdef QTOPIA_PHONE
#include "cleanupwizard.h"
#else
#include "cleanupwizard_pda.h"
#endif

#endif

class QTabWidget;
class QMenu;
class SystemInfo : public QWidget
{
    Q_OBJECT
public:
    SystemInfo( QWidget *parent = 0, Qt::WFlags f = 0 );
protected:
    bool event(QEvent *e);

public slots:
    void startCleanupWizard();
private:

    QTabWidget *tab;
#ifdef QTOPIA4_TODO
    CleanupWizard * wizard;
#endif
#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
#endif
    static QScrollArea *wrapWithScrollArea(QWidget *);
};

class CleanupWizardService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class SystemInfo;
private:
    CleanupWizardService( SystemInfo *parent )
        : QtopiaAbstractService( "CleanupWizard", parent )
        { this->parent = parent; publishAll(); }

public:
    ~CleanupWizardService();

public slots:
    void showCleanupWizard();

private:
    SystemInfo *parent;
};
