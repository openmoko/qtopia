/****************************************************************************
**
** Copyright (C) 2006-2007 TROLLTECH ASA. All rights reserved.
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
#include "loggingedit.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qtranslatablesettings.h>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QWhatsThis>
#include <QHelpEvent>
#include <QMenu>


LoggingEdit::LoggingEdit( QWidget* parent, Qt::WFlags fl )
:   QDialog( parent, fl )
{
    setWindowTitle(tr("Categories"));

    QVBoxLayout* vbLayout = new QVBoxLayout(this);
    vbLayout->setMargin(0);
    vbLayout->setSpacing(0);

    QMenu* menu = QSoftMenuBar::menuFor( this );

    // This implementation is a generic mechanism for adding a What's This?
    // menu item to the softmenubar menu.
    //
    // This is a prototype for a mechanism to be put into a Qtopia library,
    // potentially Qtopia Core or Qt proper.
    //
    // See also LoggingEdit::showWhatsThis below.
    //
    QAction *aWhatsThis = QWhatsThis::createAction(this);
    QAction *qtopiaWhatsThis = new QAction(aWhatsThis->icon(),aWhatsThis->text(),this);
    connect( qtopiaWhatsThis, SIGNAL(triggered()), this, SLOT(showWhatsThis()) );
    menu->addAction( qtopiaWhatsThis );
    delete aWhatsThis;

    // Using a tree since when the number of log streams gets large, we
    // should put them in an hierarchy.
    list = new QTreeWidget;
    connect( list, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemClicked(QTreeWidgetItem*)) );

    list->setRootIsDecorated(false);
    list->setColumnCount(1);
    list->header()->hide();
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setFrameStyle(QFrame::NoFrame);
    vbLayout->addWidget(list);

    QTranslatableSettings settings("Trolltech","Log");

    foreach (QString group, settings.childGroups()) {
        settings.beginGroup(group);
        QString name = settings.value("Name").toString();
        QStringList missing = settings.value("Requires").toStringList();

#ifdef QTOPIA_CELL
        missing.removeAll("CELL");
#endif
#ifdef QTOPIA_DRM
        missing.removeAll("DRM");
#endif
#ifndef QT_NO_SXE
        missing.removeAll("SXE");
#endif
#ifdef QTOPIA_VOIP
        missing.removeAll("VOIP");
#endif
#ifdef QTOPIA_TEST
        missing.removeAll("TEST");
#endif

        if ( !name.isEmpty() && missing.isEmpty() ) {
            QTreeWidgetItem *i = new QTreeWidgetItem(list,QStringList() << name);
            i->setCheckState(0,qtopiaLogEnabled(group.toLatin1()) ? Qt::Checked : Qt::Unchecked);
            if (!qtopiaLogOptional(group.toLatin1())) {
                i->setFlags(i->flags() & ~Qt::ItemIsEnabled);
            }
            i->setWhatsThis(0,settings.value("Help").toString());
            item.insert(group,i);
        }
        settings.endGroup();
    }

    list->sortItems( 0, Qt::AscendingOrder );
    list->setCurrentItem(list->topLevelItem(0));
    showMaximized();
}

LoggingEdit::~LoggingEdit()
{
}

void LoggingEdit::showWhatsThis()
{
    // This implementation is a generic mechanism for finding the widget
    // and the point of interest within that widget for which a QHelpEvent
    // can be sent (and send it).
    //
    // This is a prototype for a mechanism to be put into a Qtopia library,
    // potentially Qtopia Core or Qt proper.
    //
    // See also qtopiaWhatsThis above.
    //
    QWidget* whatswhat = QApplication::focusWidget();
    if ( whatswhat ) {
        QPoint p = whatswhat->inputMethodQuery(Qt::ImMicroFocus).toRect().center();
        QPoint gp = whatswhat->mapToGlobal(p);
        QWidget *whatsreallywhat = QApplication::widgetAt(gp);
        if ( whatsreallywhat ) {
            whatswhat = whatsreallywhat;
            p = whatsreallywhat->mapFromGlobal(gp);
        }
        QHelpEvent e(QEvent::WhatsThis, p, gp);
        QApplication::sendEvent(whatswhat, &e);
    }
}

void LoggingEdit::accept()
{
    QTranslatableSettings settings("Trolltech","Log");

    foreach (QString group, settings.childGroups()) {
        QTreeWidgetItem *i = item[group];
        if ( i ) {
            settings.beginGroup(group);
            settings.setValue("Enabled",item[group]->checkState(0) == Qt::Checked);
            settings.endGroup();
        }
    }

    QDialog::accept();
}

void LoggingEdit::itemClicked( QTreeWidgetItem *clickedItem )
{
    if ( clickedItem->flags() & Qt::ItemIsEnabled )
        clickedItem->setCheckState( 0, (clickedItem->checkState(0) == Qt::Checked ? Qt::Unchecked : Qt::Checked) );
}

