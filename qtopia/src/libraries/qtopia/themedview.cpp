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

#include <qtopia/qpeapplication.h>
#include <qtopia/private/themedview_p.h>
#include <qtopia/private/themedviewinterface_p.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>
#ifndef QTOPIA_DESKTOP
#include <qtopia/pluginloader.h>
#endif
#include <qtopia/global.h>
#include <qtopia/image.h>
#include <qxml.h>
#include <qfile.h>
#include <qpainter.h>
#include <qstack.h>
#include <qsimplerichtext.h>
#include <qstylesheet.h>
#include <qstringlist.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qfileinfo.h>
#include <qdatastream.h>

#include <stdlib.h>

static struct {
    const char *name;
    QColorGroup::ColorRole role;
} colorTable[] = {
    { "Foreground", QColorGroup::Foreground },
    { "Button", QColorGroup::Button },
    { "Light", QColorGroup::Light },
    { "Midlight", QColorGroup::Midlight },
    { "Dark", QColorGroup::Dark },
    { "Mid", QColorGroup::Mid },
    { "Text", QColorGroup::Text },
    { "BrightText", QColorGroup::BrightText },
    { "ButtonText", QColorGroup::ButtonText },
    { "Base", QColorGroup::Base },
    { "Background", QColorGroup::Background },
    { "Shadow", QColorGroup::Shadow },
    { "Highlight", QColorGroup::Highlight },
    { "HighlightedText", QColorGroup::HighlightedText },
    { "None", QColorGroup::NColorRoles },
    { 0, QColorGroup::Foreground }
};

// Qtopia changes the palette after startup by setting the background pixmap
// to be used with brushes for Base and Background color roles. See PhoneLauncher::updateBackground
// ThemeCache only needs to be based on palette colors, so only use this 
// information when generating caches so they don't get needlessly invalidated
// QPalette::operator== compares brushes
bool ThemeCache::paletteColorsEqual( const QPalette &a, const QPalette &b ) const
{
    QValueList<QColorGroup> agroups;
    QValueList<QColorGroup> bgroups;
    agroups.append( a.active() );
    agroups.append( a.inactive() );
    agroups.append( a.disabled() );
    bgroups.append( b.active() );
    bgroups.append( b.inactive() );
    bgroups.append( b.disabled() );
    for( int i = 0 ; i < 3 ; ++i ) {
	const QColorGroup &ag = agroups[i],
			  &bg = bgroups[i];
	int j = 0;
	while (colorTable[j].role != QColorGroup::NColorRoles) {
	    if ( ag.color( colorTable[j].role ) != 
		    bg.color( colorTable[j].role ) ) {
		return FALSE;
	    }
	    j++;
	}
    }
    return TRUE;
}

/* This number should be incremented each time any changes are made to the
   Theme*::readData() and Theme*::writeData() functions */
#define THEME_CACHE_VERSION 4

QString itemNameFromRtti( int rtti )
{
    QString in = "Unknown";
    switch( rtti ) {
		case ThemedView::Page: 
		    in = "Page";
		    break;
		case ThemedView::Animation: 
		    in = "Animation";
		    break;
		case ThemedView::Level: 
		    in = "Level";
		    break;
		case ThemedView::Status: 
		    in = "Status";
		    break;
		case ThemedView::Image: 
		    in = "Image";
		    break;
		case ThemedView::Text: 
		    in = "Text";
		    break;
		case ThemedView::Rect:
		    in = "Rect";
		    break;
		case ThemedView::Line: 
		    in = "Line";
		    break;
#ifndef QTOPIA_DESKTOP
		case ThemedView::Plugin: 
		    in = "Plugin";
		    break;
#endif
		case ThemedView::Exclusive: 
		    in = "Exclusive";
		    break;
		case ThemedView::Layout: 
		    in = "Layout";
		    break;
		case ThemedView::Group: 
		    in = "Group";
		    break;
		case ThemedView::Input: 
		    in = "Input";
		    break;
		case ThemedView::ListItem:
		    in = "ListItem";
		    break;
    }
    return in;
}

class ThemedViewPrivate : public QXmlDefaultHandler
{
public:
    ThemedViewPrivate(ThemedView *v);
    ~ThemedViewPrivate();

    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool characters(const QString &ch);
    bool error(const QXmlParseException &exception);
    bool fatalError(const QXmlParseException &exception);

    ThemeItem *item(const QString &name);

    void paletteChange(ThemeItem *item, const QPalette &p);

protected:
    bool filter( const QXmlAttributes &atts );

private:
    ThemePageItem *page;
    QList<ThemeItem> items;
    QStack<ThemeItem> parseStack;
    ThemedView *view;
    int ignore;
    bool needLayout;
    bool parseDone;

    friend class ThemedView;
    friend class ThemeCache;
};

ThemeCache::ThemeCache()
    : m_cacheStream( 0 ), m_loaded( false )
#ifdef DEBUG
    , m_paletteBeenSet( false )
#endif
{
}

void ThemeCache::setPalette( const QPalette &p )
{
    if( loaded() &&  // loaded a cache already and
	!paletteColorsEqual( p, m_palette ) // its palette is different 
	)
    {
	// palette change is logically associated with Theme change in Qtopia
	// theme change will come after palette change, so delete the cache 
	// now so it wont be used.
	remove();
    }
    // else check and decide what to do during cache load

    m_palette = p;
#ifdef DEBUG
    m_paletteBeenSet = true;
#endif
}

const QString ThemeCache::baseDir() const
{
    Config config( "qpe" );
    config.setGroup( "Appearance" );
    char *homeDir = getenv( "HOME" );
    QString dir;
    if( homeDir )
       dir = QString( homeDir );
    dir = config.readEntry( "ThemeCacheDir", dir ) + "/";
    dir = dir.stripWhiteSpace();
    if( dir[dir.length()-1] != '/' )
	dir += '/';
    return dir;
}

void ThemeCache::setThemeFile( const QString &themeFile )
{
    if( themeFile.isNull() ) {
	//qDebug("ThemeCache::setName( null )");
	return;
    }

    m_themeFile = themeFile;

    QFileInfo fi( m_themeFile );
    QString newFileName = baseDir() + ".qtopia_theme_cache_" + fi.baseName();
    if( newFileName != m_cacheFileName ) {
	if( QFile::exists( m_cacheFileName ) )
	    QFile::remove( m_cacheFileName );
	m_cacheFileName = newFileName;
    }
}

bool ThemeCache::load( ThemedViewPrivate *data )
{
    if( data->page ) {
	qDebug("BUG: ThemeCache::load called with existing page");
	return FALSE;
    }
    if( m_cacheFileName.isNull() ) {
	qDebug("BUG: ThemeCache::save called with no cache name");
	return FALSE;
    }

#ifdef DEBUG
    qDebug("Trying to load ThemeCache %s", m_cacheFileName.latin1());
#endif
    m_cacheFile.setName( m_cacheFileName );
    if( !m_cacheFile.open( IO_ReadOnly ) ) {
	return FALSE;
    }
    //qDebug("loading theme cache %s",m_cacheFileName.latin1());
    m_cacheStream = new QDataStream( &m_cacheFile );

    data->page = loadHelper( data );

    m_cacheFile.close();

    delete m_cacheStream;
    m_cacheStream = 0;

#ifdef DEBUG
    if( data->page )
	qDebug("Loaded ThemeCache %s", m_cacheFileName.latin1());
#endif

    if( data->page == 0 ) {
	remove(); // invalid theme cache for this theme
#ifdef DEBUG
	qDebug("ThemeCache::load - Cache invalid, removing..");
#endif
	return FALSE;
    }	


    m_loaded = true;

    return true;
}

bool ThemeCache::loaded() const 
{ 
    return m_loaded; 
}

ThemePageItem *ThemeCache::loadHelper( ThemedViewPrivate *data )
{
    //qDebug("ThemeCache::loadHelper");

    // read header info
    int cacheVersion = -1;
    QString themeFile;
    QPalette palette;
    *m_cacheStream >> themeFile >> cacheVersion >> palette;
    int mousePref;
    *m_cacheStream >> mousePref;
    if( themeFile != m_themeFile ) {
#ifdef DEBUG
	qDebug("Not loading cache for unmatching theme file");
#endif
	return 0;
    }
    if( cacheVersion != THEME_CACHE_VERSION ) {
#ifdef DEBUG
	qDebug("Ignoring cache files with unmatching version");
#endif
	return 0;
    }
    if( !paletteColorsEqual( palette, m_palette ) ) {
#ifdef  DEBUG
	qDebug("Not loading cache that was generated with wrong palette");
#endif
	return 0;
    }
    if( mousePref != Global::mousePreferred() ) {
#ifdef  DEBUG
	qDebug("Not loading cache with unmatching input preference");
#endif
	return 0;
    }

    ThemePageItem *page = 0;
    ThemedView *view = data->view;

    ThemeItem *item = 0;
    ThemeItem *top = 0;
    while( !m_cacheStream->atEnd() ) {
	int rtti = 0;
	*m_cacheStream >> rtti;
	if( rtti < 0 ) { // item end marker
	    if( !top ) {
		qFatal("parse error reading theme cache");
		return 0;
	    }
	    //qDebug("read ***** end '%s' item", itemNameFromRtti(top->rtti()).latin1());
	    data->parseStack.pop();
	    top = data->parseStack.top();
	} else {
	    //qDebug("read ***** begin '%s' item", itemNameFromRtti(rtti).latin1());
	    switch( rtti ) {
		case ThemedView::Page: 
		    item = new ThemePageItem(top, view);
		    page = (ThemePageItem *)item;
		    break;
		case ThemedView::Animation: 
		    item = new ThemeAnimationItem(top, view);
		    break;
		case ThemedView::Level: 
		    item = new ThemeLevelItem(top, view);
		    break;
		case ThemedView::Status: 
		    item = new ThemeStatusItem(top, view);
		    break;
		case ThemedView::Image: 
		    item = new ThemeImageItem(top, view);
		    break;
		case ThemedView::Text: 
		    item = new ThemeTextItem(top, view);
		    break;
		case ThemedView::Rect:
		    item = new ThemeRectItem(top, view);
		    break;
		case ThemedView::Line: 
		    item = new ThemeLineItem(top, view);
		    break;
#ifndef QTOPIA_DESKTOP
		case ThemedView::Plugin: 
		    item = new ThemePluginItem(top, view);
		    break;
#endif
		case ThemedView::Exclusive: 
		    item = new ThemeExclusiveItem(top, view);
		    break;
		case ThemedView::Layout: 
		    item = new ThemeLayoutItem(top, view);
		    break;
		case ThemedView::Group: 
		    item = new ThemeGroupItem(top, view);
		    break;
		case ThemedView::Input: 
		    item = new ThemeInputItem(top, view);
		    break;
		case ThemedView::ListItem:
		    item = new ThemeListItem(top, view);
		    break;
	    }
	}
	if( item ) {
	    item->readData( *m_cacheStream );
	    if( top )
		top->chldn.append( item );
	    else if( data->parseStack.count() != 0 )
	    {
		qDebug("BUG: ThemeCache::load count != 0");
		return 0;
	    }
	    data->parseStack.push( item );
	    top = item;
	}
	item = 0;
    }
    data->parseStack.clear();
    return page;
}

bool ThemeCache::save( ThemedViewPrivate *data )
{
    if( !data->page ) {
	qDebug("BUG: ThemeCache::save called on null page");
	return FALSE;
    }
    if( m_cacheFileName.isNull() ) {
	qDebug("BUG: ThemeCache::save called with no cache name");
	return FALSE;
    }

    m_cacheFile.setName( m_cacheFileName );
    if( !m_cacheFile.open( IO_WriteOnly ) ) {
	qWarning("Unable to save ThemeCache");
	return FALSE;
    }
    m_cacheStream = new QDataStream( &m_cacheFile );

    //qDebug("saving cache to %s", m_cacheFileName.latin1());
#ifdef DEBUG
    if( !m_paletteBeenSet )
	qDebug("ThemeCache::save called when no palette has been set");
#endif
    *m_cacheStream << m_themeFile << THEME_CACHE_VERSION << m_palette; // save header info
    *m_cacheStream << (int)Global::mousePreferred();

    save( data->page ); // save items

    m_cacheFile.close();
    delete m_cacheStream;
    m_cacheStream = 0;

    return TRUE;
}

void ThemeCache::save( ThemeItem *item )
{
    //qDebug("write ***** begin '%s' item", itemNameFromRtti(item->rtti()).latin1());
    *m_cacheStream << item->rtti();
    item->writeData( *m_cacheStream );
    QListIterator<ThemeItem> it(item->chldn);
    while (it.current()) {
	save( it.current() );
	++it;
    }
    *m_cacheStream << -1;
    //qDebug("write ***** end '%s' item", itemNameFromRtti(item->rtti()).latin1());
}

