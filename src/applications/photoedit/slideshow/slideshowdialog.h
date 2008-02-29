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

#ifndef SLIDESHOWDIALOG_H
#define SLIDESHOWDIALOG_H

#include <qdialog.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qstring.h>

class SlideShowDialog : public QDialog {
    Q_OBJECT
public:
    SlideShowDialog( QWidget* parent = 0, Qt::WFlags f = 0 );

    // Return length of each slide in seconds
    int slideLength() { return slide_length_slider->value(); }

    // Return if true display name has been enabled
    bool isDisplayName() { return display_name_check->isChecked(); }

    // Return if true loop through has been enabled
    bool isLoopThrough() { return loop_through_check->isChecked(); }

private slots:
    // Update label with pause value
    void updateSlideLengthLabel( int sec ) { slide_length_label->setText(
        QString( tr("Slide length ( %1 sec. )") ).arg( sec ) ) ; }

private:
    QLabel *slide_length_label;
    QSlider *slide_length_slider;
    QCheckBox *display_name_check, *loop_through_check;
};

#endif
