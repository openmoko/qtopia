//depot/qt/2.3/tools/qvfb/skin.cpp#2 - edit change 67695 (text)

// get Key_Flip
#define QT_KEYPAD_MODE
#include <qnamespace.h>
#undef QT_KEYPAD_MODE

// FIXME: When building qvfb 2.3.9 against Qt 2.3.2 this fails because Key_Flip isn't defined in Qt 2.3.2
// So create a workaround for that.
#if QT_VERSION < 239
# define Key_Flip 0xffea
#endif

#include <qapplication.h>
#include <qbitmap.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qtimer.h>
#include <qdir.h>
#include <qregexp.h>
#include "skin.h"
#include "qvfb.h"
#include "qvfbview.h"

#include <unistd.h>
#include <stdlib.h>

const int key_repeat_delay = 500;
const int key_repeat_period = 50;
const int joydistance = 10;

class CursorWindow : public QWidget
{
public:
    CursorWindow( const QString& fn, QPoint hot, QWidget *sk);

    void setView(QVFbView*);
    void setPos(QPoint);
    void handleMouseEvent(QEvent *ev);

protected:
    bool event( QEvent *);
    bool eventFilter( QObject*, QEvent *);

private:
    QWidget *mouseRecipient;
    QVFbView *view;
    QWidget *skin;
    QPoint hotspot;
};

QSize Skin::screenSize(const QString &skinFile)
{
    QString _skinFileName = skinFileName(skinFile,0);
    if ( _skinFileName.isEmpty() )
        return QSize(0,0);
    QFile f( _skinFileName );
    f.open( IO_ReadOnly );
    QTextStream ts( &f );
    int viewW=0, viewH=0;
    parseSkinFileHeader(ts, 0, 0, &viewW, &viewH, 0, 0, 0, 0, 0, 0);
    return QSize(viewW,viewH);
}

QString Skin::skinFileName(const QString &skinFile, QString* prefix)
{
    QFileInfo fi(skinFile);
    QString pref;
    QString fn;
    if ( fi.isDir() ) {
	pref = skinFile + "/";
	fn = pref + fi.baseName() + ".skin";
    } else if (fi.isFile()){
	fn = skinFile;
	pref = fi.dirPath() + "/";
    }else if (!skinFile.isNull()){
	qDebug("Skin file \"%s\" not found", skinFile.latin1());
	return "";
    }
    if ( prefix ) *prefix = pref;
    return fn;
}

bool Skin::parseSkinFileHeader(QTextStream& ts,
		    int *viewX1, int *viewY1,
		    int *viewW, int *viewH,
		    int *numberOfAreas,
		    QString* skinImageUpFileName,
		    QString* skinImageDownFileName,
		    QString* skinImageClosedFileName,
		    QString* skinCursorFileName,
		    QPoint* cursorHot)
{
    QString mark;
    ts >> mark;
    if ( mark == "[SkinFile]" ) {
	// New
	int nareas=0;
	while (!nareas) {
	    QString line = ts.readLine();
	    if ( line.isNull() )
		break;
	    if ( line[0] != '#' && !line.isEmpty() ) {
		int eq = line.find('=');
		if ( eq >= 0 ) {
		    QString key = line.left(eq);
		    eq++;
		    while (eq<(int)line.length()-1 && line[eq]==' ')
			eq++;
		    QString value = line.mid(eq);
		    if ( key == "Up" ) {
			if ( skinImageUpFileName ) *skinImageUpFileName = value;
		    } else if ( key == "Down" ) {
			if ( skinImageDownFileName ) *skinImageDownFileName = value;
		    } else if ( key == "Closed" ) {
			if ( skinImageClosedFileName ) *skinImageClosedFileName = value;
		    } else if ( key == "Screen" ) {
			QStringList l = QStringList::split(" ",value);
			if ( viewX1 ) *viewX1 = l[0].toInt();
			if ( viewY1 ) *viewY1 = l[1].toInt();
			if ( viewW ) *viewW = l[2].toInt();
			if ( viewH ) *viewH = l[3].toInt();
		    } else if ( key == "Cursor" ) {
			QStringList l = QStringList::split(" ",value);
			if ( skinCursorFileName ) *skinCursorFileName = l[0];
			if ( cursorHot ) *cursorHot = QPoint(l[1].toInt(),l[2].toInt());
		    } else if ( key == "Areas" ) {
			nareas = value.toInt();
		    }
		} else {
		    qDebug("Broken line: %s",line.latin1());
		}
	    }
	}
	if ( numberOfAreas )
	    *numberOfAreas = nareas;
    } else {
	// Old
	if ( skinImageUpFileName ) *skinImageUpFileName = mark;
	QString s;
	int x,y,w,h,na;
	ts >> s >> x >> y >> w >> h >> na;
	if ( skinImageDownFileName ) *skinImageDownFileName = s;
	if ( viewX1 ) *viewX1 = x;
	if ( viewY1 ) *viewY1 = y;
	if ( viewW ) *viewW = w;
	if ( viewH ) *viewH = h;
	if ( numberOfAreas ) *numberOfAreas = na;
	ts.readLine(); // eol
    }
    return TRUE;
}