void ThemeCache::remove()
{
    if( m_cacheFileName.isNull() )
	return;
    if( m_cacheFileName == m_cacheFile.name() && m_cacheFile.isOpen() )
	m_cacheFile.close();
    if( QFile::exists( m_cacheFileName ) ) {
	QFile::remove( m_cacheFileName );
#ifdef DEBUG
	qDebug("ThemeCache::remove() removed %s", m_cacheFileName.latin1());
#endif
    }
    m_loaded = false;
}

ThemeCache::~ThemeCache()
{
}


ThemedViewPrivate::ThemedViewPrivate(ThemedView *v)
    : page(0), view(v), ignore(0), needLayout(FALSE), parseDone(FALSE)
{
}

ThemedViewPrivate::~ThemedViewPrivate()
{
    delete page;
}

bool ThemedViewPrivate::startElement(const QString &, const QString &, const QString &qName, const QXmlAttributes &atts)
{
//    qDebug("%*s Start element: %s", parseStack.count()*2, "", qName.latin1());
    if( filter( atts ) ) {
	if (qName == "page") {
	    page = new ThemePageItem(view, atts);
	    parseStack.push(page);
	} else if (page && !ignore) {
	    ThemeItem *top = parseStack.top();
	    ThemeItem *item = 0;
	    if (qName == "anim") {
		item = new ThemeAnimationItem(top, view, atts);
	    } else if (qName == "level") {
		item = new ThemeLevelItem(top, view, atts);
	    } else if (qName == "status") {
		item = new ThemeStatusItem(top, view, atts);
	    } else if (qName == "image") {
		item = new ThemeImageItem(top, view, atts);
	    } else if (qName == "text") {
		item = new ThemeTextItem(top, view, atts);
	    } else if (qName == "rect") {
		item = new ThemeRectItem(top, view, atts);
	    } else if (qName == "line") {
		item = new ThemeLineItem(top, view, atts);
#ifndef QTOPIA_DESKTOP
	    } else if (qName == "plugin") {
		item = new ThemePluginItem(top, view, atts);
#endif
	    } else if (qName == "exclusive") {
		item = new ThemeExclusiveItem(top, view, atts);
	    } else if (qName == "layout") {
		item = new ThemeLayoutItem(top, view, atts);
	    } else if(qName == "group") {
		item = new ThemeGroupItem(top, view, atts);
	    } else if(qName == "input") {
		item = new ThemeInputItem(top, view, atts);
	    } else if(qName == "listitem") {
		item = new ThemeListItem(top,view,atts);
	    }
	    if (item && top) {
		parseStack.top()->chldn.append(item);
		parseStack.push(item);
	    } else {
		//invalid item, ignore
		ignore++;
	    }
	} else if( page ) {
	    //ignoring from previous
	    ++ignore;
	}
    } else {
		//doesn't pass filter, ignore 
	++ignore;
    }

    return TRUE;
}

bool ThemedViewPrivate::endElement(const QString &, const QString &, const QString &)
{
    if (ignore) {
	ignore--;
    } else {
	if (parseStack.top() != page)
	    parseStack.pop();
	else
	    parseDone = TRUE;
    }
    return TRUE;
}

bool ThemedViewPrivate::characters(const QString &ch)
{
    if (!parseDone && !parseStack.isEmpty())
	parseStack.top()->addCharacters(ch);
    return TRUE;
}

bool ThemedViewPrivate::filter( const QXmlAttributes &atts )
{
    bool keypad = atts.value( "keypad" ) != "no";
    bool touchscreen = atts.value( "touchscreen" ) != "no";
    if( !keypad && !Global::mousePreferred() )
	return FALSE;
    if( !touchscreen && Global::mousePreferred() )
	return FALSE;
    return TRUE;
}

bool ThemedViewPrivate::error(const QXmlParseException &exception)
{
    qWarning("%s (line %d)", exception.message().latin1(), exception.lineNumber());
    return TRUE;
}

bool ThemedViewPrivate::fatalError(const QXmlParseException &exception)
{
    qWarning("%s (line %d)", exception.message().latin1(), exception.lineNumber());
    return TRUE;
}

void ThemedViewPrivate::paletteChange(ThemeItem *item, const QPalette &p)
{
    item->paletteChange(p);
    QListIterator<ThemeItem> it(item->chldn);
    while (it.current()) {
	paletteChange(it.current(), p);
	++it;
    }
}

//---------------------------------------------------------------------------

static int stateToIndex( int st )
{
    switch( st ) {
	case ThemeItem::Default:
	    return 0;
	case ThemeItem::Focus:
	    return 1;
	case ThemeItem::Pressed:
	    return 2;
	default:
	    qDebug("BUG : stateToIndex passed invalid st %d", st);
	    return -1;
    }
}

static ThemeItem::State indexToState( int idx )
{
    switch( idx ) {
	case 0:
	    return ThemeItem::Default;
	case 1:
	    return ThemeItem::Focus;
	case 2:
	    return ThemeItem::Pressed;
	default:
	    qDebug("BUG : indexToState passed invalid index %d", idx);
	    return ThemeItem::Default; 
    }
}

ThemeItem::ThemeItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : parent(p), view(v)
{
    init();

    name = atts.value("name");
    sr = parseRect(atts, rmode);
    transient = atts.value("transient") == "yes";
    actv = atts.value("active") != "no";
    if( atts.value("interactive").isNull() && parent )
	interactive = parent->isInteractive();
    else
	interactive = atts.value("interactive") == "yes";
}

ThemeItem::~ThemeItem()
{
}

void ThemeItem::init()
{
    rmode = Rect; actv = TRUE; visible = TRUE; press = FALSE; focus = FALSE; interactive = FALSE;
    chldn.setAutoDelete(TRUE);
}

void ThemeItem::setAttribute( const QString &key, const int &val, int st )
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeItem::setAttribute() - Invalid state '%d'", st );
	return;
    }
    QMap<QString,int> &dataset = intattributes[st];
    if( dataset.contains( key ) )
	dataset.remove( key );
    dataset.insert( key, val );
}

int ThemeItem::attribute( const QString &key, int st ) const
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeItem::attribute() - Invalid state '%d'", st );
	return -1;
    }
    const QMap<QString,int> &dataset = intattributes[st];
    return dataset[key];
}

void ThemeItem::setAttribute( const QString &key, const QString &val, int st )
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeItem::attribute() - Invalid state '%d'", st );
	return;
    }
    QMap<QString,QString> &dataset = strattributes[st];
    if( dataset.contains( key ) )
	dataset.remove( key );
    dataset.insert( key, val );
}

QString ThemeItem::strAttribute( const QString &key, int st )
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeItem::strAttribute() - Invalid state '%d'", st );
	return QString::null;
    }
    const QMap<QString,QString> &dataset = strattributes[st];
    return dataset[key];
}

void ThemeItem::paletteChange(const QPalette &)
{
}

int ThemeItem::state() const
{
    if( pressed() ) {
	return ThemeItem::Pressed;
    } else if( hasFocus() ) {
	return ThemeItem::Focus;
    } else {
	return ThemeItem::Default;
    }
}

void ThemeItem::setFocus( bool f )
{
    if( !isInteractive() )
	return;

    if( f != focus )
    {
	if( active() && (rtti() != ThemedView::Status || ((ThemeStatusItem *)this)->enabled()) )
	{
	    focus = f;
	    if( !focus )
		press = FALSE;
	    if( isVisible() )
		update();
	}
    }
}

void ThemeItem::setPressed( bool p )
{
    if( !isInteractive() )
	return;

    if( p != press ) // only allow presses if enabled
    {
	if( active() && (rtti() != ThemedView::Status || ((ThemeStatusItem *)this)->enabled()) )
	{
	    press = p;
	    if( press )
		focus = TRUE;
	    if( isVisible() )
		update();
	}
    }
}

QString ThemeItem::itemName() const
{
    return name;
}

QRect ThemeItem::rect() const
{
    int x = br.x();
    int y = br.y();
    ThemeItem *item = parent;
    while (item) {
	x += item->br.x();
	y += item->br.y();
	item = item->parent;
    }

    return QRect(x, y, br.width(), br.height());
}

QRect ThemeItem::geometry() const
{
    return br;
}

QRect ThemeItem::geometryHint()const
{
    return sr;
}

void ThemeItem::setGeometry(const QRect& b)
{
    br = b;
}

void ThemeItem::setActive(bool a)
{
    if (a != actv) {
	actv = a;
	if( !actv )
	    press = FALSE; //can't be pressed when not active
	view->layout();
	update();
    }
}

bool ThemeItem::isVisible() const
{
    if ((!transient || actv) && visible) {
	if (parent)
	    return parent->isVisible();
	else
	    return TRUE;
    }

    return FALSE;
}

void ThemeItem::layout()
{
    br = sr;
    if (rmode == Coords) {
	if (sr.right() <= 0)
	    br.setRight(parent->br.width() + sr.right());
	if (sr.left() < 0)
	    br.setLeft(parent->br.width() + sr.left());
    } else if (!sr.width()) {
	br.setWidth(parent->br.width());
    } else if (sr.left() < 0) {
	br.setX(parent->br.width()+sr.x());
	br.setWidth(sr.width());
    }
    if (rmode == Coords) {
	if (sr.top() < 0)
	    br.setTop(parent->br.height() + sr.top());
	if (sr.bottom() <= 0)
	    br.setBottom(parent->br.height() + sr.bottom());
    } else if (!sr.height()) {
	br.setHeight(parent->br.height());
    } else if (sr.top() < 0) {
	br.setY(parent->br.height()+sr.y());
	br.setHeight(sr.height());
    }
}

void ThemeItem::update()
{
    update(br.x(), br.y(), br.width(), br.height());
}

void ThemeItem::update(int x, int y, int w, int h)
{
    ThemeItem *item = parent;
    while (item) {
	x += item->br.x();
	y += item->br.y();
	item = item->parent;
    }

#if 1
    view->update(x, y, w, h);
#else
    qDebug("repainting: %d, %d, %d x %d", x, y, w, h);
    view->repaint(x, y, w, h, FALSE);
    qDebug("done repaint");
#endif
}

int ThemeItem::rtti() const
{
    return ThemedView::Item;
}

void ThemeItem::paint(QPainter *, const QRect &)
{
}

QRect ThemeItem::parseRect(const QXmlAttributes &atts, RMode &mode, const QString &name)
{
    mode = Rect;
    QString rn(name.isEmpty() ? QString("rect") : name);
    QString val = atts.value(rn);
    QRect rect;
    if (!val.isEmpty()) {
	QStringList args = QStringList::split(',', atts.value(rn));
	if (args.count() == 4) {
	    mode = Coords;
	    rect.setCoords(args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt());
	} else if (args.count() == 3 && args[2].contains('x')) {
	    QStringList dimargs = QStringList::split('x', args[2]);
	    rect.setRect(args[0].toInt(), args[1].toInt(), dimargs[0].toInt(), dimargs[1].toInt());
	}
    } else {
	// No rect means use all available space
	rect.setRect(0, 0, 0, 0);
    }

    return rect;
}

QMap<QString,QString> ThemeItem::parseSubAtts( const QString &subatts ) const
{
    QMap<QString,QString> subAtts;
    if( subatts.stripWhiteSpace().isEmpty() )
	return subAtts;
    QStringList assignments = QStringList::split( ";", subatts );
    for( uint i = 0 ; i < assignments.count() ; ++i )
    {
	QStringList assignment = QStringList::split( "=", assignments[(int)i] );
	if( assignment.count() == 2 )
	    subAtts.insert( assignment[0], assignment[1] );
    }
    return subAtts;
}

int ThemeItem::parseAlignment(const QXmlAttributes &atts, const QString &name, int defAlign)
{
    int align = defAlign;

    QString val = atts.value(name.isEmpty() ? QString("align") : name);
    if (!val.isEmpty()) {
	QStringList list = QStringList::split(',', val);
	QStringList::Iterator it;
	for (it = list.begin(); it != list.end(); ++it) {
	    val = *it;
	    if (val == "right")
		align = (align & 0x38) | Qt::AlignRight;
	    else if (val == "hcenter")
		align = (align & 0x38) | Qt::AlignHCenter;
	    else if (val == "left")
		align = (align & 0x38) | Qt::AlignLeft;
	    else if (val == "bottom")
		align = (align & 0x07) | Qt::AlignBottom;
	    else if (val == "vcenter")
		align = (align & 0x07) | Qt::AlignVCenter;
	    else if (val == "top")
		align = (align & 0x07) | Qt::AlignTop;
	}
    }

    return align;
}

void ThemeItem::addCharacters(const QString &)
{
}

