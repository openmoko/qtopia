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

#include "slideshowdialog.h"

#include <qlayout.h>

SlideShowDialog::SlideShowDialog( QWidget* parent, Qt::WFlags f )
    : QDialog( parent, f )
{
#define LAYOUT_MARGIN 5
#define LAYOUT_SPACING 4
#define PAUSE_MIN 5
#define PAUSE_MAX 60

    setWindowTitle( tr( "Slide Show" ) );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( LAYOUT_MARGIN );
    vbox->setSpacing( LAYOUT_SPACING );
    vbox->addStretch();

    // Construct pause between images label and slider
    slide_length_label = new QLabel( this );
    vbox->addWidget( slide_length_label );
    slide_length_slider = new QSlider( Qt::Horizontal, this );
    vbox->addWidget( slide_length_slider );
    connect( slide_length_slider, SIGNAL( valueChanged( int ) ),
        this, SLOT( updateSlideLengthLabel( int ) ) );
    slide_length_slider->setMinimum( PAUSE_MIN );
    slide_length_slider->setMaximum( PAUSE_MAX );

    // Construct display image name label and check box
    display_name_check = new QCheckBox( tr( "Display names" ), this );
    vbox->addWidget( display_name_check );

    // Construct loop through label and check box
    loop_through_check = new QCheckBox( tr( "Loop through" ), this );
    vbox->addWidget( loop_through_check );

    vbox->addStretch();
}