Skin::Skin( QVFb *p, const QString &skinFile, int &viewW, int &viewH ) : 
    QWidget(p), view(0), buttonPressed(FALSE), buttonIndex(0), skinValid(FALSE),
    zoom(1.0), numberOfAreas(0), areas(0),
    cursorw(0),
    joystick(-1), joydown(0),
    flipped_open(TRUE)
{
    setMouseTracking(TRUE);
    setBackgroundMode(NoBackground);
    parent = p;

    QString _skinFileName = skinFileName(skinFile,&prefix);
    if ( _skinFileName.isEmpty() ) {
        skinValid = FALSE;
        return;
    }
    QFile f( _skinFileName );
    if (!f.exists()) {
        qWarning("failed to find %s", _skinFileName.ascii());
        skinValid = FALSE;
        return;
    }
    f.open( IO_ReadOnly );
    QTextStream ts( &f );
    parseSkinFileHeader(ts, &viewX1, &viewY1, &viewW, &viewH, &numberOfAreas,
	&skinImageUpFileName, &skinImageDownFileName, &skinImageClosedFileName,
	&skinCursorFileName, &cursorHot);

//  Debug the skin file parsing
//  printf("read: -%s- -%i- -%i- -%i-\n", skinImage.latin1(), viewX1, viewY1, numberOfAreas );
    areas = new ButtonAreas[numberOfAreas];

    skinImageUpFileName = prefix + skinImageUpFileName;
    skinImageDownFileName = prefix + skinImageDownFileName;
    if ( !skinImageClosedFileName.isEmpty() )
	skinImageClosedFileName = prefix + skinImageClosedFileName;
    if ( !skinCursorFileName.isEmpty() )
	skinCursorFileName = prefix + skinCursorFileName;

    int i = 0;
    joystick = -1;
    while (i < numberOfAreas) {
	QString line = ts.readLine();
	if ( line.isNull() )
	    break;
	if ( line[0] != '#' && !line.isEmpty() ) {
	    QStringList tok = QStringList::split(QRegExp("[ \t][ \t]*"),line);
	    if ( tok.count()<6 ) {
		qDebug("Broken line: %s",line.latin1());
	    } else {
		areas[i].name = tok[0];
		QString k = tok[1];
		if ( k.left(2).lower() == "0x" ) {
		    areas[i].keyCode = k.mid(2).toInt(0,16);
		} else {
		    areas[i].keyCode = k.toInt();
		}

		int p=0;
		for (int j=2; j<(int)tok.count(); ) {
		    int x = tok[j++].toInt();
		    int y = tok[j++].toInt();
		    areas[i].area.putPoints(p++,1,x,y);
		}

		if ( areas[i].name[0] == '"' && areas[i].name.right(1) == "\"" )
		    areas[i].name = areas[i].name.mid(1,areas[i].name.length()-2);
		if ( areas[i].name.length() == 1 )
		    areas[i].text = areas[i].name;
		if ( areas[i].name == "Joystick" )
		    joystick = i;
		i++;
	    }
	}
    }
    setZoom(1.0);

    t_skinkey = new QTimer( this );
    connect( t_skinkey, SIGNAL(timeout()), this, SLOT(skinKeyRepeat()) );
    t_parentmove = new QTimer( this );
    connect( t_parentmove, SIGNAL(timeout()), this, SLOT(moveParent()) );

    skinValid = TRUE;
}