//---------------------------------------------------------------------------

ThemeGroupItem::ThemeGroupItem( ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts )
    : ThemeItem( p, ir, atts )
{
}

int ThemeGroupItem::rtti() const
{
    return ThemedView::Group;
}

void ThemeGroupItem::setPressed( bool p )
{
    ThemeItem::setPressed( p );
    ThemeItem *item = 0;
    for( item = chldn.first() ; item ; item = chldn.next() )
	item->setPressed( p );
}


//---------------------------------------------------------------------------

#ifndef QTOPIA_DESKTOP

ThemePluginItem::ThemePluginItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemeItem(p, v, atts)
{
    init();
}

ThemePluginItem::~ThemePluginItem()
{
    releasePlugin();
    delete loader;
}

void ThemePluginItem::init()
{
    loader = 0;
    iface = 0;
    builtin = FALSE;
}

void ThemePluginItem::setPlugin(const QString &name)
{
    if (!loader)
	loader = new PluginLoader("themedview");
    releasePlugin();
    loader->queryInterface(name, IID_ThemedItem, (QUnknownInterface**)&iface);
    builtin = FALSE;
    iface->resize(br.width(), br.height());
    if (isVisible())
	update();
}

void ThemePluginItem::setBuiltin(ThemedItemInterface *i)
{
    releasePlugin();
    iface = i;
    if (iface) {
	iface->addRef();
	builtin = TRUE;
	iface->resize(br.width(), br.height());
    }
    if (isVisible())
	update();
}

int ThemePluginItem::rtti() const
{
    return ThemedView::Plugin;
}

void ThemePluginItem::paint(QPainter *p, const QRect &r)
{
    if (iface)
	iface->paint(p, r);
}

void ThemePluginItem::layout()
{
    QSize oldSize = br.size();
    ThemeItem::layout();
    if (iface && br.size() != oldSize) {
	iface->resize(br.width(), br.height());
    }
}

void ThemePluginItem::releasePlugin()
{
    if (iface && loader && !builtin)
	loader->releaseInterface(iface);
    else if (builtin)
	iface->release();
    iface = 0;
}

#endif

//---------------------------------------------------------------------------

ThemeExclusiveItem::ThemeExclusiveItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemeItem(p, v, atts)
{
}

int ThemeExclusiveItem::rtti() const
{
    return ThemedView::Exclusive;
}

void ThemeExclusiveItem::layout()
{
    ThemeItem::layout();
    QListIterator<ThemeItem> it(chldn);
    it.toLast();
    bool found = FALSE;
    ThemeItem *item;
    while ((item = it.current()) != 0) {
	if (!found) {
	    item->visible = TRUE;
	    if (item->active())
		found = TRUE;
	} else if (found) {
	    item->visible = FALSE;
	}
	--it;
    }
}

//---------------------------------------------------------------------------

ThemeLayoutItem::ThemeLayoutItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemeItem(p, v, atts)
{
    init();
    orient = atts.value("orientation") == "vertical" ? Qt::Vertical : Qt::Horizontal;
    QString val = atts.value("spacing");
    if (!val.isEmpty())
	spacing = val.toInt();
    align = parseAlignment(atts);
    stretch = atts.value("stretch") == "yes";
}

void ThemeLayoutItem::init()
{
    spacing = 0;
}

int ThemeLayoutItem::rtti() const
{
    return ThemedView::Layout;
}

void ThemeLayoutItem::layout()
{
    ThemeItem::layout();
    QListIterator<ThemeItem> it(chldn);
    ThemeItem *item;
    int totalSize = 0;
    int visCount = 0;
    while ((item = it.current()) != 0) {
	if ((!item->transient || item->actv) && item->visible) {
	    visCount++;
	    if (orient == Qt::Horizontal) {
		totalSize += item->sr.width();
	    } else {
		totalSize += item->sr.height();
	    }
	}
	++it;
    }
    if (!visCount)
	return;

    int offs = 0;
    int itemSize = 0;
    int size = orient == Qt::Horizontal ? br.width() : br.height();
    if (stretch) {
	itemSize = (size - (visCount-1)*spacing)/visCount;
    } else {
	if (align & Qt::AlignHCenter || align & Qt::AlignVCenter)
	    offs = (size - totalSize - (visCount-1)*spacing)/2;
	else if (align & Qt::AlignRight)
	    offs = size - totalSize - (visCount-1)*spacing;
    }
    item = it.toFirst();
    while ((item = it.current()) != 0) {
	if ((!item->transient || item->actv) && item->visible) {
	    if (orient == Qt::Horizontal) {
		item->sr.moveTopLeft(QPoint(offs,item->sr.y()));
		if (stretch)
		    item->sr.setWidth(itemSize);
		offs += QABS(item->sr.width());
	    } else {
		item->sr.moveTopLeft(QPoint(item->sr.x(), offs));
		if (stretch)
		    item->sr.setHeight(itemSize);
		offs += QABS(item->sr.height());
	    }
	    offs += spacing;
	}
	++it;
    }
}

//---------------------------------------------------------------------------

ThemePageItem::ThemePageItem(ThemedView *v, const QXmlAttributes &atts)
    : ThemeItem(0, v, atts), stretch(FALSE)
{
    bd = atts.value("base");
    if (bd[bd.length()-1] != '/')
	bd.append('/');
    QString val = atts.value("background");
    if (!val.isEmpty()) {
	bg = Resource::loadPixmap(bd+val);
	if( bg.isNull() )
	    bg = Resource::loadPixmap(v->defaultPics()+val);
    }
    v->clearMask();
    maskImg = atts.value("mask");
    val = atts.value("stretch");
    if (!val.isEmpty()) {
	QStringList ol = QStringList::split(',', val);
	if (ol.count() == 2) {
	    stretch = TRUE;
	    offs[0] = ol[0].toInt();
	    offs[1] = ol[1].toInt();
	}
    }
}

void ThemePageItem::applyMask()
{
    if (!br.width() || !br.isValid())
	return;

    if (!maskImg.isEmpty()) {
	QBitmap mask;
	QPixmap maskPm = Resource::loadPixmap(bd+maskImg);
	if( maskPm.isNull() )
	    maskPm = Resource::loadPixmap(view->defaultPics());
	if (!maskPm.isNull()) {
	    if (maskPm.depth() == 1) {
		mask = maskPm;
	    } else if (maskPm.mask()) {
		mask = *maskPm.mask();
	    }
	}

	QRegion maskRgn;
	if (stretch) {
	    QRegion r = QRegion(mask);
	    QRegion rgn[3];
	    rgn[0] = r & QRegion(0,0,offs[0],mask.height());
	    rgn[1] = r & QRegion(offs[0],0,offs[1]-offs[0],mask.height());
	    rgn[2] = r & QRegion(offs[1],0,mask.width()-offs[1],mask.height());

	    QRegion trgn;
	    trgn = rgn[0].isEmpty() ? QRect(0, 0, offs[0], mask.height()) : rgn[0];
	    maskRgn = trgn;

	    int w = br.width() - offs[0] - (mask.width()-offs[1]);
	    if (!rgn[1].isNull()) {
		QRegion tmp(rgn[1]);
		int sw = offs[1] - offs[0];
		for (int x = 0; x < w; x+=sw) {
		    trgn += tmp;
		    tmp.translate(sw, 0);
		}
		trgn &= QRect(offs[0], 0, w, mask.height());
	    } else {
		trgn = QRect(offs[0], 0, w, mask.height());
	    }
	    maskRgn += trgn;

	    trgn = rgn[2].isEmpty() ? QRect(offs[1], 0, mask.width()-offs[1], mask.height()) : rgn[2];
	    trgn.translate(br.width()-mask.width(),0);
	    maskRgn += trgn;
	} else {
	    maskRgn = QRegion(mask);
	}
	view->setMask(maskRgn);
    }
}

int ThemePageItem::rtti() const
{
    return ThemedView::Page;
}

void ThemePageItem::paint(QPainter *p, const QRect &)
{
    if (!bg.isNull())
	p->drawPixmap(0, 0, bg);
}

void ThemePageItem::layout()
{
    int oldWidth = br.width();
    br = sr;
    if (!sr.width()) {
	br.setWidth(view->width());
    } else if (sr.width() < 0) {
	br.setX(view->width()+sr.width()-sr.x());
	br.setWidth(-sr.width());
    }
    if (!sr.height()) {
	br.setHeight(view->height());
    } else if (sr.height() < 0) {
	br.setY(view->height()+sr.height()-sr.y());
	br.setHeight(-sr.height());
    }

    if (!maskImg.isEmpty() && br.width() != oldWidth)
	applyMask();
}

QSize ThemePageItem::sizeHint() const
{
    return sr.size();
}

//---------------------------------------------------------------------------

ThemeGraphicItem::ThemeGraphicItem( ThemeItem *p, ThemedView *v, const QXmlAttributes &atts )
    : ThemeItem( p, v, atts )
{

}

void ThemeGraphicItem::setColor( const QString &key, const QColor &val, int st )
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeGraphicItem::setPixmap() - Invalid state '%d'", st );
	return;
    }
    QMap<QString,QColor> &dataset = colors[st];
    if( dataset.contains( key ) )
	dataset.remove( key );
    dataset.insert( key, val );
}

QColor ThemeGraphicItem::color( const QString &key, int st ) const
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeGraphicItem::color() - Invalid state '%d'", st );
	return QColor();
    }
    const QMap<QString,QColor> &dataset = colors[st];
    return dataset[key];
}

void ThemeGraphicItem::setFont( const QString &key, const QFont &val, int st )
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeGraphicItem::setFont() - Invalid state '%d'", st );
	return;
    }
    QMap<QString,QFont> &dataset = fonts[st];
    if( dataset.contains( key ) )
	dataset.remove( key );
    dataset.insert( key, val );
}

QFont ThemeGraphicItem::font( const QString &key, int st ) const
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeGraphicItem::font() - Invalid state '%d'", st );
	return QFont();
    }
    const QMap<QString,QFont> &dataset = fonts[st];
    return dataset[key];
}

QFont ThemeGraphicItem::parseFont(const QFont &defFont, const QString &size, const QString &bold)
{
    QFont font(defFont);
    if (!size.isEmpty())
	font.setPointSize(size.toInt());
    bool b = bold == "yes";
    font.setWeight(b ? QFont::Bold : QFont::Normal);

    return font;
}

int ThemeGraphicItem::parseColor(const QString &val, QColor &col)
{
    int role = QColorGroup::NColorRoles;
    if (!val.isEmpty()) {
	int i = 0;
	while (colorTable[i].name) {
	    if (QString(colorTable[i].name).lower() == val.lower()) {
		role = colorTable[i].role;
		break;
	    }
	    i++;
	}
	if (!colorTable[i].name) {
	    role = QColorGroup::NColorRoles+1;
	    col.setNamedColor(val);
	}
    }

    return role;
}

QColor ThemeGraphicItem::getColor(const QColor &col, int role) const
{
    if (role < QColorGroup::NColorRoles)
	return view->colorGroup().color((QColorGroup::ColorRole)role);
    else
	return col;
}

//---------------------------------------------------------------------------

ThemeInputItem::ThemeInputItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts)
    : ThemeGraphicItem(p, ir, atts)
{
    init();

    setAttribute( "size", atts.value("size") );
    setAttribute( "bold", atts.value("bold") );

    setupWidget();

    if( !atts.value("colorGroup").isEmpty() )
	colorGroupAtts = parseSubAtts(atts.value("colorGroup"));
    parseColorGroup( colorGroupAtts );
}

void ThemeInputItem::init()
{
    mWidget = 0;
}

QWidget *ThemedView::newInputItemWidget(ThemeInputItem*, const QString&)
{
    return 0;
}

void ThemeInputItem::setupWidget()
{
    const QString in = itemName().lower();
#ifndef QTOPIA_DESKTOP
    mWidget = view->newInputItemWidget(this,in);
    if ( !mWidget ) {
	if( in == "callscreennumber" ) 
	    mWidget = (QWidget *)new QLineEdit( view );
    }
    if ( !mWidget )
	qWarning("Unknown input field '%s'", in.latin1());
#else
    if( "dialernumber" == in  || "callscreennumber" == in )
	mWidget = new QLineEdit( view );
    else if( "callscreen" == in )
	mWidget = new QListBox( view );
#endif

    if( mWidget )
	mWidget->setFont( 
		ThemeGraphicItem::parseFont( mWidget->font(), strAttribute("size"), 
							strAttribute("bold") )
		);

}

QWidget *ThemeInputItem::widget() const
{
    if( !mWidget )
	qWarning("No valid widget set for input element %s, returning NULL", itemName().latin1());
    return mWidget;
}

