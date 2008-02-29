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

#include "memory.h"
#include "load.h"
#include "storage.h"
//#include "graphics.h"
#include "versioninfo.h"
#include "sysinfo.h"
#include "dataview.h"
#include "securityinfo.h"
#ifdef QTOPIA_CELL
#include "siminfo.h"
#include "modeminfo.h"
#endif

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#include <qtabwidget.h>
#include <qlayout.h>
#include <QScrollBar>
#include <QAbstractSlider>
#include <qpushbutton.h>
#include <QKeyEvent>

SystemInfo::SystemInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
#ifdef QTOPIA4_TODO
, wizard(0)
#endif
{
    setWindowIcon( QPixmap( ":image/SystemInfo" ) );
    setWindowTitle( tr("System Info") );

    QVBoxLayout *lay = new QVBoxLayout( this );
    lay->setMargin( 1 );
    tab = new QTabWidget( this );
    lay->addWidget( tab );

    tab->addTab( wrapWithScrollArea(new VersionInfo( tab )), tr("Version") );
    tab->addTab( new StorageInfoView(tab), tr("Storage"));

#ifdef SYSINFO_GEEK_MODE
    tab->addTab( wrapWithScrollArea(new LoadInfo(tab)), tr("CPU") );
    //tab->addTab( new Graphics( tab ), tr("Graphics") );
#endif

    tab->addTab( wrapWithScrollArea(new MemoryInfo(tab)), tr("Memory") );
    tab->addTab( wrapWithScrollArea(new DataView(tab)), tr("Data") );
    tab->addTab( wrapWithScrollArea(new SecurityInfo(tab)), tr("Security") );
#ifdef QTOPIA_CELL
    tab->addTab( wrapWithScrollArea(new ModemInfo(tab)), tr("Modem") );
    tab->addTab( wrapWithScrollArea(new SimInfo(tab)), tr("SIM") );
#endif
    resize( 220, 180 );

#ifdef QTOPIA_PHONE
    contextMenu = QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
#endif
}


bool SystemInfo::event(QEvent *e)
{
    if ( e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
            QWidget* w = tab->currentWidget();
            QAbstractScrollArea* area = NULL;
            if (w->inherits("QAbstractScrollArea"))
                area = qobject_cast<QAbstractScrollArea *>(w);
            else
                area = w->findChild<QAbstractScrollArea *>();
            if ( !area )
                return QWidget::event( e );
            QScrollBar* vertBar = area->verticalScrollBar();
            if ( !vertBar )
                return QWidget::event( e );
            if ( ke->key() == Qt::Key_Up )
                vertBar->triggerAction( QAbstractSlider::SliderSingleStepSub );
            else
                vertBar->triggerAction( QAbstractSlider::SliderSingleStepAdd );
        }
    }
#ifdef QTOPIA_PHONE
    if (e->type() == QEvent::Show)
        tab->currentWidget()->setEditFocus( true );
#endif
    if (e->type() == QEvent::ShortcutOverride /*|| e->type() == QEvent::KeyPress*/) {
        // If the tab widget has focus, then Left/Right keys will be
        // handled by the tab bar.  If we also handle them, we will
        // get double-tab jumps on every keypress.
        if ( !tab->hasFocus() ) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(e);
            if (ke && ke->key() == Qt::Key_Left && tab->currentIndex() > 0) {
                tab->setCurrentIndex((tab->currentIndex() + tab->count() - 1) % tab->count());
                return true;
            }
            else if (ke && ke->key() == Qt::Key_Right && (tab->currentIndex()+1) < tab->count()) {
                tab->setCurrentIndex((tab->currentIndex() + 1) % tab->count());
                return true;
            }
        }
    }
    return QWidget::event(e);
}

void SystemInfo::startCleanupWizard()
{
#ifdef QTOPIA4_TODO
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
    sv->setFocusPolicy( Qt::NoFocus );
    return sv;
}


/*!
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
