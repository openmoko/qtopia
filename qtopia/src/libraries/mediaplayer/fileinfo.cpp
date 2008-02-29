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
#include <qlabel.h>
#include <qheader.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qfileinfo.h>
#include <qvgroupbox.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qgrid.h>
#include "fileinfo.h"

#include <qtopia/mediaplayerplugininterface.h>
#include "mediaplayerstate.h"
#include "id3tag.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <qtopia/docproperties.h>
#include <qtopia/vscrollview.h>


/*!
  \class MediaFileInfoWidget fileinfo.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    It displays a property box specific for media files.
*/


class GeneralInfoView;


class ID3TagEditWidget : public QWidget {
    Q_OBJECT
public:
    ID3TagEditWidget( QWidget *parent, const DocLnk& lnk );
    void writeTagData();
private:
    bool tagDataChanged();
    void readInputFields();
    const DocLnk& fileInfo;
    ID3Tag inputTag;
    ID3Tag outputTag;
    QLineEdit *id3Title;
    QLineEdit *id3Artist;
    QSpinBox *id3Year;
    QLineEdit *id3Album;
    QSpinBox *id3Track;
    QLineEdit *id3Comment;
};


ID3TagEditWidget::ID3TagEditWidget( QWidget *parent, const DocLnk& lnk ) : QWidget( parent ), fileInfo( lnk )
{
    QGridLayout *gl = new QGridLayout( this, 5, 4, 6, 2 );
    
    gl->addWidget( new QLabel( tr("Title"), this ),   0, 0 );
    gl->addWidget( new QLabel( tr("Artist"), this ),  1, 0 );
    gl->addWidget( new QLabel( tr("Album"), this ),   2, 0 );
    gl->addWidget( new QLabel( tr("Year"), this ),    3, 0 ); 
    gl->addWidget( new QLabel( tr("Track"), this ),   3, 2 );
    gl->addWidget( new QLabel( tr("Misc"), this ),    4, 0 );

    int fd = ::open( fileInfo.file().local8Bit().data(), O_RDONLY );
    inputTag.read( fd );
    ::close( fd );

    gl->addMultiCellWidget( id3Title = new QLineEdit( inputTag.title(), this ), 0, 0, 1, 3 );
    gl->addMultiCellWidget( id3Artist = new QLineEdit( inputTag.artist(), this ), 1, 1, 1, 3 ); 
    gl->addMultiCellWidget( id3Album = new QLineEdit( inputTag.album(), this ),   2, 2, 1, 3 );
    gl->addWidget( id3Year = new QSpinBox( 0, 9999, 1, this ), 3, 1 );
    gl->addWidget( id3Track = new QSpinBox( 0, 255, 1, this ), 3, 3 );
    gl->addMultiCellWidget( id3Comment = new QLineEdit( inputTag.comment(), this ), 4, 4, 1, 3 );

    id3Year->setValue( inputTag.year() );
    id3Track->setValue( inputTag.track() );

    gl->setRowStretch(5,1);
}


void ID3TagEditWidget::readInputFields()
{
    // To my knowledge, ID3 tags are assumed to be latin1
    outputTag.setTitle( id3Title->text().latin1() );
    outputTag.setArtist( id3Artist->text().latin1() );
    outputTag.setYear( id3Year->value() );
    outputTag.setAlbum( id3Album->text().latin1() );
    outputTag.setTrack( id3Track->value() );
    outputTag.setComment( id3Comment->text().latin1() );
}


bool ID3TagEditWidget::tagDataChanged()
{
    readInputFields();
    if ( inputTag.title() != outputTag.title() || inputTag.artist()  != outputTag.artist() ||
	 inputTag.year()  != outputTag.year()  || inputTag.album()   != outputTag.album() ||
	 inputTag.track() != outputTag.track() || inputTag.comment() != outputTag.comment() )
	return TRUE;
    return FALSE;
}


void ID3TagEditWidget::writeTagData()
{
    if ( tagDataChanged() ) {
	int fd = ::open( fileInfo.file().latin1(), O_RDWR );
	outputTag.write( fd );
	::close( fd );
    }
}