void ThemeInputItem::paletteChange(const QPalette &)
{
    parseColorGroup( colorGroupAtts );
}

void ThemeInputItem::parseColorGroup( const QMap<QString,QString> &cgatts )
{
    if( !mWidget )
	return;
    QPalette pal = view->palette();
    for( int i = 0 ; colorTable[i].role != QColorGroup::NColorRoles ; ++i )
    {
	const QString curColorName = QString(colorTable[i].name).lower();
	QColor colour;
	for( QMap<QString,QString>::ConstIterator it = cgatts.begin() ; 
						    it != cgatts.end() ; ++it )
	{
	    if( it.key().lower() == curColorName ) {
		colour = getColor( it.data(), parseColor( it.data(), colour ) );
		break;
	    }
	}
	if( colour.isValid() ) {
	    pal.setColor( QPalette::Active, colorTable[i].role, colour );
	    pal.setColor( QPalette::Inactive, colorTable[i].role, colour );
	    pal.setColor( QPalette::Disabled, colorTable[i].role, colour );
	}
    }
    mWidget->setPalette( pal );
}

void ThemeInputItem::layout()
{
    ThemeItem::layout();
    if( !mWidget )
	return;
    if( sr.width() > view->width() )
	br.setWidth( mWidget->sizeHint().width() );
    if( sr.height() > view->height() )
	br.setHeight( mWidget->sizeHint().height() );
    if( itemName() == "dialernumber" ) {
	QRect r = rect();
	mWidget->setGeometry( r.x(), r.y(), r.width(), r.height() );
    }
    // call screen handles the setting of the geometry of its items
}

int ThemeInputItem::rtti() const
{ 
    return ThemedView::Input;
}

//---------------------------------------------------------------------------

ThemeTextItem::ThemeTextItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemeGraphicItem(p, v, atts), shortLbl(FALSE), shadow(0), shadowImg(0),
	format(Qt::AutoText), richText(FALSE)
{

    QMap<QString,QString> onClickAtts, onFocusAtts;
    if( interactive ) {
	onClickAtts = parseSubAtts( atts.value("onclick") );
	onFocusAtts = parseSubAtts( atts.value("onfocus") );
    }
    setupFont( v->font(), atts.value("size"), atts.value("bold"), atts.value("color"), atts.value("outline") );
    setupFont( font("font"), onClickAtts["size"], onClickAtts["bold"], onClickAtts["color"], onClickAtts["outline"], ThemeItem::Pressed );
    setupFont( font("font"), onFocusAtts["size"], onFocusAtts["bold"], onFocusAtts["color"], onFocusAtts["outline"], ThemeItem::Focus );

    QString val = atts.value("shadow");
    if (!val.isEmpty())
	shadow = QMIN(val.toInt(), 255);
    align = parseAlignment(atts);
    shortLbl = atts.value("short") == "yes";
    val = atts.value("format");
    if (!val.isEmpty()) {
	if (val == "RichText") {
	    format = Qt::RichText;
	    richText = TRUE;
	} else if (val == "PlainText") {
	    format = Qt::PlainText;
	}
    }
}

ThemeTextItem::~ThemeTextItem()
{
    delete shadowImg;
}

void ThemeTextItem::setupFont( const QFont &deffont, const QString &size, const QString &bold, const QString &col, const QString &outline, ThemeItem::State st )
{
    setFont( "font", parseFont( deffont, size, bold ), st );
    QColor colour;
    int role = parseColor( col, colour );
    if( role == QColorGroup::NColorRoles && st != ThemeItem::Default )
	role = attribute( "colorRole" );
    if( !colour.isValid() && st != ThemeItem::Default )
	colour = color( "color" );
    setAttribute( "colorRole", role, st );
    setColor( "color", colour, st );
    role = parseColor( outline, colour );
    if( role == QColorGroup::NColorRoles && st != ThemeItem::Default )
	role = attribute( "outlineRole" );
    if( !colour.isValid() && st != ThemeItem::Default )
	colour = color( "outline" );
    setAttribute( "outlineRole", role, st );
    setColor( "outline", colour, st );
}

void ThemeTextItem::setText(const QString &t)
{
    if (t != txt) {
	txt = t;
	if (format == Qt::AutoText)
	    richText = QStyleSheet::mightBeRichText(txt);
	delete shadowImg;
	shadowImg = 0;
	if (isVisible())
	    update();
    }
}

void ThemeTextItem::setTextFormat(Qt::TextFormat fmt)
{
    format = fmt;
    switch (format) {
	case Qt::AutoText:
	    richText = QStyleSheet::mightBeRichText(txt);
	    break;
	case Qt::RichText:
	    richText = TRUE;
	    break;
	default:
	    richText = FALSE;
    }
}

int ThemeTextItem::rtti() const
{
    return ThemedView::Text;
}

void ThemeTextItem::paint(QPainter *p, const QRect &)
{
    QFont defaultFnt = font( "font", state() );
    if (!txt.isEmpty()) {
        QFont fnt(defaultFnt);
        if (name == "time" ) {
            // (workaround) use helvetica for time strings or we run into trouble
            // when the default font is not scalable
            // assumption!!??!: time string can always be displayed 
            // in helvetica no matter what language we have
            fnt.setFamily("helvetica");
        }
            
	QString text(txt);
	int x = 0;
	int y = 0;
	int w = br.width();
	int h = br.height();
	QSimpleRichText *rt = 0;
	if (richText) {
	    if (align & Qt::AlignHCenter)
		text = "<center>"+text+"</center>";
	    rt = new QSimpleRichText(text, fnt);
	    rt->setWidth(br.width());
	    w = rt->widthUsed();
	    h = rt->height();
	    if (align & Qt::AlignRight)
		x += br.width()-w-1;
	    if (align & Qt::AlignBottom)
		y += br.height()-h-1;
	    else if (align & Qt::AlignVCenter)
		y += (br.height()-h-1)/2;
	}

	QPalette pal(view->palette());
	if (shadow) {
	    if (!shadowImg) {
		QPixmap spm(QMIN(w+2,br.width()), QMIN(h+2, br.height()));
		spm.fill();
		QPainter sp(&spm);
		if (richText) {
		    pal.setColor(QColorGroup::Text, Qt::black);
		    rt->draw(&sp, 1, 1, QRect(0, 0, br.width(), br.height()), pal.active());
		} else {
		    sp.setPen(Qt::black);
		    sp.setFont(fnt);
		    sp.drawText(QRect(1,1,w,h), align, text);
		}
		QImage img = spm.convertToImage();
		shadowImg = new QImage(w+2, h+2, 32);
		shadowImg->fill(0x00000000);
		shadowImg->setAlphaBuffer(TRUE);
		int sv = (shadow/5) << 24;
		for (int i = 1; i < img.height()-1; i++) {
		    QRgb *r0 = (QRgb *)shadowImg->scanLine(i-1);
		    QRgb *r1 = (QRgb *)shadowImg->scanLine(i);
		    QRgb *r2 = (QRgb *)shadowImg->scanLine(i+1);
		    for (int j = 1; j < img.width()-1; j++, r0++, r1++, r2++) {
			if (!(img.pixel(j, i) & 0x00ffffff)) {
			    *r0 += sv;
			    *r1 += sv;
			    *(r1+1) += sv;
			    *(r1+2) += sv;
			    *r2 += sv;
			    *(r2+1) += sv;
			}
		    }
		}
	    }
	    p->drawImage(x, y-1, *shadowImg);
	}
	QColor col = color( "color", state() );
	int role = attribute( "colorRole", state() ),
	    outlineRole = attribute( "outlineRole", state() );
	if (richText) {
	    pal.setColor(QColorGroup::Text, getColor(col, role));
	    if (outlineRole != QColorGroup::NColorRoles)
		drawOutline(p, QRect(x,y,br.width(),br.height()), pal, rt);
	    else
		rt->draw(p, x, y, QRect(0, 0, br.width(), br.height()), pal.active());
	} else {
	    p->setPen(getColor(col, role));
	    p->setFont(fnt);
	    if (outlineRole != QColorGroup::NColorRoles)
		drawOutline(p, QRect(x,y,w,h), align, text);
	    else {
		p->drawText(QRect(x,y,w,h), align, text);
	    }
	}
	delete rt;
    }
}

void ThemeTextItem::drawOutline(QPainter *p, const QRect &r, int flags, const QString &text)
{
    QColor outlineColor = color( "outline", state() );
    int outlineRole = attribute( "outlineRole", state() );
    QPen oldPen = p->pen();
    p->setPen(getColor(outlineColor, outlineRole));
    QRect sr(r);
    sr.moveBy(-1,0);
    p->drawText(sr, flags, text);
    sr.moveBy(2,0);
    p->drawText(sr, flags, text);
    sr.moveBy(-1,-1);
    p->drawText(sr, flags, text);
    sr.moveBy(0,2);
    p->drawText(sr, flags, text);

    p->setPen(oldPen);
    p->drawText(r, flags, text);
}

void ThemeTextItem::drawOutline(QPainter *p, const QRect &r, const QPalette &pal, QSimpleRichText *rt)
{
    QPalette sp(pal);
    QColor outlineColor = color( "outline", state() );
    int outlineRole = attribute( "outlineRole", state() );
    sp.setColor(QColorGroup::Text, getColor(outlineColor, outlineRole));
    QRect sr(r);
    sr.moveBy(-1,0);
    rt->draw(p, sr.x(), sr.y(), QRect(0, 0, r.width(), r.height()), sp.active());
    sr.moveBy(2,0);
    rt->draw(p, sr.x(), sr.y(), QRect(0, 0, r.width(), r.height()), sp.active());
    sr.moveBy(-1,-1);
    rt->draw(p, sr.x(), sr.y(), QRect(0, 0, r.width(), r.height()), sp.active());
    sr.moveBy(0,2);
    rt->draw(p, sr.x(), sr.y(), QRect(0, 0, r.width(), r.height()), sp.active());


    rt->draw(p, r.x(), r.y(), QRect(0, 0, r.width(), r.height()), pal.active());
}

void ThemeTextItem::addCharacters(const QString &ch)
{
    setText(text() + ch);
}

//---------------------------------------------------------------------------

class ThemeRectItemPrivate
{
public:
    int alpha;
    QBrush brush;
};

ThemeRectItem::ThemeRectItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemeGraphicItem(p, v, atts)
{
    d[0] = 0;
    d[1] = 0;
    d[2] = 0;
    QMap<QString,QString> onClickAtts, onFocusAtts;
    if( interactive ) {
	onClickAtts = parseSubAtts( atts.value("onclick") );
	onFocusAtts = parseSubAtts( atts.value("onfocus") );
    }

    setupColor( "fg", atts.value( "color" ) );
    setupColor( "fg", onClickAtts["color"], ThemeItem::Pressed );
    setupColor( "fg", onFocusAtts["color"], ThemeItem::Focus );

    setupColor( "bg", atts.value( "brush" ) );
    setupColor( "bg", onClickAtts["brush"], ThemeItem::Pressed );
    setupColor( "bg", onFocusAtts["brush"], ThemeItem::Focus );

    setupAlpha( "alpha", atts.value( "alpha" ) );
    setupAlpha( "alpha", onClickAtts["alpha"], ThemeItem::Pressed );
    setupAlpha( "alpha", onFocusAtts["alpha"], ThemeItem::Focus );
}

void ThemeRectItem::setupColor( const QString &key, const QString &col, ThemeItem::State st )
{
    QColor colour;
    int role = parseColor( col, colour );
    if( role == QColorGroup::NColorRoles && st != ThemeItem::Default )
    {
	role = attribute( key + "Role" );
	colour = color( key );
    }
    setAttribute( key + "Role", role, st );
    setColor( key, colour, st );
}

void ThemeRectItem::setupAlpha( const QString &, const QString &al, ThemeItem::State st )
{
    int idx = stateToIndex( st );
    if( idx < 0 || idx > 2 ) 
    {
	qDebug( "ThemeRectItem::setupAlpha - Invalid state '%d'", st );
	return;
    }
    bool ok;
    int alpha = al.toInt(&ok);
    if (ok) {
	if (!d[idx])
	    d[idx] = new ThemeRectItemPrivate;
	d[idx]->alpha = QMIN(alpha, 255);
	QImage img(8,8,32);
	Q_UINT32 val = brushColor(st).rgb();
	val = (val & 0x00FFFFFF) | (alpha << 24);
	Q_UINT32 *ptr = (Q_UINT32*)img.scanLine(0);
	for (int i = 0; i < img.width(); i++)
	    *ptr++ = val;
	for (int j = 1; j < img.height(); j++)
	    memcpy(img.scanLine(j), img.scanLine(0), img.bytesPerLine());
	img.setAlphaBuffer(TRUE);
	QPixmap pix;
	pix.convertFromImage(img);
	d[idx]->brush = QBrush(brushColor(st), pix);
    }
}

