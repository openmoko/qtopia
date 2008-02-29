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

#include "wallpaperselector.h"
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/applnk.h>
#include <qdir.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qimage.h>
#include <qtimer.h>

class ImageItem : public QListBoxText
{
public:
    ImageItem( QListBox * listbox, const QString &t )
	: QListBoxText( listbox, t ) {}
    ImageItem( const QString &t, const QString &f )
	: QListBoxText( t ), fname(f) {}
    void setFilename( const QString &f ) { fname = f; }
    const QString &filename() const { return fname; }
private:
    QString fname;
};


WallpaperSelector::WallpaperSelector( const QString &dflt, QWidget *parent, const char *name )
    : WallpaperSelectorBase( parent, name, TRUE, WStyle_ContextHelp ), image(dflt)
{
    preview->setPixmap( QPixmap() );
    preview->setFixedHeight( QMAX(height()/4,7) );
    // Wallpapers have no paths
    if ( image.find( '/' ) >= 0 ) {
	sourceButtonGroup->setButton( 1 );
	sourceSelected( 1 );
    } else {
	sourceButtonGroup->setButton( 0 );
	sourceSelected( 0 );
    }
}

void WallpaperSelector::sourceSelected( int s )
{
    QListBoxItem *hilite = 0;
    wallpaperListBox->clear();
    wallpaperListBox->setEnabled( FALSE );
    if ( s == 0 ) {
	QStringList qpepaths = Global::qtopiaPaths();
	for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
	    QString picsPath = *qit + "pics/wallpaper";
	    QDir dir( picsPath );
	    QStringList list = dir.entryList( "*.png" );
	    QStringList::Iterator it;
	    for ( it = list.begin(); it != list.end(); ++it ) {
		QString name =  (*it).left((*it).findRev('.'));
		ImageItem *item = new ImageItem( name, *it );
		wallpaperListBox->insertItem( item );
		if ( *it == image )
		    hilite = item;
	    }
	    list = dir.entryList( "*.xpm" );
	    for ( it = list.begin(); it != list.end(); ++it ) {
		QString name =  (*it).left((*it).findRev('.'));
		ImageItem *item = new ImageItem( name, *it );
		wallpaperListBox->insertItem( item );
		if ( *it == image )
		    hilite = item;
	    }
	}
    } else {
	DocLnkSet files;
	Global::findDocuments(&files, "image/*");
	QListIterator<DocLnk> dit( files.children() );
	for ( ; dit.current(); ++dit ) {
	    ImageItem *item = new ImageItem( (*dit)->name(), (*dit)->file() );
	    wallpaperListBox->insertItem( item );
	    if ( (*dit)->file() == image )
		hilite = item;
	}
    }
    wallpaperListBox->setEnabled( TRUE );
    if ( !hilite )
	hilite = wallpaperListBox->item( 0 );
    if ( hilite )
	wallpaperListBox->setSelected( hilite, TRUE );
}

void WallpaperSelector::imageSelected( QListBoxItem *lbi )
{
    if ( lbi ) {
	ImageItem *item = (ImageItem *)lbi;
	if ( !item->filename().isEmpty() ) {
	    image = item->filename();
	    QTimer::singleShot( 0, this, SLOT(showImage()) );
	}
    } else {
	preview->setPixmap( QPixmap() );
    }
}

void WallpaperSelector::showImage()
{
    showImage( image );
}

void WallpaperSelector::showImage( const QString &file )
{
    if ( file.isEmpty() )
	return;
    QImage img( Resource::loadImage( "wallpaper/" + file ) );
    if ( img.isNull() )
	img.load( file );
    QPixmap pm;
    if ( !img.isNull() ) {
	double xr = double(preview->width()) / img.width();
	double yr = double(preview->height()) / img.height();
	int w = preview->width();
	int h = preview->height();
	if ( xr < yr )
	    h = img.height() * preview->width() / img.width();
	else
	    w = img.width() * preview->height() / img.height();
	img = img.smoothScale( w, h );
	pm.convertFromImage( img );
    }
    preview->setPixmap( pm );
}

