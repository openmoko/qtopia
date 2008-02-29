/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef FILEINFO_H
#define FILEINFO_H

#define QTOPIA_INTERNAL_APPLNKASSIGN

#include <qtopia/applnk.h>
#include <qtabdialog.h>
#include <qvbox.h>
#include <qmap.h>
#include "id3tag.h"


class MediaFileInfoWidgetPrivate;


/*
    Widget to display meta information about media files
*/
class MediaFileInfoWidget : public QDialog {
Q_OBJECT
    public:
	MediaFileInfoWidget( const DocLnk& lnk, QWidget *parent, const char *name = 0 );
	~MediaFileInfoWidget(); 
	void accept(); 
    private:
	MediaFileInfoWidgetPrivate *d;
};


#endif
