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
#include "speeddialedit.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#include <qtranslatablesettings.h>

#include <QVBoxLayout>


QSpeedDialEdit::QSpeedDialEdit( QWidget* parent, Qt::WFlags fl )
:   QDialog( parent, fl ),
    origEmpty(),
    origSet()
{
    setWindowTitle(tr("Speed Dial"));

    QVBoxLayout* vbLayout = new QVBoxLayout(this);
    vbLayout->setMargin(0);
    vbLayout->setSpacing(0);

    list = new QSpeedDialList(0);
    list->setFrameStyle(QFrame::NoFrame);
    vbLayout->addWidget(list);

    // Store original settings
    for ( int i=0; i<list->count(); ++i ) {
        QString input = list->rowInput( i );
        QtopiaServiceDescription* desc = QSpeedDial::find( input );
        if ( desc != 0 )
            origSet[input] = *desc;
        else
            origEmpty.append( input );
    }
}

QSpeedDialEdit::~QSpeedDialEdit()
{
}

void QSpeedDialEdit::reject()
{
    QMap<QString, QtopiaServiceDescription>::const_iterator setCit;
    for ( setCit = origSet.begin(); setCit != origSet.end(); ++setCit ) {
        QSpeedDial::set( setCit.key(), setCit.value() );
    }

    QList<QString>::const_iterator emptyCit;
    for ( emptyCit = origEmpty.begin(); emptyCit != origEmpty.end(); ++emptyCit ) {
        if ( QSpeedDial::find( *emptyCit ) != 0 )
            QSpeedDial::remove( *emptyCit );
    }

    QDialog::reject();
}

