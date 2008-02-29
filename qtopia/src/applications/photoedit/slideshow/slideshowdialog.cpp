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

#include "slideshowdialog.h"

#include <qlayout.h>

SlideShowDialog::SlideShowDialog( QWidget* parent, const char* name, bool modal,
    WFlags f )
    : QDialog( parent, name, modal, f )
{
#define LAYOUT_MARGIN 5
#define LAYOUT_SPACING 4
#define PAUSE_MIN 5
#define PAUSE_MAX 60

    setCaption( tr( "Slide Show" ) );

    QVBoxLayout *vbox = new QVBoxLayout( this, LAYOUT_MARGIN, LAYOUT_SPACING );
    vbox->addStretch();
    
    // Construct pause between images label and slider
    slide_length_label = new QLabel( this );
    vbox->addWidget( slide_length_label );
    slide_length_slider = new QSlider( Qt::Horizontal, this );
    vbox->addWidget( slide_length_slider );
    connect( slide_length_slider, SIGNAL( valueChanged( int ) ),
        this, SLOT( updateSlideLengthLabel( int ) ) );
    slide_length_slider->setMinValue( PAUSE_MIN );
    slide_length_slider->setMaxValue( PAUSE_MAX ); 
    
    // Construct display image name label and check box
    display_name_check = new QCheckBox( tr( "Display names" ), this );
    vbox->addWidget( display_name_check );
    
    // Construct loop through label and check box
    loop_through_check = new QCheckBox( tr( "Loop through" ), this );
    vbox->addWidget( loop_through_check );
    
    vbox->addStretch();
}