QColor ThemeRectItem::brushColor( ThemeItem::State st ) const
{
    return getColor( color( "bg", st ), attribute( "bgRole", st ) );
}

int ThemeRectItem::rtti() const
{
    return ThemedView::Rect;
}

void ThemeRectItem::paint(QPainter *p, const QRect &)
{
    int fgRole = attribute( "fgRole", state() );
    if (fgRole == QColorGroup::NColorRoles)
	p->setPen(QPen::NoPen);
    else
	p->setPen(getColor(color( "fg", state() ), fgRole));
    int idx = stateToIndex(state());
    int bgRole = attribute( "bgRole", state() );
    if (bgRole == QColorGroup::NColorRoles)
	p->setBrush(QBrush::NoBrush);
    else if (d[idx] && d[idx]->alpha != 255)
	p->setBrush(d[idx]->brush);
    else
	p->setBrush(getColor(color( "bg", state()), bgRole));
    p->drawRect(0, 0, br.width(), br.height());
}

//---------------------------------------------------------------------------

ThemeLineItem::ThemeLineItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemeGraphicItem(p, v, atts)
{
    QColor colour;
    setAttribute( "colorRole", parseColor(atts.value("color"), colour) );
    setColor( "color", colour );
}

int ThemeLineItem::rtti() const
{
    return ThemedView::Line;
}

void ThemeLineItem::paint(QPainter *p, const QRect &)
{
    p->setPen(getColor(color( "color" ), attribute( "colorRole")));
    p->drawLine(0, 0, br.width()-1, br.height()-1);
}

//---------------------------------------------------------------------------

ThemePixmapItem::ThemePixmapItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts)
    : ThemeGraphicItem( p, ir, atts )
{
}

QPixmap ThemePixmapItem::pixmap( const QString &key, int st ) const
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemePixmapItem::pixmap() - Invalid state '%d'", st );
	return QPixmap();
    }
    const QMap<QString,QPixmap> &dataset = pixmaps[st];
    return dataset[key];
}

void ThemePixmapItem::setPixmap( const QString &key, const QPixmap &val, int st )
{
    st = stateToIndex( st );
    if( st < 0 || st > 2 ) 
    {
	qDebug( "ThemeItem::setPixmap() - Invalid state '%d'", st );
	return;
    }
    QMap<QString,QPixmap> &dataset = pixmaps[st];
    if( dataset.contains( key ) )
	dataset.remove( key );
    dataset.insert( key, val );
}


static void yuv_to_rgb(int Y, int U, int V, int& R, int& G, int&B)
{
    R = int(Y +  ((92242 * (V - 128)) >> 16));
    G = int(Y - (((22643 * (U - 128)) >> 16)) - ((46983 * (V - 128)) >> 16));
    B = int(Y + ((116589 * (U - 128)) >> 16));
}

static void rgb_to_yuv(int R, int G, int B, int& Y, int& U, int& V)
{
    Y = int(R *  19595 + G *  38470 + B *  7471) >> 16;
    U = int(R * -11076 + G * -21758 + (B << 15) + 8388608) >> 16;
    V = int(R *  32768 + G * -27460 + B * -5328 + 8388608) >> 16;
}

QPixmap ThemePixmapItem::loadImage(const QString &filename, int colorRole, const QColor &col, int alpha)
{
    QPixmap pm;
    static QString dflt_path("default/");

    if (filename.isEmpty())
	return pm;

    QString imgName = filename;

    if (colorRole != QColorGroup::NColorRoles || alpha != 255) {
	QColor colour = getColor(col, colorRole);
	QImage img;
	if (!imgName.startsWith(dflt_path))
	    imgName = view->base()+imgName;
	else
	    imgName = view->defaultPics() + imgName.mid(dflt_path.length());
	img = Resource::loadImage( imgName );
        if ( img.isNull() )
            return pm;
	QRgb *rgb;
	int count;
	if (img.depth() == 32) {
	    rgb = (QRgb*)img.bits();
	    count = img.bytesPerLine()/sizeof(QRgb)*img.height();
	} else {
	    rgb = img.colorTable();
	    count = img.numColors();
	}
	int sr, sg, sb;
	colour.rgb(&sr, &sg, &sb);
	int sy, su, sv;
	rgb_to_yuv(sr,sg,sb,sy,su,sv);
	for (int i = 0; i < count; i++, rgb++) {
	    int a = (*rgb >> 24) & 0xff;
	    int r = (*rgb >> 16) & 0xff;
	    int g = (*rgb >> 8) & 0xff;
	    int b = *rgb & 0xff;
	    if (colorRole != QColorGroup::NColorRoles) {
		int y,u,v;
		rgb_to_yuv(r,g,b,y,u,v);
		y = (y*2+sy)/3;
		u = (u+su*2)/3;
		v = (v+sv*2)/3;
		yuv_to_rgb(y,u,v,r,g,b);
		if (r>255) r = 255; if (r<0) r=0;
		if (g>255) g = 255; if (g<0) g=0;
		if (b>255) b = 255; if (b<0) b=0;
	    }
	    if (alpha != 255)
		a = (a*alpha)/255;
	    *rgb = qRgba(r, g, b, a);
	}
	if (alpha != 255)
	    img.setAlphaBuffer(TRUE);
	pm.convertFromImage(img);

    } else {
	if (!imgName.startsWith(dflt_path))
	    pm = Resource::loadPixmap(view->base()+imgName);
	else
	    imgName = imgName.mid(dflt_path.length());
	if( pm.isNull() )
	    pm = Resource::loadPixmap(view->defaultPics()+imgName);
    }

    return pm;
}

//---------------------------------------------------------------------------

class ThemeAnimationItemPrivate : public QObject
{
public:
    ThemeAnimationItemPrivate(ThemeAnimationItem *a, int p)
	: QObject(), anim(a), tid(0), period(p) {
    }

    void setPeriod(int p) {
	period = p;
    }

    int getPeriod() const {
	return period;
    }

    void start() {
	stop();
	tid = startTimer(period);
    }
    void stop() {
	if (tid) {
	    killTimer(tid);
	    tid = 0;
	}
    }
	
protected:
    void timerEvent(QTimerEvent *) {
	anim->advance();
    }

private:
    ThemeAnimationItem *anim;
    int tid;
    int period;
};


ThemeAnimationItem::ThemeAnimationItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemePixmapItem(p, v, atts), currFrame(0), inc(1)
{
    d[0] = d[1] = d[2] = 0;

    QMap<QString,QString> onClickAtts, onFocusAtts;
    if( interactive ) {
	onClickAtts = parseSubAtts( atts.value("onclick") );
	onFocusAtts = parseSubAtts( atts.value("onfocus") );
    }

    setupAnimation( v->base(), atts.value( "src" ), atts.value( "color" ), atts.value( "alpha" ), atts.value( "count" ), atts.value( "width" ), 
	    atts.value( "loop" ), atts.value( "looprev" ), atts.value( "delay" ) );
    QString filename;
    filename = onClickAtts["src"];
    /*
    if( filename.isEmpty() )
	filename = atts.value( "src" );
	*/
    setupAnimation( v->base(), filename, onClickAtts["color"], onClickAtts["alpha"], onClickAtts["count"], onClickAtts["width"], 
	    onClickAtts["loop"], onClickAtts["looprev"], onClickAtts["delay"], ThemeItem::Pressed );
    filename = onFocusAtts["src"];
    /*
    if( filename.isEmpty() )
	filename = atts.value( "src" );
	*/
    setupAnimation( v->base(), filename, onFocusAtts["color"], onFocusAtts["alpha"], onFocusAtts["count"], onFocusAtts["width"], 
	    onFocusAtts["loop"], onFocusAtts["looprev"], onFocusAtts["delay"], ThemeItem::Focus );

    for( int i = 0 ; i < 3 ; ++i )
	if( !pixmap( "src", indexToState(i) ).isNull() && attribute( "delay", indexToState(i) ) > 0 )
	    d[i] = new ThemeAnimationItemPrivate(this, attribute("delay", indexToState(i) ));
	else
	    d[i] = 0;

    currFrame = 0;
    inc = 1;
}

ThemeAnimationItem::~ThemeAnimationItem()
{
    for( int i = 0 ; i < 3 ; ++i )
	if( d[i] )
	    delete d[i];
}

void ThemeAnimationItem::setupAnimation( const QString &, const QString &src, const QString &col, const QString &alpha, const QString &count, 
const QString &width, const QString &loop, const QString &looprev, const QString &delay, ThemeItem::State st )
{
    QColor colour;
    int role = QColorGroup::NColorRoles;
    QString c = col;
    if( c.isEmpty() && st != ThemeItem::Default ) {
	colour = color( "color" );
	role = attribute( "colorRole" );
    } else if( !c.isEmpty() )
	role = parseColor( c, colour );
    int al;
    if( alpha.isEmpty() && st != ThemeItem::Default )
	al = attribute( "alpha" );
    else if( !alpha.isEmpty() )
	al = alpha.toInt();
    else
	al = 255;
    QString s = src;
    if( s.isEmpty() && st != ThemeItem::Default ) {
	s = strAttribute( "src" );
    }
    setAttribute( "src", s, st );
    QPixmap pm = loadImage( s, role, colour, al );
    setPixmap( "src", pm, st );
    setAttribute( "colorRole", role, st );
    setColor( "color", colour, st );
    setAttribute( "alpha", al, st );

    if( !count.isEmpty() )
	setAttribute( "count", count.toInt(), st );
    else if( st != ThemeItem::Default )
	setAttribute( "count", attribute( "count" ), st );
    else
	setAttribute( "count", 0 );
    if( !width.isEmpty() )
	setAttribute( "width", width.toInt(), st );
    else if( st != ThemeItem::Default )
	setAttribute( "width", attribute( "width" ), st );
    else
    {
	int count = attribute("count");
	if( count <= 0 )
	    count = 1;
	setAttribute( "width", pixmap( "src" ).width() / count );
    }
    if( !loop.isEmpty() )
	setAttribute( "loop", loop.toInt(), st );
    else if( st != ThemeItem::Default )
	setAttribute( "loop", attribute( "loop" ), st );
    else
	setAttribute( "loop", -1, st );
    if( !looprev.isEmpty() )
	setAttribute( "looprev", looprev.toInt(), st );
    else if( st != ThemeItem::Default )
	setAttribute( "looprev", attribute( "looprev" ), st );
    else
	setAttribute( "looprev", 0 );
    if( !delay.isEmpty() )
	setAttribute( "delay", delay.toInt(), st );
    else if( st != ThemeItem::Default )
	setAttribute( "delay", attribute( "delay" ), st );
    else 
	setAttribute( "delay", 0 );
}

void ThemeAnimationItem::setFrame(int f)
{
    if (f >= 0 && f < attribute("count", state())) {
	currFrame = f;
	if (isVisible())
	    update();
    }
}

void ThemeAnimationItem::start()
{
    int cst = stateToIndex( state() );
    //set all variables, even if there's no animation for this state
    if( rtti() != ThemedView::Level ) {
	currFrame = 0;
	inc = 1;
    } //don't reset current frame for levels
    QPixmap pm = pixmap( "src", state() );
    if ( d[cst] && attribute("delay",state()) > 0 && !pm.isNull() )
	d[cst]->start();
}

void ThemeAnimationItem::stop()
{
    for( int i = 0 ; i < 3 ; ++i )
	if(d[i]) {
	    d[i]->stop();
	}
}

void ThemeAnimationItem::paint(QPainter *p, const QRect &)
{
    QPixmap pm = pixmap( "src", state() );
    if (!pm.isNull()) {
	p->drawPixmap((br.width()-attribute("width",state()))/2, (br.height()-pm.height())/2,
		pm, currFrame*attribute("width",state()), 0, attribute("width",state()), pm.height());
    }
}

void ThemeAnimationItem::layout()
{
    ThemeItem::layout();

    if ((!transient || active()) && visible) {
	start();
    } else {
	stop();
	currFrame = 0;
    }
}

void ThemeAnimationItem::advance()
{
    currFrame += inc;
    bool rev = attribute("looprev", state());
    //qDebug("current frame = %d count = %d inc = %d rev = %d loop = %d", currFrame, count, inc, rev, loop);
    if (currFrame >= attribute("count",state())) {
	if (attribute("loop",state()) >= 0) {
	    if (rev) {
		inc = -inc;
		currFrame = attribute("count",state())-2;
	    } else {
		currFrame = attribute("loop",state());
	    }
	} else {
	    stop();
	}
    } else if (currFrame < attribute("loop",state()) && inc < 0) {
	currFrame = attribute("loop",state())+1;
	inc = -inc;
    }
    if (isVisible())
	update();
}

