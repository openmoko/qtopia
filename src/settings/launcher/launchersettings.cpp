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
#include "launchersettings.h"
#include "wallpaperselector.h"
#include "colorselector.h"
#include <qpe/qpeapplication.h>
#include <qpe/resource.h>
#include <qpe/applnk.h>
#include <qpe/mimetype.h>
#include <qpe/config.h>
#include <qpe/qcopenvelope_qws.h>
#include <qpe/fontdatabase.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qtabwidget.h>
#include <qwidgetlist.h>
#include <qobjectlist.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qheader.h>
#include <qlayout.h>

// Copied from QtMail editaccount.cpp DO NOT EDIT HERE - PUT IT IN A LIBRARY, I FORGOT TO
class AutoTabLayout : QObject {
    QWidget* group;
public:
    AutoTabLayout(QWidget* child, QWidget* parent) :
	QObject(parent),
	group(child)
    {
	group->installEventFilter(this);
    }

    bool eventFilter( QObject *obj, QEvent *e )
    {
	if ( obj == group ) {
	    if ( e->type() == QEvent::LayoutHint ) {
		// Only shrinks
		if ( group->sizeHint().width() > group->width()
		  || group->sizeHint().height() > group->height() )
		{
		    const QObjectList *list = group->children();
		    QWidgetList tabs;
		    if ( list ) {
			QObjectListIt it( *list );
			QObject *obj;
			while ( ( obj = it.current() ) ) {
			    ++it;
			    if ( obj->isWidgetType() )
				tabs.append((QWidget*)obj);
			}
		    }

		    if ( tabs.count() == 1 )
			// no point using tabs (and it's probable a tabwidget already)
			return FALSE;

		    QTabWidget* tw = new QTabWidget(group);
		    delete group->layout();
		    QVBoxLayout* vb = new QVBoxLayout(group);
		    vb->addWidget(tw);
		    QWidgetListIt it(tabs);
		    QWidget *t;
		    while ( ( t = it.current() ) ) {
			++it;
			if ( t != tw ) {
			    QString l;
			    if ( t->inherits("QGroupBox") ) {
				QGroupBox* g = (QGroupBox*)t;
				l = g->title();
				g->setFrameShape(QFrame::NoFrame);
				g->setTitle(QString::null);
			    } else {
				l = t->caption();
			    }
			    tw->addTab(t,l);
			}
		    }
		    group->topLevelWidget()->setMinimumSize(0,0);

		    // WHY CAN'T I GET IT WORKING WITHOUT THIS!!!!!
		    group->topLevelWidget()->showMaximized();

		    tw->show();
		}
	    }
	}
	return FALSE;
    }
};


LauncherSettings::LauncherSettings( QWidget *parent, const char *name )
    : LauncherSettingsBase( parent, name, TRUE, WStyle_ContextHelp )
{
    maxFontSize = qApp->desktop()->width() >= 640 ? 14 : 12;
    connect( categoryCombo, SIGNAL(activated(int)),
	     this, SLOT(categorySelected(int)) );
    connect( viewGroup, SIGNAL(clicked(int)), this, SLOT(setViewMode(int)) );
    connect( fontFamily, SIGNAL(activated(int)),
	     this, SLOT(fontFamilySelected(int)) );
    connect( fontSize, SIGNAL(activated(const QString&)),
	     this, SLOT(fontSizeSelected(const QString&)) );
    connect( backgroundCombo, SIGNAL(activated(int)),
	     this, SLOT(setBackgroundType(int)) );
    connect( selectButton, SIGNAL(clicked()), this, SLOT(selectWallpaper()) );
    connect( colorButton, SIGNAL(selected(const QColor &)), this, SLOT(selectColor(const QColor &)) );
    connect( textColorButton, SIGNAL(selected(const QColor &)), this, SLOT(textColor(const QColor &)) );
//    new AutoTabLayout(group,this);
    initialize();
}

void LauncherSettings::categorySelected( int i )
{
    showTabSettings( ids[i] );
}