void Skin::skinKeyRepeat()
{
    if ( view ) {
	view->skinKeyReleaseEvent( areas[buttonIndex].keyCode, areas[buttonIndex].text, TRUE );
	view->skinKeyPressEvent( areas[buttonIndex].keyCode, areas[buttonIndex].text, TRUE );
	t_skinkey->start(key_repeat_period);
    }
}

void Skin::calcRegions()
{
    for (int i=0; i<numberOfAreas; i++) {
	QPointArray xa(areas[i].area.count());
	int n = areas[i].area.count();
	for (int p=0; p<n; p++) {
	    xa.setPoint(p,
		int(areas[i].area[p].x()*zoom),
		int(areas[i].area[p].y()*zoom));
	}
	if ( n == 2 ) {
	    areas[i].region = QRegion(xa.boundingRect());
	} else {
	    areas[i].region = QRegion(xa);
	}
    }
}

void Skin::loadImages()
{
    QImage iup(skinImageUpFileName);
    QImage idown(skinImageDownFileName);
    QImage iclosed(skinImageClosedFileName);
    QImage icurs(skinCursorFileName);

    if ( zoom != int(zoom) ) {
	iup = iup.smoothScale(int(iup.width()*zoom),int(iup.height()*zoom));
	idown = idown.smoothScale(int(idown.width()*zoom),int(idown.height()*zoom));
	if ( !skinImageClosedFileName.isEmpty() )
	    iclosed = iclosed.smoothScale(int(idown.width()*zoom),int(idown.height()*zoom));
	if ( !skinCursorFileName.isEmpty() )
	    icurs = icurs.smoothScale(int(idown.width()*zoom),int(idown.height()*zoom));
    }
    int conv = ThresholdAlphaDither|AvoidDither;
    //int conv = -1;
    skinImageUp.convertFromImage(iup, conv);
    skinImageDown.convertFromImage(idown, conv);
    if ( !skinImageClosedFileName.isEmpty() )
	skinImageClosed.convertFromImage(iclosed, conv);
    if ( !skinCursorFileName.isEmpty() )
	skinCursor.convertFromImage(icurs, conv);

    if ( zoom == int(zoom) ) {
	QWMatrix scale; scale = scale.scale(zoom,zoom);
	skinImageUp = skinImageUp.xForm(scale);
	skinImageDown = skinImageDown.xForm(scale);
	if ( !skinImageClosedFileName.isEmpty() )
	    skinImageClosed = skinImageClosed.xForm(scale);
	if ( !skinCursorFileName.isEmpty() )
	    skinCursor = skinCursor.xForm(scale);
    }

    setFixedSize( skinImageUp.size() );
    if (!skinImageUp.mask())
	skinImageUp.setMask(skinImageUp.createHeuristicMask());
    if (!skinImageClosed.mask())
	skinImageClosed.setMask(skinImageClosed.createHeuristicMask());

    QWidget* parent = parentWidget();
    parent->setMask( *skinImageUp.mask() );
    parent->setFixedSize( skinImageUp.size() );

    delete cursorw;
    cursorw = 0;
    if ( !skinCursorFileName.isEmpty() ) {
	cursorw = new CursorWindow(skinCursorFileName,cursorHot,this);
	if ( view )
	    cursorw->setView(view);
    }
}

Skin::~Skin( )
{
    delete cursorw;
}

void Skin::setZoom( double z )
{
    zoom = z;
    calcRegions();
    loadImages();
    if ( view )
	view->move( int(viewX1*zoom), int(viewY1*zoom) );
}