int ThemeAnimationItem::rtti() const
{
    return ThemedView::Animation;
}

void ThemeAnimationItem::setPressed( bool p )
{
    int bst = stateToIndex( state() );
    ThemeItem::setPressed( p );
    int ast = stateToIndex( state() );
    stateChange( bst, ast );
}

void ThemeAnimationItem::setFocus( bool f )
{
    int bst = stateToIndex( state() );
    ThemeItem::setFocus( f );
    int ast = stateToIndex( state() );
    stateChange( bst, ast );
}

void ThemeAnimationItem::stateChange( int bst, int ast )
{
    if( bst != ast ) {
	stop();
	start();
    }
}

//---------------------------------------------------------------------------

ThemeLevelItem::ThemeLevelItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemeAnimationItem(p, v, atts), minVal(0), val(0)
{
    maxVal = attribute("count",state());
}

void ThemeLevelItem::setValue(int v)
{
    if (val != v) {
	val = v;
	int idx = 0;
	if (maxVal-minVal != 0)
	    idx = attribute("count",state())*(val-minVal)/(maxVal-minVal);
	if (idx < 0)
	    idx = 0;
	if (idx >= attribute("count",state()))
	    idx = attribute("count",state())-1;
	setFrame(idx);
    }
}

void ThemeLevelItem::setRange(int minv, int maxv)
{
    minVal = minv;
    maxVal = maxv;
}

int ThemeLevelItem::rtti() const
{
    return ThemedView::Level;
}
//---------------------------------------------------------------------------

ThemeStatusItem::ThemeStatusItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemePixmapItem(p, v, atts), enable(FALSE)
{
    QMap<QString,QString> onClickAtts, onFocusAtts;
    if( interactive ) {
	onClickAtts = parseSubAtts( atts.value("onclick") );
	onFocusAtts = parseSubAtts( atts.value("onfocus") );
    }

    QString filename = atts.value( "imageon" );
    createImage( "on", filename, atts.value( "coloron" ), atts.value( "alphaon" ) );
    filename = onClickAtts["imageon"];
    if( filename.isEmpty() && onClickAtts["coloron"].length() || onClickAtts["alphaon"].length() )
	filename = atts.value( "imageon" );
    createImage( "on", filename, onClickAtts["coloron"], onClickAtts["alphaon"], ThemeItem::Pressed );
    filename = onFocusAtts["imageon"];
    if( filename.isEmpty() && onFocusAtts["coloron"].length() || onFocusAtts["alphaon"].length() )
	filename = atts.value( "imageon" );
    createImage( "on", filename, onFocusAtts["coloron"], onFocusAtts["alphaon"], ThemeItem::Focus );
    filename = atts.value( "imageoff" );
    createImage( "off", filename, atts.value( "coloroff" ), atts.value( "alphaoff" ) );
}

void ThemeStatusItem::createImage( const QString &key, const QString &filename, const QString &col, const QString &alpha, ThemeItem::State st )
{
    //if the user has specified a file, then col and alpha apply even if they're null
    //else copy deafult state's values
    QColor colour;
    int role = QColorGroup::NColorRoles;
    int al = 255;
    QPixmap pm;
    if( !filename.isEmpty() ) {
	role = parseColor( col, colour );
	if( !alpha.isEmpty() )
	    al = alpha.toInt();
	pm = loadImage( filename, role, colour, al );
    } else if( st != ThemeItem::Default ) {

	role = attribute( "colorrole" + key );
	pm = pixmap( "image" + key );
	colour = color( "color" + key );
	al = attribute( "alpha" + key );
    }
    setPixmap( "image" + key, pm, st );
    setAttribute( "colorrole" + key, role, st );
    setColor( "color" + key , colour, st );
    setAttribute( "alpha" + key, al, st );
}

void ThemeStatusItem::setEnabled(bool e)
{
    if (e != enable) {
	enable = e;
	if( !enable )
	    press = FALSE; // if disabled can't be pressed
	if (isVisible())
	    update();
    }
}

int ThemeStatusItem::rtti() const
{
    return ThemedView::Status;
}

void ThemeStatusItem::paint(QPainter *p, const QRect &)
{
    const QPixmap &pm = enable ? pixmap( "imageon", state() ) : pixmap( "imageoff" );
    if (!pm.isNull())
	p->drawPixmap((br.width()-pm.width())/2, (br.height()-pm.height())/2, pm);
}

//---------------------------------------------------------------------------

ThemeImageItem::ThemeImageItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemePixmapItem(p, v, atts), stretch(FALSE), scale(FALSE), sorient(Qt::Horizontal )
{
    QMap<QString,QString> onClickAtts, onFocusAtts;
    if( interactive ) {
	onClickAtts = parseSubAtts( atts.value("onclick") );
	onFocusAtts = parseSubAtts( atts.value("onfocus") );
    }

    scale = atts.value("scale") == "yes";

    imgName = atts.value("src");
    pressedImgName = onClickAtts["src"];
    if( pressedImgName.isEmpty() && (!onClickAtts["alpha"].isEmpty() || !onClickAtts["color"].isEmpty()) )
	pressedImgName = imgName;
    focusImgName = onFocusAtts["src"];
    if( focusImgName.isEmpty() && (!onFocusAtts["alpha"].isEmpty() || !onFocusAtts["color"].isEmpty()) )
	focusImgName = imgName;

    createImage( imgName, atts.value( "color" ), atts.value( "alpha" ) );
    createImage( pressedImgName, onClickAtts["color"], onClickAtts["alpha"], ThemeItem::Pressed );
    createImage( focusImgName, onFocusAtts["color"], onFocusAtts["alpha"], ThemeItem::Focus );

    QString val = atts.value("stretch");
    if (!val.isEmpty()) {
	QStringList ol = QStringList::split(',', val);
	if (ol.count() == 2) {
	    stretch = TRUE;
	    offs[0] = ol[0].toInt();
	    offs[1] = ol[1].toInt();
	}
	sorient = atts.value("orientation") == "vertical" ? Qt::Vertical : Qt::Horizontal;
    }
    align = parseAlignment(atts, "align", Qt::AlignHCenter|Qt::AlignVCenter);
    tile = atts.value("tile") == "yes";
}

//loads and sets up an image for a given state
void ThemeImageItem::createImage( const QString &filename, const QString &col, const QString &al, ThemeItem::State st )
{
    int colorRole = -1, alpha = 255;
    QColor colour;
    QPixmap pm;
    
    colorRole = parseColor( col, colour );
    if( !al.isEmpty() )
	alpha = al.toInt();
    pm = loadImage( filename, colorRole, colour, alpha );
    setPixmap( "src", pm, st );
    setColor( "color", colour, st );
    setAttribute( "alpha", alpha, st );
    setAttribute( "colorRole", colorRole, st );
}

void ThemeImageItem::setImage( const QPixmap &p, ThemeItem::State st )
{ 
    setPixmap( "src", p, st );
    if( isVisible() ) 
	update();
} 

QPixmap ThemeImageItem::image( ThemeItem::State st ) const
{ 
    return pixmap( "src", st );
}

void ThemeImageItem::layout() 
{
    ThemeItem::layout();
    QPixmap pm = image();
    if( scale && (pm.width() != br.width() || pm.height() != br.height()) ) {
	createImage( imgName, strAttribute("color"), strAttribute("alpha") );
	createImage( pressedImgName, strAttribute("color",ThemeItem::Pressed), strAttribute("alpha",ThemeItem::Pressed), ThemeItem::Pressed );
	createImage( focusImgName, strAttribute("color",ThemeItem::Focus), strAttribute("alpha",ThemeItem::Focus), ThemeItem::Focus );
	setImage( scaleImage( image(),  br.width(), br.height() ) );
	setPixmap( "src", scaleImage( pixmap( "src", ThemeItem::Pressed ), br.width(), br.height() ), ThemeItem::Pressed );
	setPixmap( "src", scaleImage( pixmap( "src", ThemeItem::Focus ), br.width(), br.height() ), ThemeItem::Focus );
    }
}

QPixmap ThemeImageItem::scaleImage( const QPixmap &pix, int width, int height )
{
    if( pix.isNull() || (width <= 0 || height <= 0) || (pix.width() == width && pix.height() == height) )
	return pix;
    qDebug("Scaling %s: width: %d -> %d height: %d -> %d", imgName.latin1(), 
                pix.width(), width, pix.height(), height);
    QImage img = pix.convertToImage();
#if 0
    QSize is = Image::aspectScaleSize( pix.width(), pix.height(), width, height );
    img = img.smoothScale( is.width(), is.height() );
#else
    img = img.smoothScale( width, height );	
#endif
    QPixmap ps;
    ps.convertFromImage( img );
    return ps;
}

int ThemeImageItem::rtti() const
{
    return ThemedView::Image;
}

void ThemeImageItem::paint(QPainter *p, const QRect &r)
{
    QPixmap pix = pixmap( "src", state() );
    if( pix.isNull() ) {
	pix = image();
    }

    if( pix.isNull() )
	return;

    if (tile) {
	int dx = r.x() % pix.width();
	int dy = r.y() % pix.height();
	p->drawTiledPixmap(r.x()-dx, r.y()-dy,
		r.width()+dx, r.height()+dy, pix);
    } else if (!stretch) {
        int xoff = 0;
        if (align & Qt::AlignHCenter)
            xoff = (br.width()-pix.width())/2;
        else if (align & Qt::AlignRight)
            xoff = br.width()-pix.width();
        int yoff = 0;
        if (align & Qt::AlignVCenter)
            yoff = (br.height()-pix.height())/2;
        else if (align & Qt::AlignBottom)
            yoff = br.height()-pix.height();
	QRect cr = r;
	cr.moveBy(-xoff, -yoff);
	cr &= QRect(0, 0, pix.width(), pix.height());
	p->drawPixmap(cr.topLeft()+QPoint(xoff, yoff), pix, cr);
    } else {
	if (sorient == Qt::Horizontal) {
	    int h = pix.height();
	    p->drawPixmap(0, 0, pix, 0, 0, offs[0], h);
	    int w = br.width() - offs[0] - (pix.width()-offs[1]);
	    int sw = offs[1]-offs[0];
	    int x = 0;
	    if (sw) {
		for (; x < w-sw; x+=sw)
		    p->drawPixmap(offs[0]+x, 0, pix, offs[0], 0, sw, h);
	    }
	    p->drawPixmap(offs[0]+x, 0, pix, offs[0], 0, w-x);
	    p->drawPixmap(br.width()-(pix.width()-offs[1]), 0, pix, offs[1], 0, pix.width()-offs[1], h);
	} else {
	    int w = pix.width();
	    p->drawPixmap(0, 0, pix, 0, 0, w, offs[0]);
	    int h = br.height() - offs[0] - (pix.height()-offs[1]);
	    int sh = offs[1]-offs[0];
	    int y = 0;
	    if (offs[1]-offs[0]) {
		for (; y < h-sh; y+=sh)
		    p->drawPixmap(0, offs[0]+y, pix, 0, offs[0], w, sh);
	    }
		p->drawPixmap(0, offs[0]+y, pix, 0, offs[0], w, h-y);
	    p->drawPixmap(0, br.height()-(pix.height()-offs[1]), pix, 0, offs[1], w, pix.height()-offs[1]);
	}
    }
}

void ThemeImageItem::paletteChange(const QPalette &)
{
    if( attribute( "colorRole" ) != QColorGroup::NColorRoles)
	setImage( loadImage(imgName, attribute( "colorRole" ), color( "color" ), attribute( "alpha" ) ) );
    if( attribute( "colorRole", ThemeItem::Pressed ) != QColorGroup::NColorRoles )
	setPixmap( "src", loadImage( pressedImgName, attribute( "colorRole", ThemeItem::Pressed ), 
	color( "color", ThemeItem::Pressed ), attribute( "alpha", ThemeItem::Pressed ) ), ThemeItem::Pressed );
    if( attribute( "colorRole", ThemeItem::Focus ) != QColorGroup::NColorRoles )
	setPixmap( "src", loadImage( focusImgName, attribute( "colorRole", ThemeItem::Focus ), 
	color( "color", ThemeItem::Focus ), attribute( "alpha", ThemeItem::Focus ) ), ThemeItem::Focus );
}

//---------------------------------------------------------------------------

ThemeListItem::ThemeListItem(ThemeItem *p, ThemedView *v, const QXmlAttributes &atts)
    : ThemeGraphicItem( p, v, atts )
{
}

int ThemeListItem::rtti() const
{
    return ThemedView::ListItem;
}

//---------------------------------------------------------------------------