void LauncherSettings::initialize()
{
    // Launcher Tab
    AppLnkSet rootFolder( MimeType::appsFolderName() );
    QStringList types = rootFolder.types();
    for ( QStringList::Iterator it=types.begin(); it!=types.end(); ++it) {
	categoryCombo->insertItem( rootFolder.typePixmap(*it),rootFolder.typeName(*it) );
	ids += *it;
    }
    QImage img( Resource::loadImage( "DocsIcon" ) );
    QPixmap pm;
    pm = img.smoothScale( AppLnk::smallIconSize(), AppLnk::smallIconSize() );
    categoryCombo->insertItem( pm, tr("Documents") );
    ids += "Documents"; // No tr

    fontFamily->insertItem( tr("[ Default Font ]") );
    FontDatabase fd;
    QStringList f = fd.families();
    for ( QStringList::ConstIterator it=f.begin(); it!=f.end(); ++it ) {
	QString n = *it;
	n[0] = n[0].upper();
	fontFamily->insertItem(n);
    }
    
    readTabSettings();
    showTabSettings( *ids.begin() );
}

void LauncherSettings::showTabSettings( const QString &id )
{
    currentTab = id;
    const TabSettings &ts = tabSettings[id];
    viewGroup->setButton( ts.view ? 0 : 1 );
    backgroundCombo->setCurrentItem( ts.bgType );
    colorButton->setDefaultColor( colorGroup().base() );
    if ( !ts.bgColor.isEmpty() )
	colorButton->setColor( ts.bgColor );
    textColorButton->setDefaultColor( colorGroup().text() );
    if ( !ts.textColor.isEmpty() )
	textColorButton->setColor( ts.textColor );
    selectButton->setEnabled( ts.bgType == Image );
    colorButton->setEnabled( ts.bgType == SolidColor );
    if ( ts.customFont ) {
	for ( int i = 1; i < fontFamily->count(); i++ ) {
	    if ( ts.fontFamily.lower() == fontFamily->text(i).lower() ) {
		fontFamily->setCurrentItem( i );
		break;
	    }
	}
	populateFontSizes( ts.fontFamily );
	fontSize->setEnabled( TRUE );
    } else {
	fontFamily->setCurrentItem( 0 );
	fontSize->setEnabled( FALSE );
    }
}

void LauncherSettings::setViewMode( int m )
{
    TabSettings &ts = tabSettings[currentTab];
    ts.view = m ? Icon : List;
    ts.changed = TRUE;
}

void LauncherSettings::fontFamilySelected( int idx )
{
    TabSettings &ts = tabSettings[currentTab];
    if ( idx == 0 ) {
	ts.customFont = FALSE;
	fontSize->setEnabled( FALSE );
    } else {
	ts.customFont = TRUE;
	QString name = fontFamily->text( idx );
	populateFontSizes( name );
	ts.fontFamily = name;
	ts.fontSize = fontSize->currentText().toInt();
	fontSize->setEnabled( TRUE );
    }
    ts.changed = TRUE;
}

void LauncherSettings::fontSizeSelected( const QString &sz )
{
    TabSettings &ts = tabSettings[currentTab];
    ts.fontSize = sz.toInt();
    ts.changed = TRUE;
}

void LauncherSettings::setBackgroundType( int t )
{
    TabSettings &ts = tabSettings[currentTab];
    ts.bgType = (BackgroundType)t;
    ts.changed = TRUE;
    selectButton->setEnabled( ts.bgType == Image );
    colorButton->setEnabled( ts.bgType == SolidColor );
}

void LauncherSettings::selectWallpaper()
{
    TabSettings &ts = tabSettings[currentTab];
    WallpaperSelector base( ts.bgImage, this );
    base.showMaximized();
    base.exec();
    ts.bgImage = base.filename();
    ts.changed = TRUE;
}

void LauncherSettings::selectColor( const QColor &c )
{
    TabSettings &ts = tabSettings[currentTab];
    if ( c == colorGroup().base() )
	ts.bgColor = QString::null;
    else
	ts.bgColor = c.name();
    ts.changed = TRUE;
}

void LauncherSettings::textColor( const QColor &c )
{
    TabSettings &ts = tabSettings[currentTab];
    ts.textColor = c.name();
    ts.changed = TRUE;
}