class MediaFileInfoWidgetPrivate {
public:
    GeneralInfoView *prop; 
    ID3TagEditWidget *tagEdit;
    DocLnk docLnk;
};


class DetailedInfoView : public VScrollView {
    Q_OBJECT
public:
    DetailedInfoView( QWidget *parent, const DocLnk& fileInfo ) : VScrollView( parent ) {
	grid = new QGrid( 2, widget() );
	grid->setMargin( 3 );
	QFileInfo fi( fileInfo.file() );
	addEntryPair( tr("File"), fi.fileName() );
	addEntryPair( tr("Path"), fi.dirPath( true ) );
	addEntryPair( tr("Size"), QString::number( fi.size() ) + " " + tr("bytes") );
	addEntryPair( tr("Type"), fileInfo.type() );
	addEntryPair( tr("Date"), fi.lastModified().date().toString() );
	if ( mediaPlayerState->decoder( fileInfo.file() ) ) {
	    mediaPlayerState->decoder()->open( fileInfo.file() );
	    QStringList fileInfo = QStringList::split( ',', mediaPlayerState->decoder()->fileInfo() );
	    mediaPlayerState->decoder()->close();
	    QStringList::Iterator it;
	    for( it = fileInfo.begin(); it != fileInfo.end(); ++it ) {
		QString entry = (*it);
		QStringList entryPair = QStringList::split( ": ", entry );
		addEntryPair( entryPair[0]+" ", entryPair[1] );
	    }
	}
	QVBoxLayout *l = new QVBoxLayout( widget() );
	l->setMargin( 3 );
	l->addWidget( grid );
    }
    void addEntryPair(QString lab, QString val) {
	new QLabel( lab, grid );
	(new QLineEdit( val, grid ))->setReadOnly(TRUE);
    }

    QGrid *grid;
};


class GeneralInfoView : public VScrollView {
    Q_OBJECT
public:
    GeneralInfoView( QWidget *parent, DocLnk *fileInfo ) : VScrollView( parent ) {
	QVBox *vbox = new QVBox(widget());
	QHBox *title = new QHBox( vbox );
	title->setMargin( 3 );
	QLabel *icon = new QLabel( title );
	QLabel *filename = new QLabel( title );
	filename->setText( fileInfo->name() );
	icon->setPixmap( fileInfo->bigPixmap() );
	prop = new DocPropertiesWidget( fileInfo, vbox );
	QVBoxLayout *l = new QVBoxLayout( widget() );
	l->setMargin( 3 );
	l->addWidget( vbox );
    }
    void applyChanges() {
	prop->applyChanges();
    }
    DocPropertiesWidget *prop;
};

MediaFileInfoWidget::MediaFileInfoWidget( const DocLnk& fileInfo, QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    d = new MediaFileInfoWidgetPrivate;
    d->docLnk = DocLnk( fileInfo );

    setCaption( tr("File Information") );

    QTabWidget *tw = new QTabWidget( this );

    tw->addTab( d->prop = new GeneralInfoView( tw, &d->docLnk ), tr("General") );
    connect( d->prop->prop, SIGNAL( done() ), this, SLOT( reject() ) );

    QFileInfo fi( fileInfo.file() );
    if ( fi.extension( FALSE ).upper() == "MP3" ) 
	tw->addTab( d->tagEdit = new ID3TagEditWidget( tw, fileInfo), tr("ID3 Tags") );

    tw->addTab( new DetailedInfoView( tw, fileInfo ), tr("Details") );

    QVBoxLayout *l = new QVBoxLayout( this ); // Give it a layout
    l->addWidget( tw );
}


MediaFileInfoWidget::~MediaFileInfoWidget()
{
    delete d;
}


void MediaFileInfoWidget::accept()
{
    d->prop->applyChanges();
    
    QFileInfo fi( d->docLnk.file() );
    if ( fi.extension( FALSE ).upper() == "MP3" ) 
	d->tagEdit->writeTagData();

    QDialog::accept();    
}

#include "fileinfo.moc"
