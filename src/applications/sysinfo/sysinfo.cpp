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

#include "memory.h"
#include "load.h"
#include "storage.h"
#include "versioninfo.h"
#include "sysinfo.h"
#include "dataview.h"
#include "securityinfo.h"

#ifdef QTOPIA_CELL
#include "siminfo.h"
#include "modeminfo.h"
#endif

#include <qsoftmenubar.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <QScrollArea>

SystemInfo::SystemInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
#ifdef QTOPIA_UNPORTED
    , wizard(0)
#endif
{
    setWindowTitle( tr("System Info") );

    QVBoxLayout *lay = new QVBoxLayout( this );
    lay->setMargin( 1 );
    tab = new QTabWidget( this );
    lay->addWidget( tab );

    tab->addTab( wrapWithScrollArea(new VersionInfo( tab )), tr("Version") );
    tab->addTab( new StorageInfoView(tab), tr("Storage"));
#ifdef SYSINFO_GEEK_MODE
    tab->addTab( wrapWithScrollArea(new LoadInfo(tab)), tr("CPU") );
#endif
    tab->addTab( wrapWithScrollArea(new MemoryInfo(tab)), tr("Memory") );
    tab->addTab( wrapWithScrollArea(new DataView(tab)), tr("Data") );
    tab->addTab( new SecurityInfo(tab), tr("Security") );
#ifdef QTOPIA_CELL
    tab->addTab( new ModemInfo(tab), tr("Modem") );
    tab->addTab( wrapWithScrollArea(new SimInfo(tab)), tr("SIM") );
#endif

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
}

void SystemInfo::startCleanupWizard()
{
#ifdef QTOPIA_UNPORTED
    if (!wizard)
        wizard = new CleanupWizard(this, "CleanupWizard", WType_Modal);
    wizard->showMaximized();
#endif
}

QScrollArea *SystemInfo::wrapWithScrollArea(QWidget *widget)
{
    QScrollArea *sv = new QScrollArea();
    sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidgetResizable(true);
    sv->setWidget( widget );
    sv->setFocusPolicy( Qt::TabFocus );
    return sv;
}

/*TODO: Once the cleanup wizard has been ported it must be published using \service tag
 
    \service CleanupWizardService CleanupWizard
 
    \brief Provides the Qtopia CleanupWizard service.

    The \i CleanupWizard service enables applications to pop up the
    cleanup wizard for deleting old documents.
*/

/*!
    \internal
*/
CleanupWizardService::~CleanupWizardService()
{
}

/*!
    Start the cleanup wizard. The wizard allows the deletion of old documents,
    the cleanup of the mailbox and purges old (and finished) events.

    This slot corresponds to the QCop service message
    \c{CleanupWizard::showCleanupWizard()}.
*/
void CleanupWizardService::showCleanupWizard()
{
    parent->startCleanupWizard();
}