void Skin::setView( QVFbView *v )
{
    view = v;
    view->setFocus();
    view->move( int(viewX1*zoom), int(viewY1*zoom) );
    if ( cursorw )
	cursorw->setView(v);

    qDebug( "setView" );
    setupDefaultButtons();
}


void Skin::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    if ( flipped_open ) {
	p.drawPixmap( 0, 0, skinImageUp );
	if (buttonPressed == TRUE) {
	    ButtonAreas *ba = &areas[buttonIndex];
	    QRect r = ba->region.boundingRect();
	    if ( ba->area.count() > 2 )
		p.setClipRegion(ba->region);
	    p.drawPixmap( r.topLeft(), skinImageDown, r);
	}
    } else {
	p.drawPixmap( 0, 0, skinImageClosed );
    }
}


void Skin::mousePressEvent( QMouseEvent *e )
{
    if (e->button() == RightButton) {
	parent->popupMenu();
    } else {
	buttonPressed = FALSE;

	onjoyrelease = -1;
	if ( !flipped_open ) {
	    flip(TRUE);
	} else {
	    for (int i = 0; i < numberOfAreas; i++) {
		ButtonAreas *ba = &areas[i];
		if ( ba->region.contains( e->pos() ) ) {
		    if ( joystick == i ) {
			joydown = TRUE;
		    } else {
			if ( joydown )
			    onjoyrelease = i;
			else
			    startPress(i);
			break;
//		Debug message to be sure we are clicking the right areas
//		printf("%s clicked\n", areas[i].name);
		    }
		}
	    }
	}
	
//	This is handy for finding the areas to define rectangles for new skins
//	printf("Clicked in %i,%i\n",  e->pos().x(),  e->pos().y());
	clickPos = e->pos();
    }
}

void Skin::flip(bool open)
{
    if ( flipped_open == open )
	return;
    if ( open ) {
	parent->setMask( *skinImageUp.mask() );
	view->skinKeyReleaseEvent( Qt::Key(Key_Flip), "Flip" );
    } else {
	parent->setMask( *skinImageClosed.mask() );
	view->skinKeyPressEvent( Qt::Key(Key_Flip), "Flip" );
    }
    flipped_open = open;
    repaint(FALSE);
}

void Skin::startPress(int i)
{
    buttonPressed = TRUE;
    buttonIndex = i;
    if (view) {
	if ( areas[buttonIndex].keyCode == Key_Flip ) {
	    flip(FALSE);
	} else {
	    view->skinKeyPressEvent( areas[buttonIndex].keyCode, areas[buttonIndex].text );
	    t_skinkey->start(key_repeat_delay);
	}
	ButtonAreas *ba = &areas[buttonIndex];
	repaint( ba->region.boundingRect(), FALSE );
    }
}

void Skin::endPress()
{
    if (view && areas[buttonIndex].keyCode != Key_Flip )
	view->skinKeyReleaseEvent( areas[buttonIndex].keyCode, areas[buttonIndex].text );
    t_skinkey->stop();
    buttonPressed = FALSE;
    ButtonAreas *ba = &areas[buttonIndex];
    repaint( ba->region.boundingRect(), FALSE );
}

void Skin::mouseMoveEvent( QMouseEvent *e )
{
    if ( e->state() & LeftButton ) {
	QPoint newpos =  e->globalPos() - clickPos;
	if ( joydown ) {
	    int k1=0, k2=0;
	    if ( newpos.x() < -joydistance ) {
		k1 = joystick+1;
	    } else if ( newpos.x() > +joydistance ) {
		k1 = joystick+3;
	    }
	    if ( newpos.y() < -joydistance ) {
		k2 = joystick+2;
	    } else if ( newpos.y() > +joydistance ) {
		k2 = joystick+4;
	    }
	    if ( k1 || k2 ) {
		if ( !buttonPressed ) {
		    onjoyrelease = -1;
		    if ( k1 && k2 ) {
			startPress(k2);
			endPress();
		    }
		    startPress(k1 ? k1 : k2);
		}
	    } else if ( buttonPressed ) {
		endPress();
	    }
	} else if ( buttonPressed == FALSE ) {
	    parentpos = newpos;
	    if ( !t_parentmove->isActive() )
		t_parentmove->start(50,TRUE);
	}
    }
    if ( cursorw )
	cursorw->setPos(e->globalPos());
}