ThemedView::ThemedView(QWidget *parent, const char *name, WFlags f)
    : QWidget(parent, name, f|WRepaintNoErase), pressedItem( 0 ) 
{
    d = new ThemedViewPrivate(this);
    m_themeCache = new ThemeCache;
}

ThemedView::~ThemedView()
{
    delete d;
}

void ThemedView::mousePressEvent( QMouseEvent *e )
{
    QWidget::mousePressEvent( e );
    QPoint p = e->pos();
    //find the closest item to p
    //qDebug("mousePressEvent( %d, %d )", e->pos().x(), e->pos().y() );
    pressedItem = itemAt( p );
    if( pressedItem ) {
	pressedItem->setPressed( TRUE );
	if( !pressedItem->pressed() )
	    pressedItem = 0;
	else
	    emit itemPressed( pressedItem );
    }
}

void ThemedView::mouseMoveEvent( QMouseEvent *e )
{
    ThemeItem *item = itemAt( e->pos() );
    //no item under mouse or an item that is not the initially pressed item
    if( !item || item != pressedItem )
	while( (item = findItem( QString::null, 0, ThemeItem::Pressed)) )
	    item->setPressed( FALSE );
    //item that was initially pressed, set to be pressed if not already
    else if( !item->pressed() )
	item->setPressed( TRUE );
    //else initially pressed item already pressed
}

void ThemedView::mouseReleaseEvent( QMouseEvent *e )
{
    ThemeItem *item = itemAt( e->pos() );
    bool ic = FALSE;
    if( item && item == pressedItem )
	ic = TRUE;
    while( (item = findItem( QString::null, 0, ThemeItem::Pressed )) )
	item->setPressed( FALSE );
    if( ic )
    {
	emit itemClicked( pressedItem );
	pressedItem = 0;
    }
}

void ThemedView::setGeometryAndLayout(int gx, int gy, int w, int h)
{
    if ( w!=width() || h!=height() || gx!=x() || gy!=y() ) {
	setGeometry(gx,gy,w,h);
	layout();
    }
}

ThemeItem *ThemedView::itemAt( const QPoint &pos ) const
{
    if( !d->page )
	return 0;
    return itemAt( pos, d->page );
}

ThemeItem *ThemedView::itemAt( const QPoint &pos, ThemeItem *item ) const
{
    if( !item->chldn.count() )
	return 0;
    ThemeItem *cur = item->chldn.first();
    ThemeItem *bestMatch = 0; //
    while( cur )
    {
	//br is local coordinates, ThemeItem::rect sums the x,y offsets back to top parent
	if( cur->rect().contains( pos )
		&& (!item->transient || item->actv))
	{
	    if( cur->rtti() == ThemedView::Group && cur->isInteractive() && cur->active() && isEnabled( cur ) )
		bestMatch = cur; // current is a group and interactive
	    else 
	    {
		ThemeItem *f = itemAt( pos, cur );
		if( f && f->isInteractive() && f->active() && isEnabled( f ) )
		    bestMatch = f; // an item, deeper than cur
		else if( cur->isInteractive() && cur->active() && isEnabled( cur ) )
		    bestMatch = cur; //cur item is deepest
		else if( f && f->active() && isEnabled( f ) && (!bestMatch || !bestMatch->isInteractive()) )
		    bestMatch = f; // not user items, but some kind of items under the cursor. only set if don't already have an interactive match
		else if( cur->active() && isEnabled( cur ) && (!bestMatch || !bestMatch->isInteractive()) )
		    bestMatch = cur;
		//if it's not a user item, just a rect or a line or something, keep looking for a better match
	    }
	}

	cur = item->chldn.next();
    }
    return bestMatch;
}

bool ThemedView::isEnabled( ThemeItem *item ) const
{
    if( !item )
	return FALSE;
    if( item->rtti() == ThemedView::Status ) 
    {
	ThemeStatusItem *statusItem = (ThemeStatusItem *)item;
	if( !statusItem->enabled() )
	    return FALSE;
    }
    return TRUE;
}

bool ThemedView::sourceLoaded() const
{
    return !m_curSourceFile.isNull();
}

void ThemedView::setSourceFile(const QString &fileName)
{
    m_newSourceFile = fileName;
}

bool ThemedView::setSource(const QString &fileName)
{
    if( !fileName.isEmpty() )
	m_newSourceFile = fileName;
    if( m_newSourceFile.isEmpty() ) {
#ifdef DEBUG
	qDebug("ThemedView::setSource() - No theme file to set.");
#endif
	return FALSE;
    }

    if( d->page )
	delete d->page;
    d->page = 0;
    QFile file(m_newSourceFile);
    if (file.exists()) {
	if( m_curSourceFile != m_newSourceFile )
	    m_themeCache->remove();
	m_curSourceFile = m_newSourceFile;
	m_themeCache->setThemeFile( m_curSourceFile );
	m_themeCache->setPalette( palette() );
	m_themeCache->load( d );
	if( !d->page ) {
	    QXmlInputSource source( file );
	    QXmlSimpleReader reader;
	    reader.setContentHandler( d );
	    reader.setErrorHandler( d );
	    if (!reader.parse( source )){
		qWarning("Unable to parse Theme file %s\n", file.name().latin1());
	    } else {
		themeCache()->save( d );
	    }
	}
	if (d->page && isVisible()) {
	    layout();
	    update();
	} else {
	    d->needLayout = TRUE;
	}
    } else {
	qWarning("Unable to open %s", file.name().latin1());
	return FALSE;
    }
    return TRUE;
}

void ThemedView::paint(QPainter *p, const QRect &clip, ThemeItem *item)
{
    if (!d->page)
	return;
    if (d->needLayout)
	layout();
    if (!item)
	item = d->page;
    p->translate(-clip.x(), -clip.y());
    paintItem(p, item, clip);
    p->translate(clip.x(), clip.y());
}

void ThemedView::paintEvent(QPaintEvent *pe)
{
    if (!d->page)
	return;
    if (d->needLayout)
	layout();
    const QRect &clip = pe->rect();
    QPixmap buffer(clip.size());
    QPainter bp(&buffer);

//    qDebug("PaintRegion: %d, %d, %d x %d", clip.x(), clip.y(), clip.width(), clip.height());

    bp.translate(-clip.x(), -clip.y());
    paintItem(&bp, d->page, clip);

    bitBlt(this, clip.x(), clip.y(), &buffer);
}

void ThemedView::paintItem(QPainter *p, ThemeItem *item, const QRect &clip)
{
    if (item->br.intersects(clip) && (!item->transient || item->actv)
	    && item->visible && item->rtti() != ThemedView::Input) {
	int tx = item->br.x();
	int ty = item->br.y();
	p->translate(tx, ty);
	QRect iclip(tx-clip.x(), ty-clip.y(), item->br.width(), item->br.height());
	p->setClipRect(iclip);
	/*
	qDebug("Paint %s, clip %d, %d, %d x %d", item->name.latin1(),
	    iclip.x(), iclip.y(), iclip.width(), iclip.height());
	*/

	QRect crect(clip);
	crect.moveBy(-item->br.x(), -item->br.y());
	item->paint(p, crect & QRect(0, 0, item->br.width(), item->br.height()));

	QListIterator<ThemeItem> it(item->chldn);
	while (it.current()) {
	    paintItem(p, it.current(), crect);
	    ++it;
	}
	p->translate(-tx, -ty);
    }
}

QSize ThemedView::sizeHint() const
{
    if (d->page)
	return d->page->sizeHint();
    return QWidget::sizeHint();
}

const QString &ThemedView::base() const
{
    return d->page->base();
}

const QString ThemedView::defaultPics() const
{
    return QString("themes/default/");
}

ThemeCache *ThemedView::themeCache()
{
    return m_themeCache;
}

ThemeItem *ThemedView::findItem(const QString &in, int type, int state) const
{
    if (!d->page)
	return 0;
    if (d->needLayout) {
	ThemedView *that = (ThemedView *)this;
	that->layout();
    }

    return findItem(d->page, in, type, state);
}

ThemeItem *ThemedView::findItem(ThemeItem *item, const QString &in, int type, int state) const
{
    if (!item)
	item = d->page;
    bool statematch = (state == ThemeItem::All || state == item->state());
    if ( (in.isNull() || item->name == in) && (!type || type == item->rtti()) && statematch )
	return item;

    QListIterator<ThemeItem> it(item->chldn);
    while (it.current()) {
	ThemeItem *fi = findItem(it.current(), in, type, state);
	if (fi)
	    return fi;
	++it;
    }

    return 0;
}

QValueList<ThemeItem*> ThemedView::findItems(const QString &in, int type, int state) const
{
    QValueList<ThemeItem*> list;
    if (d->page) {
	if (d->needLayout) {
	    ThemedView *that = (ThemedView *)this;
	    that->layout();
	}
	findItems(d->page, in, type,state, list);
    }

    return list;
}

void ThemedView::findItems(ThemeItem *item, const QString &in, int type, int state, QValueList<ThemeItem*> &list) const
{
    if( !item )
	return;
    bool statematch = (state == ThemeItem::All || state == item->state());
    if ( (in.isNull() || item->name == in) && (!type || type == item->rtti()) && statematch )
	list.append(item);

    QListIterator<ThemeItem> it(item->chldn);
    while (it.current()) {
	findItems(it.current(), in, type, state, list);
	++it;
    }
}


void ThemedView::layout(ThemeItem *item)
{
    if (!item) {
	item = d->page;
	d->needLayout = FALSE;
    }
    if( !item )
	return;
    item->layout();
    QListIterator<ThemeItem> it(item->chldn);
    while (it.current()) {
	layout(it.current());
	++it;
    }
}

void ThemedView::paletteChange(const QPalette &p)
{
    if (d->page)
	d->paletteChange(d->page, p);

    m_themeCache->setPalette( p ); // inform the cache that the palette 
				   // the theme is based on has changed
    QWidget::paletteChange( p );
}

void ThemedView::focusInEvent(QFocusEvent *)
{
}

void ThemedView::focusOutEvent(QFocusEvent *)
{
}

void ThemedView::resizeEvent(QResizeEvent *r)
{
    QWidget::resizeEvent(r);
    if (d->page) {
	if (isVisible())
	    layout();
	else
	    d->needLayout = TRUE;
    }
}

void ThemedView::showEvent(QShowEvent *)
{
    if (d->needLayout && d->page)
	layout();
}

// ThemeItem
ThemeItem::ThemeItem(ThemeItem *p, ThemedView *ir)
    : parent(p), view(ir)
{
    init();
}

void ThemeItem::readData( QDataStream &s )
{
    uchar cactv = 0, ctransient = 0, cvisible = 0, cpress = 0, cfocus = 0, cinteractive = 0;
    int irmode = Rect;

    s >> name >> sr >> br >> irmode >> cactv >> ctransient  >> cvisible >> cpress 
      >> cfocus >> cinteractive;
    actv = (bool)cactv;
    transient = (bool)ctransient;
    visible = (bool)cvisible;
    press = (bool)cpress;
    focus = (bool)cfocus;
    rmode = (RMode)irmode;
    interactive = (bool)cinteractive;
    for( int i = 0 ; i < 3 ; ++i )
	s >> intattributes[i];
    for( int i = 0 ; i < 3 ; ++i )
        s >> strattributes[i]; 
    //qDebug("read ***** name=%s sr=%d,%d,%d,%d br=%d,%d,%d,%d, rmode=%d actv=%d  transient=%d visible=%d press=%d focus=%d interactive=%d", name.latin1(), sr.x(), sr.y(), sr.width(), sr.height(), br.x(), br.y(), br.width(), br.height(), rmode, (int)actv, (int)transient, (int)visible, (int)press, (int)focus, (int)interactive);

}

void ThemeItem::writeData( QDataStream &s )
{
    s << name << sr << br << ((int)rmode) << ((uchar)actv) << ((uchar)transient) << ((uchar)visible) << ((uchar)press) << ((uchar)focus) << ((uchar)interactive);
   for( int i = 0 ; i < 3 ; ++i )
       s << intattributes[i];
   for( int i = 0 ; i < 3 ; ++i )
       s << strattributes[i];

    //qDebug("write ***** name=%s sr=%d,%d,%d,%d br=%d,%d,%d,%d, rmode=%d actv=%d  transient=%d visible=%d press=%d focus=%d interactive=%d", name.latin1(), sr.x(), sr.y(), sr.width(), sr.height(), br.x(), br.y(), br.width(), br.height(), rmode, (int)actv, (int)transient, (int)visible, (int)press, (int)focus, (int)interactive);
   /*
    QString name; 
    QRect sr;
    QRect br;
    RMode rmode;
    bool actv;
    bool transient;
    bool visible;
    QList<ThemeItem> chldn;
    bool press;
    bool focus;
    bool interactive;
    QMap<QString,int> intattributes[3];
    QMap<QString,QString> strattributes[3];
    */
}

