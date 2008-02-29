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

#ifndef IMAGESOURCEDIALOG_H
#define IMAGESOURCEDIALOG_H

#include "imagesource.h"
#include <qdialog.h>


#ifdef Q_WS_QWS
class ImageServerManager;
#endif
class ImageSourceWidgetPrivate;
class ImageSourceWidget : public QWidget
{
    Q_OBJECT
public:
    ImageSourceWidget( QWidget *parent, const char *name = 0 );
    ImageSourceWidget( const QPixmap &pix, QWidget *parent, const char *name = 0 );
    ~ImageSourceWidget();

    void setMaximumImageSize(const QSize &s);
    QSize maximumImageSize() const;

    virtual QPixmap pixmap() const;

public slots:
    virtual void setPixmap( const QPixmap & );

protected slots:
    void change();
    void remove();

protected:
    void haveImage( bool f );
    void resizeEvent(QResizeEvent *);

private:
    void init();
#ifdef Q_WS_QWS
    ImageServerManager *is;
#endif
    ImageSourceWidgetPrivate *d;
};

class ImageSourceDialog : public QDialog
{
    Q_OBJECT
public:
    ImageSourceDialog( QWidget *parent, const char *name = 0, bool modal = TRUE, WFlags fl =
#ifdef QTOPIA_DESKTOP
    WStyle_Customize | WStyle_Dialog | WStyle_Title 
#else
     0
#endif
    );
    ImageSourceDialog( const QPixmap &pix, QWidget *parent, const char *name = 0, bool modal = TRUE, WFlags fl = 0 );

    void setMaximumImageSize(const QSize &s);
    QSize maximumImageSize() const;

    virtual void setPixmap( const QPixmap &pix );
    virtual QPixmap pixmap() const;
private:
    void init();
    ImageSourceWidget *mWidget;
};

#endif