void Skin::moveParent()
{
    parent->move( parentpos );
}

void Skin::mouseReleaseEvent( QMouseEvent * )
{
    if ( buttonPressed )
	endPress();
    if ( joydown ) {
	joydown = FALSE;
	if ( onjoyrelease >= 0 ) {
	    startPress(onjoyrelease);
	    endPress();
	}
    }
}


bool Skin::hasCursor() const
{
    return !skinCursorFileName.isEmpty();
}

void Skin::setupDefaultButtons()
{
    QString destDir = QString("/tmp/qtembedded-%1/").arg(view->displayId());
    QFileInfo src(prefix + "defaultbuttons.conf");
    QFileInfo dst(destDir + "defaultbuttons.conf");
    unlink(dst.absFilePath().latin1());
    if (src.exists()) {
	QString srcFile = src.absFilePath();
	QString origDir = QDir::current().absPath();
	QDir::setCurrent(destDir);
	symlink(srcFile.latin1(), dst.fileName().latin1());
	QDir::setCurrent(origDir);
    }
}

// ====================================================================

bool CursorWindow::eventFilter( QObject *, QEvent *ev)
{
    handleMouseEvent(ev);
    return FALSE;
}

bool CursorWindow::event( QEvent *ev )
{
    handleMouseEvent(ev);
    return QWidget::event(ev);
}

void CursorWindow::handleMouseEvent(QEvent *ev)
{
    static int inhere=0;
    if ( !inhere ) {
	inhere++;
	if ( view ) {
	    if ( ev->type() >= QEvent::MouseButtonPress && ev->type() <= QEvent::MouseMove ) {
		QMouseEvent *e = (QMouseEvent*)ev;
		QPoint gp = e->globalPos();
		QPoint vp = view->mapFromGlobal(gp);
		QPoint sp = skin->mapFromGlobal(gp);
		if ( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick ) {
		    if ( view->rect().contains(vp) )
			mouseRecipient = view->viewport();
		    else if ( skin->parentWidget()->geometry().contains(gp) )
			mouseRecipient = skin;
		    else
			mouseRecipient = 0;
		}
		if ( mouseRecipient ) {
		    QMouseEvent me(e->type(),mouseRecipient==skin ? sp : vp,gp,e->button(),e->state());
		    QApplication::sendEvent(mouseRecipient, &me);
		    setPos(gp);
		} else if ( !skin->parentWidget()->geometry().contains(gp) ) {
		    hide();
		} else {
		    setPos(gp);
		}
		if ( e->type() == QEvent::MouseButtonRelease )
		    mouseRecipient = 0;
	    }
	}
	inhere--;
    }
}

void CursorWindow::setView(QVFbView* v)
{
    if ( view ) {
	view->removeEventFilter(this);
	view->viewport()->removeEventFilter(this);
    }
    view = v;
    view->installEventFilter(this);
    view->viewport()->installEventFilter(this);
    mouseRecipient = 0;
}

CursorWindow::CursorWindow( const QString& fn, QPoint hot, QWidget* sk)
	:QWidget(0,0, Qt::WStyle_Customize|Qt::WStyle_NoBorder),
	view(0), skin(sk),
	hotspot(hot)
{
    mouseRecipient = 0;
    setMouseTracking(TRUE);
    setCursor(BlankCursor);
    QImage img( fn );
    QPixmap p;
    p.convertFromImage( img );
    if ( !p.mask() )
	if ( img.hasAlphaBuffer() ) {
	    QBitmap bm;
	    bm = img.createAlphaMask();
	    p.setMask( bm );
	} else {
	    QBitmap bm;
	    bm = img.createHeuristicMask();
	    p.setMask( bm );
	}
    setBackgroundPixmap( p );
    setFixedSize( p.size() );
    if ( p.mask() )
	setMask( *p.mask() );
}

void CursorWindow::setPos(QPoint p)
{
    move(p-hotspot);
    show();
    raise();
}