// ThemePageItem
void ThemePageItem::writeData( QDataStream &s )
{
    ThemeItem::writeData( s );
    int pixMarker = bg.isNull() ? 0 : 1; // Can't stream null images on 2.3 QDataStream
    s << pixMarker;
    if( pixMarker != 0 )
	s << bg;
    s << bd << (uchar)stretch << maskImg << offs[0] << offs[1];

    //qDebug("ThemePageItem::writeData - bd is %s", bd.latin1());
}

ThemePageItem::ThemePageItem(ThemeItem *p, ThemedView *ir)
    : ThemeItem(p, ir)
{

}

void ThemePageItem::readData( QDataStream &s )
{
    ThemeItem::readData( s );
    uchar cstretch = 0;
    int pixMarker = 0;
    s >> pixMarker;
    if( pixMarker != 0 )
	s >> bg;
    s >> bd >> cstretch >> maskImg >> offs[0] >> offs[1];
    stretch = (bool)cstretch;
    //qDebug("ThemePageItem::readData - bd is %s", bd.latin1());
    /*
    QPixmap bg;
    QString bd;
    bool stretch;
    QString maskImg;
    int offs[2];
    */
}

// ThemeGroupItem
// ThemePluginItem

// ThemeLayoutItem

ThemeLayoutItem::ThemeLayoutItem(ThemeItem *p, ThemedView *ir)
    : ThemeItem(p, ir)
{
    init();
}

void ThemeLayoutItem::readData( QDataStream &s )
{
    ThemeItem::readData( s );
    uchar cstretch = 0;
    int iorient = 0;
    s >> iorient >> spacing >> align >> cstretch;
    orient = (Qt::Orientation)iorient;
    stretch = (bool)cstretch;
}

void ThemeLayoutItem::writeData( QDataStream &s )
{
    ThemeItem::writeData( s );
    s << (int)orient << spacing << align << (uchar)stretch;
    /*
    Qt::Orientation orient;
    int spacing;
    int align;
    bool stretch;
    */
}

// ThemeExclusiveItem

// ThemeGraphicItem

ThemeGraphicItem::ThemeGraphicItem(ThemeItem *p, ThemedView *ir)
    : ThemeItem(p, ir)
{

}

void ThemeGraphicItem::readData( QDataStream &s )
{
    ThemeItem::readData( s );
    for( int i = 0 ; i < 3 ; ++i ) {
	s >> colors[i];
	//qDebug("read ***** graphic numcolors=%d", colors[i].count());
    }
    Config cfg("qpe");
    QString fontFamily = cfg.readEntry("FontFamily", "helvetica");
    for( int i = 0 ; i < 3 ; ++i ) {
	s >> fonts[i];
	//qDebug("read ***** graphic numfonts=%d", fonts[i].count());
        //validate that the font family is still valid 
        //if the language has changed we may need a new font
        QMap<QString,QFont>::Iterator it;
        for( it = fonts[i].begin(); it != fonts[i].end(); ++it)
        {
            it.data().setFamily(fontFamily);
        }
    }
}

void ThemeGraphicItem::writeData( QDataStream &s )
{
    ThemeItem::writeData( s );
    for( int i = 0 ; i < 3 ; ++i ) {
	s << colors[i];
	//qDebug("write ***** graphic numcolors=%d", colors[i].count());
    }
    for( int i = 0 ; i < 3 ; ++i ) {
	s << fonts[i];
	//qDebug("write ***** graphic numfonts=%d", fonts[i].count());
    }
/*
    QMap<QString,QColor> colors[3];
    QMap<QString,QFont> fonts[3];
    */
}

// ThemeInputItem
ThemeInputItem::ThemeInputItem(ThemeItem *p, ThemedView *ir)
    : ThemeGraphicItem(p, ir)
{
    init();
}

void ThemeInputItem::readData( QDataStream &s )
{
    ThemeGraphicItem::readData( s );
    s >> colorGroupAtts;
    setupWidget();
}

void ThemeInputItem::writeData( QDataStream &s )
{
    ThemeGraphicItem::writeData( s );
    s << colorGroupAtts;
    /*
    QMap<QString,QString> colorGroupAtts;
    */
}

 // ThemeTextItem
ThemeTextItem::ThemeTextItem(ThemeItem *p, ThemedView *ir)
    : ThemeGraphicItem(p, ir), shadowImg(0)
{

}

void ThemeTextItem::readData( QDataStream &s )
{
    ThemeGraphicItem::readData( s );
    uchar cshortLbl = 0, crichText = 0;
    s >> txt >> cshortLbl >> shadow; 

    int pixMarker = 0;
    s >> pixMarker;
    if( pixMarker ) {
	delete shadowImg;
	shadowImg = new QImage;
	s >> *shadowImg;
    }

    int iformat = 0;
    s >> align >> iformat >> crichText;
    format = (Qt::TextFormat)iformat;

    shortLbl = (bool)cshortLbl;
    richText = (bool)crichText;

    //qDebug("read '%s' text for item '%s'", txt.latin1(), itemName().latin1());
    /*
    QString txt;
    bool shortLbl;
    int shadow;
    QImage *shadowImg;
    int align;
    Qt::TextFormat format;
    bool richText;
    */
}

void ThemeTextItem::writeData( QDataStream &s )
{
    ThemeGraphicItem::writeData( s );
    s << txt << (uchar)shortLbl << shadow;
    int pixMarker = 0;
    if( shadowImg && !shadowImg->isNull() )
       pixMarker = 1;
    s << pixMarker;
    if( pixMarker )
	s << *shadowImg;
    s << align << (int)format << (uchar)richText;
}

// ThemeRectItem
ThemeRectItem::ThemeRectItem(ThemeItem *p, ThemedView *ir)
    : ThemeGraphicItem(p, ir)
{
    d[0] = 0;
    d[1] = 0;
    d[2] = 0;

}

void ThemeRectItem::readData( QDataStream &s )
{
    ThemeGraphicItem::readData( s );
    for( int i = 0 ; i < 3 ; ++ i ) {
	int a = -1;
	s >> a;
	if( a !=-1 ) {
	    if( !d[i] )
		d[i] = new ThemeRectItemPrivate;
	    d[i]->alpha = a;
	    s >> d[i]->brush;
	}
    }
}

void ThemeRectItem::writeData( QDataStream &s )
{
    ThemeGraphicItem::writeData( s );
    for( int i = 0 ; i < 3 ; ++i ) {
	int a = -1;
	if( d[i] )
	    a = d[i]->alpha;
	s << a;
	if( a != -1 && d[i] )
	    s << d[i]->brush;
    }
    /*
    int alpha;
    QBrush brush;
    */
}

// ThemeLineItem

//  ThemeAnimationItem
ThemeAnimationItem::ThemeAnimationItem(ThemeItem *p, ThemedView *ir)
    : ThemePixmapItem(p, ir)
{
    d[0] = 0;
    d[1] = 0;
    d[2] = 0;
}

void ThemeAnimationItem::readData( QDataStream &s )
{
    ThemePixmapItem::readData( s );
    s >> currFrame >> inc;
    //qDebug("read ***** animation currFrame=%d inc=%d", currFrame, inc);
    for( int i = 0 ; i < 3 ; ++i ) {
	int p = -1;
	s >> p;
	if( p != -1 ) {
	    if( !d[i] )
		d[i] = new ThemeAnimationItemPrivate( this, p );
	    else
		d[i]->setPeriod( p );

	    //qDebug("read ***** animation period=%d", d[i]->getPeriod());

	}
    }
    /*
    int currFrame;
    int inc;
    ThemeAnimationItemPrivate *d[3];

    ThemeAnimationItem *anim;
    int tid;
    int period;
    QPixmap pm;
    */
}

void ThemeAnimationItem::writeData( QDataStream &s )
{
    ThemePixmapItem::writeData( s );
    s << currFrame << inc;
    //qDebug("write ***** animation currFrame=%d inc=%d", currFrame, inc);
    for( int i = 0 ; i < 3 ; ++i ) {
	int p = -1;
	if( d[i] ) {
	    p = d[i]->getPeriod();
	    //qDebug("write ***** animation period=%d", d[i]->getPeriod());
	}
	s << p;
    }
}

//ThemeLevelItem
ThemeLevelItem::ThemeLevelItem(ThemeItem *p, ThemedView *ir)
    : ThemeAnimationItem(p, ir), minVal(0), maxVal(0), val(0)
{

}

void ThemeLevelItem::readData( QDataStream &s )
{
    ThemeAnimationItem::readData( s );
    s >> minVal >> maxVal >> val;
    //qDebug("read ***** level %d %d %d' item", minVal, maxVal, val);
    /*
    int minVal;
    int maxVal;
    int val;
    */
}

void ThemeLevelItem::writeData( QDataStream &s )
{
    ThemeAnimationItem::writeData( s );


    s << minVal << maxVal << val;
    //qDebug("write ***** level %d %d %d' item", minVal, maxVal, val);
}

// ThemeStatusItem
ThemeStatusItem::ThemeStatusItem(ThemeItem *p, ThemedView *ir)
    : ThemePixmapItem(p, ir)
{

}

void ThemeStatusItem::readData( QDataStream &s )
{
    ThemePixmapItem::readData( s );
    uchar cenable = 0;
    s >> cenable;
    enable = (bool)cenable;
}

void ThemeStatusItem::writeData( QDataStream &s )
{
    ThemePixmapItem::writeData( s );
    s << (uchar)enable;
}

// ThemeImageItem
ThemeImageItem::ThemeImageItem(ThemeItem *p, ThemedView *ir)
    : ThemePixmapItem(p, ir)
{

}

void ThemeImageItem::readData( QDataStream &s )
{
    ThemePixmapItem::readData( s );
    uchar cstretch = 0, cscale = 0, ctile = 0;
    int isorient = 0;
    s >> offs[0] >> offs[1] >> align >> cstretch >> cscale >> ctile >> isorient >> imgName >> pressedImgName >> focusImgName;
    sorient = (Qt::Orientation)isorient;
    stretch = (bool)cstretch;
    scale = (bool)cscale;
    tile = (bool)ctile;
    /*
    int offs[2];
    bool stretch;
    bool scale;
    bool tile;
    Qt::Orientation sorient;
    QString imgName, pressedImgName, focusImgName;
    */
}

void ThemeImageItem::writeData( QDataStream &s )
{
    ThemePixmapItem::writeData( s );
    s << offs[0] << offs[1] << align << (uchar)stretch << (uchar)scale << (uchar)tile << (int)sorient << imgName << pressedImgName << focusImgName;
}

ThemePixmapItem::ThemePixmapItem(ThemeItem *p, ThemedView *ir)
    : ThemeGraphicItem(p, ir)
{

}

void ThemePixmapItem::readData( QDataStream &s )
{
    ThemeGraphicItem::readData( s );
    for( int i = 0 ; i < 3 ; ++i ) {
	s >> pixmaps[i];
	//qDebug("read ***** pixmap numpixmaps=%d", pixmaps[i].count());
    }
}

void ThemePixmapItem::writeData( QDataStream &s )
{
    ThemeGraphicItem::writeData( s );
    QMap<QString,QPixmap> pixcopy;
    QMap<QString,QPixmap>::ConstIterator it;
    QStringList removekeys;
    for( int i = 0 ; i < 3 ; ++i ) {
	// remove any null pixmaps because 2.3 QDataStream doesn't support them
	pixcopy = pixmaps[i];
	for( it = pixcopy.begin() ; it != pixcopy.end() ; ++it ) {
	    if( it.data().isNull() )
		removekeys.append( it.key() );
	}

	for( QStringList::ConstIterator it = removekeys.begin() ; it != removekeys.end() ; 
											++it )
	    pixcopy.remove( *it );

	//qDebug("write ***** pixmap numpixmaps=%d original=%d", pixcopy.count(), pixmaps[i].count());

	s << pixcopy;

	removekeys.clear();
    }
}

ThemeLineItem::ThemeLineItem(ThemeItem *p, ThemedView *ir)
    : ThemeGraphicItem(p, ir)
{
}

ThemeListItem::ThemeListItem(ThemeItem *p, ThemedView *ir)
    : ThemeGraphicItem(p, ir)
{
}

ThemeExclusiveItem::ThemeExclusiveItem(ThemeItem *p, ThemedView *ir)
    : ThemeItem(p, ir)
{

}

ThemeGroupItem::ThemeGroupItem(ThemeItem *p, ThemedView *ir)
    : ThemeItem(p, ir)
{
}

ThemePluginItem::ThemePluginItem(ThemeItem *p, ThemedView *ir)
    : ThemeItem(p, ir)
{
    init();
}