void LauncherSettings::readTabSettings()
{
    Config cfg("Launcher");
    QString grp( "Tab %1" ); // No tr
    tabSettings.clear();
    for ( QStringList::Iterator it=ids.begin(); it!=ids.end(); ++it) {
	TabSettings ts;
	ts.view = Icon;
	ts.bgType = Ruled;
	ts.changed = FALSE;
	cfg.setGroup( grp.arg( *it ) );
	QString view = cfg.readEntry( "View", "Icon" );
	if ( view == "List" ) // No tr
	    ts.view = List;
	QString bgType = cfg.readEntry( "BackgroundType", "Ruled" );
	if ( bgType == "SolidColor" )
	    ts.bgType = SolidColor;
	else if ( bgType == "Image" ) // No tr
	    ts.bgType = Image;
	ts.bgImage = cfg.readEntry( "BackgroundImage", "wallpaper/marble" );
	ts.bgColor = cfg.readEntry( "BackgroundColor" );
	ts.textColor = cfg.readEntry( "TextColor" );
	ts.customFont = cfg.readBoolEntry( "CustomFont", FALSE );
	QStringList f = cfg.readListEntry( "Font", ',' );
	if ( f.count() == 4 ) {
	    ts.fontFamily = f[0];
	    ts.fontSize = f[1].toInt();
	} else {
	    ts.fontFamily = font().family();
	    ts.fontSize = font().pointSize();
	}
	tabSettings[*it] = ts;
    }
}

void LauncherSettings::accept()
{
    Config cfg("Launcher");

    // Launcher Tab
    QString grp( "Tab %1" ); // No tr
    for ( QStringList::Iterator it=ids.begin(); it!=ids.end(); ++it) {
	QString tab( *it );
	cfg.setGroup( grp.arg( tab ) );
	TabSettings &ts = tabSettings[tab];
	if ( !ts.changed )
	    continue;
	switch ( ts.view ) {
	    case Icon:
		cfg.writeEntry( "View", "Icon" );
		break;
	    case List:
		cfg.writeEntry( "View", "List" );
		break;
	}
	QCopEnvelope e( "QPE/Launcher", "setTabView(QString,int)" );
	e << tab << ts.view;

	cfg.writeEntry( "BackgroundImage", ts.bgImage );
	cfg.writeEntry( "BackgroundColor", ts.bgColor );
	cfg.writeEntry( "TextColor", ts.textColor );
	cfg.writeEntry( "CustomFont", ts.customFont );
	QString f = ts.fontFamily+","+
		    QString::number(ts.fontSize)+",50,0";
	cfg.writeEntry( "Font", f );
	QCopEnvelope be( "QPE/Launcher", "setTabBackground(QString,int,QString)" );
	switch ( ts.bgType ) {
	    case Ruled:
		cfg.writeEntry( "BackgroundType", "Ruled" );
		be << tab << ts.bgType << QString("");
		break;
	    case SolidColor:
		cfg.writeEntry( "BackgroundType", "SolidColor" );
		be << tab << ts.bgType << ts.bgColor;
		break;
	    case Image:
		cfg.writeEntry( "BackgroundType", "Image" );
		be << tab << ts.bgType << ts.bgImage;
		break;
	}
	QCopEnvelope te( "QPE/Launcher", "setTextColor(QString,QString)" );
	te << tab << ts.textColor;
	if ( ts.customFont ) {
	    QCopEnvelope fe( "QPE/Launcher", "setFont(QString,QString,int,int,int)" );
	    fe << tab << ts.fontFamily;
	    fe << ts.fontSize;
	    fe << 50 << 0;
	} else {
	    QCopEnvelope fe( "QPE/Launcher", "setFont(QString,QString,int,int,int)" );
	    fe << tab << QString::null;
	    fe << 0;
	    fe << 0 << 0;
	}
	ts.changed = FALSE;
    }

    QDialog::accept();
}

void LauncherSettings::done(int r)
{
    QDialog::done(r);
    close();
}

void LauncherSettings::populateFontSizes( const QString &name )
{
    int diff = 1000;
    fontSize->clear();
    FontDatabase fd;
    TabSettings &ts = tabSettings[currentTab];
    QValueList<int> pointSizes = fd.pointSizes( name.lower() );
    QValueList<int>::Iterator it;
    for ( it = pointSizes.begin(); it != pointSizes.end(); ++it ) {
	if ( *it <= maxFontSize ) {
	    fontSize->insertItem( QString::number( *it ) );
	    if ( QABS(*it-ts.fontSize) < diff ) {
		diff = QABS(*it - ts.fontSize);
		fontSize->setCurrentItem( fontSize->count()-1 );
	    }
	}
    }
}

