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
#include "fileinfo.h"

#include <qtopia/mediaplayerplugininterface.h>
#include "mediaplayerstate.h"
#include "id3tag.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <qtopia/docproperties.h>


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
    QGridLayout *gl = new QGridLayout( this, 4, 4 );
    gl->setSpacing( 3 );
    
    gl->addWidget( new QLabel( tr("Title"), this ),   0, 0 );
    gl->addWidget( new QLabel( tr("Artist"), this ),  1, 0 );
    gl->addWidget( new QLabel( tr("Album"), this ),   2, 0 );
    gl->addWidget( new QLabel( tr("Comment"), this ), 3, 0 );
    gl->addWidget( new QLabel( tr("Year"), this ),    1, 2 ); 
    gl->addWidget( new QLabel( tr("Track"), this ),   2, 2 );

    int fd = ::open( fileInfo.file().latin1(), O_RDONLY );
    inputTag.read( fd );
    ::close( fd );

    gl->addMultiCellWidget( id3Title = new QLineEdit( inputTag.title(), this ), 0, 0, 1, 3 );
    gl->addWidget( id3Artist = new QLineEdit( inputTag.artist(), this ), 1, 1 ); 
    gl->addWidget( id3Album = new QLineEdit( inputTag.album(), this ),   2, 1 );
    gl->addMultiCellWidget( id3Comment = new QLineEdit( inputTag.comment(), this ), 3, 3, 1, 3 );
    gl->addWidget( id3Year = new QSpinBox( 0, 9999, 1, this ), 1, 3 );
    gl->addWidget( id3Track = new QSpinBox( 0, 255, 1, this ), 2, 3 );

    id3Year->setValue( inputTag.year() );
    id3Track->setValue( inputTag.track() );
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
    DocPropertiesWidget *prop; 
	ID3TagEditWidget *tagEdit;
	DocLnk docLnk;
};


class InfoListView : public QListView {
    Q_OBJECT

public:
	InfoListView( QWidget *parent ) : QListView( parent ) {
	    setSorting( -1 );
	    addColumn( "C1" ); // No tr
	    addColumn( "C2" ); // No tr
	    header()->hide();
	}
	void setDocLnk( const DocLnk& fileInfo ) {
	    QFileInfo fi( fileInfo.file() );
	    if ( mediaPlayerState->decoder( fileInfo.file() ) ) {
		mediaPlayerState->decoder()->open( fileInfo.file() );
		QStringList fileInfo = QStringList::split( ',', mediaPlayerState->decoder()->fileInfo() );
		mediaPlayerState->decoder()->close();
		QStringList::Iterator it;
	        for( it = fileInfo.end(); it != fileInfo.begin(); ) {
		    --it;
		    QString entry = (*it);
		    QStringList entryPair = QStringList::split( ": ", entry );
		    new QListViewItem( this, entryPair[0] + " :  ", entryPair[1] );
		}
	    }
	    new QListViewItem( this, tr("Modified") + " :  ", fi.lastModified().toString() );
	    new QListViewItem( this, tr("Media Type") + " :  ", fileInfo.type() );
	    new QListViewItem( this, tr("File Size") + " :  ", QString::number( fi.size() ) + tr(" bytes") );
	    new QListViewItem( this, tr("File Path") + " :  ", fi.dirPath( TRUE ) );
	    new QListViewItem( this, tr("File Name") + " :  ", fi.fileName() );
	}
};


MediaFileInfoWidget::MediaFileInfoWidget( const DocLnk& fileInfo, QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    d = new MediaFileInfoWidgetPrivate;

    setCaption( tr("Media File Information") );

    QVBox *vb = new QVBox( this );
    QBoxLayout *l = new QVBoxLayout( this ); // Give it a layout
    l->addWidget( vb );
    QTabWidget *tw = new QTabWidget( vb );

    d->docLnk = DocLnk( fileInfo );
    d->prop = new DocPropertiesWidget( &d->docLnk, tw );
    connect( d->prop, SIGNAL(done()), this, SLOT(reject()) );
    
    tw->addTab( d->prop, "Categories" );

    QVBox *vbox = new QVBox( tw );
    vbox->setMargin( 3 );
    tw->addTab( vbox, "Details" );
    QHBox *title = new QHBox( vbox );

    QLabel *filename = new QLabel( title );
    QLabel *spacing = new QLabel( title );
    QLabel *icon = new QLabel( title );

    filename->setText( " " + fileInfo.name() );
    title->setStretchFactor( spacing, 1 );
    icon->setPixmap( fileInfo.bigPixmap() );

    // File info fields
    InfoListView *lv = new InfoListView( vbox );
    lv->setDocLnk( fileInfo );

    QFileInfo fi( fileInfo.file() );
    if ( fi.extension( FALSE ).upper() == "MP3" ) {
	QVGroupBox *id3EditView1 = new QVGroupBox( "MP3 ID3 Tags", vbox );
	d->tagEdit = new ID3TagEditWidget( id3EditView1, fileInfo );
    }
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
