/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qcursor.h>

#include <qprogressbar.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qfile.h>

#include "qvfb.h"
#include "qvfbview.h"
#include "qvfbratedlg.h"
#include "config.h"
#include "skin.h"
#include "qanimationwriter.h"

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

// =====================================================================

static const char * logo[] = {
/* width height ncolors chars_per_pixel */
"50 50 17 1",
/* colors */
"  c #000000",
". c #495808",
"X c #2A3304",
"o c #242B04",
"O c #030401",
"+ c #9EC011",
"@ c #93B310",
"# c #748E0C",
"$ c #A2C511",
"% c #8BA90E",
"& c #99BA10",
"* c #060701",
"= c #181D02",
"- c #212804",
"; c #61770A",
": c #0B0D01",
"/ c None",
/* pixels */
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$@;.o=::=o.;@$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X*         **X#+$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$#oO*         O  **o#+$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$&.* OO              O*.&$$$$$$$$$$$$$",
"$$$$$$$$$$$$@XOO            * OO    X&$$$$$$$$$$$$",
"$$$$$$$$$$$@XO OO  O  **:::OOO OOO   X@$$$$$$$$$$$",
"$$$$$$$$$$&XO      O-;#@++@%.oOO      X&$$$$$$$$$$",
"$$$$$$$$$$.O  :  *-#+$$$$$$$$+#- : O O*.$$$$$$$$$$",
"$$$$$$$$$#*OO  O*.&$$$$$$$$$$$$+.OOOO **#$$$$$$$$$",
"$$$$$$$$+-OO O *;$$$$$$$$$$$&$$$$;*     o+$$$$$$$$",
"$$$$$$$$#O*  O .+$$$$$$$$$$@X;$$$+.O    *#$$$$$$$$",
"$$$$$$$$X*    -&$$$$$$$$$$@- :;$$$&-    OX$$$$$$$$",
"$$$$$$$@*O  *O#$$$$$$$$$$@oOO**;$$$#    O*%$$$$$$$",
"$$$$$$$;     -+$$$$$$$$$@o O OO ;+$$-O   *;$$$$$$$",
"$$$$$$$.     ;$$$$$$$$$@-OO OO  X&$$;O    .$$$$$$$",
"$$$$$$$o    *#$$$$$$$$@o  O O O-@$$$#O   *o$$$$$$$",
"$$$$$$+=    *@$$$$$$$@o* OO   -@$$$$&:    =$$$$$$$",
"$$$$$$+:    :+$$$$$$@-      *-@$$$$$$:    :+$$$$$$",
"$$$$$$+:    :+$$$$$@o* O    *-@$$$$$$:    :+$$$$$$",
"$$$$$$$=    :@$$$$@o*OOO      -@$$$$@:    =+$$$$$$",
"$$$$$$$-    O%$$$@o* O O    O O-@$$$#*   OX$$$$$$$",
"$$$$$$$. O *O;$$&o O*O* *O      -@$$;    O.$$$$$$$",
"$$$$$$$;*   Oo+$$;O*O:OO--      Oo@+=    *;$$$$$$$",
"$$$$$$$@*  O O#$$$;*OOOo@@-O     Oo;O*  **@$$$$$$$",
"$$$$$$$$X* OOO-+$$$;O o@$$@-    O O     OX$$$$$$$$",
"$$$$$$$$#*  * O.$$$$;X@$$$$@-O O        O#$$$$$$$$",
"$$$$$$$$+oO O OO.+$$+&$$$$$$@-O         o+$$$$$$$$",
"$$$$$$$$$#*    **.&$$$$$$$$$$@o      OO:#$$$$$$$$$",
"$$$$$$$$$+.   O* O-#+$$$$$$$$+;O    OOO:@$$$$$$$$$",
"$$$$$$$$$$&X  *O    -;#@++@#;=O    O    -@$$$$$$$$",
"$$$$$$$$$$$&X O     O*O::::O      OO    Oo@$$$$$$$",
"$$$$$$$$$$$$@XOO                  OO    O*X+$$$$$$",
"$$$$$$$$$$$$$&.*       **  O      ::    *:#$$$$$$$",
"$$$$$$$$$$$$$$$#o*OO       O    Oo#@-OOO=#$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X:* *     O**X#+$$@-*:#$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$%;.o=::=o.#@$$$$$$@X#$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$+$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
};

static const char *red_on_led_xpm[] = {
"11 11 10 1",
" 	c None",
".	c #FF0000",
"+	c #FF4C4C",
"@	c #FF7A7A",
"#	c #D30000",
"$	c #FF9393",
"%	c #BA0000",
"&	c #FFFFFF",
"*	c #7F0000",
"=	c #000000",
"           ",
"   .++@@   ",
"  .....+@  ",
" ##...$.+@ ",
" %#..$&$.+ ",
" *#...$..+ ",
" *%#...... ",
" =*%#..... ",
"  =*%###.  ",
"   ===*.   ",
"           "};

static const char *red_off_led_xpm[] = {
"11 11 12 1",
" 	c None",
".	c #CDB7B4",
"+	c #D2BFBD",
"@	c #DBCBCA",
"#	c #E5D9D8",
"$	c #BC9E9B",
"%	c #E2D6D5",
"&	c #AD8986",
"*	c #FFFFFF",
"=	c #A8817D",
"-	c #B2908D",
";	c #6F4D4A",
"           ",
"   .++@#   ",
"  .....@#  ",
" $$...%.@# ",
" &$..%*%.@ ",
" =-...%..+ ",
" =&-...... ",
" ;==-..... ",
"  ;=&-$$.  ",
"   ;==&$   ",
"           "};

// =====================================================================

class AnimationSaveWidget : public QVBox {
    Q_OBJECT
public:
    AnimationSaveWidget(QVFbView *v);
    ~AnimationSaveWidget();
    bool detectPpmtoMpegCommand();
    void timerEvent(QTimerEvent *te);
    void convertToMpeg(QString filename);
    void removeTemporaryFiles();
protected slots:
    void toggleRecord();
    void reset();
    void save();
private:
    QVFbView *view;
    QProgressBar *progressBar;
    QLabel *statusText;
    bool haveMpeg, savingAsMpeg, recording;
    QCheckBox *mpegSave;
    QAnimationWriter *animation;
    QPushButton *recBt, *resetBt, *saveBt;
    QLabel *timeDpy, *recLED;
    int timerId, progressTimerId;
    QPixmap recOn, recOff;
    QTime tm;
    int elapsed, imageNum;
};

// =====================================================================

Zoomer::Zoomer(QVFb* target) :
    qvfb(target)
{
    (new QVBoxLayout(this))->setAutoAdd(TRUE);
    QSlider *sl = new QSlider(10,64,1,32,Horizontal,this);
    connect(sl,SIGNAL(valueChanged(int)),this,SLOT(zoom(int)));
    label = new QLabel(this);
}

void Zoomer::zoom(int z)
{
    double d = (double)z/32.0;
    qvfb->setZoom(d);
    label->setText(QString::number(d,'g',2));
}

// =====================================================================

QVFb::QVFb( int display_id, int w, int h, int d, int r, const QString &skin, bool useMap, QWidget *parent,
	    const char *name, uint flags )
    : QMainWindow( parent, name, flags )
{
    currentSkinIndex = -1;
    findSkins(skin);
    zoomer = 0;
    imagesave = new QFileDialog(".", "*.png", this, 0, TRUE);
    imagesave->setSelection("snapshot.png");
    setIcon(QPixmap(logo));
    rateDlg = 0;
    view = 0;
#if QT_VERSION >= 0x030000
    // When compiling with Qt 3 we need to create the menu first to
    // avoid scroll bars in the main window
    createMenu( menuBar() );
    init( display_id, w, h, d, r, skin, useMap );
    enableCursor( TRUE );
#else
    init( display_id, w, h, d, r, skin, useMap );
    createMenu( menuBar() );
#endif
}

QVFb::~QVFb()
{
}

void QVFb::popupMenu()
{
    QPopupMenu *pm = new QPopupMenu( this );
    createMenu( pm );
    pm->exec(QCursor::pos());
}

void QVFb::init( int display_id, int pw, int ph, int d, int r, const QString& skin_name, bool useMap )
{
    setCaption( QString("Virtual framebuffer %1x%2 %3bpp Display :%4 Rotate %5")
		    .arg(pw).arg(ph).arg(d).arg(display_id).arg(r) );
    delete view;
    view = 0;
    skin = 0;
    skinscaleH = skinscaleV = 1.0;
    QVFbView::Rotation rot = ((r ==  90) ? QVFbView::Rot90  :
			     ((r == 180) ? QVFbView::Rot180 :
			     ((r == 270) ? QVFbView::Rot270 :
					   QVFbView::Rot0 )));
    if ( !skin_name.isEmpty() ) {
	bool vis = isVisible();
	int sw, sh;
	skin = new Skin( this, skin_name, sw, sh );
	if (skin && skin->isValid()){
            if ( !pw ) pw = sw;
            if ( !ph ) ph = sh;
    	    if ( vis ) hide();
    	    menuBar()->hide();
	    view = new QVFbView( display_id, pw, ph, d, rot, useMap, skin );
	    skin->setView( view );
	    view->setMargin( 0 );
	    view->setFrameStyle( QFrame::NoFrame );
	    view->setFixedSize( sw, sh );
	    setCentralWidget( skin );
	    adjustSize();
	    skinscaleH = (double)sw/pw;
	    skinscaleV = (double)sh/ph;
	    if ( skinscaleH != 1.0 || skinscaleH != 1.0 )
		setZoom(skinscaleH);
	    view->show();
	    if ( vis ) show();
	} else {	    
	    delete skin;
	    skin = 0;
	}
    }

    // If we failed to get a skin or we were not supplied
    //	    with one then fallback to a framebuffer without
    //	    a skin
    if (!skin){
	// Default size
	if ( !pw ) pw = 240;
	if ( !ph ) ph = 320;

     	if ( currentSkinIndex!=-1 ) {
	    clearMask();
	    reparent( 0, 0, pos(), TRUE );
	    //unset fixed size:
	    setMinimumSize(0,0);
	    setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
	}
	menuBar()->show();
	view = new QVFbView( display_id, pw, ph, d, rot, useMap, this );
	view->setMargin( 0 );
	view->setFrameStyle( QFrame::NoFrame );
	setCentralWidget( view );
#if QT_VERSION >= 0x030000
	ph += 2;					// avoid scrollbar
#endif
	resize(pw,menuBar()->height()+ph);
	view->show();
	// delete defaultbuttons.conf if it was left behind...
	unlink(QFileInfo(QString("/tmp/qtembedded-%1/defaultbuttons.conf").arg(view->displayId())).absFilePath().latin1());
    }
}

void QVFb::enableCursor( bool e )
{
    if ( skin && skin->hasCursor() ) {
	view->viewport()->setCursor( BlankCursor );
    } else {
	view->viewport()->setCursor( e ? ArrowCursor : BlankCursor );
    }
    viewMenu->setItemChecked( cursorId, e );
}

void QVFb::createMenu(QMenuData *menu)
{
    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( "&Configure...", this, SLOT(configure()), ALT+CTRL+Key_C );
    file->insertSeparator();
    file->insertItem( "&Save image...", this, SLOT(saveImage()), ALT+CTRL+Key_S );
    file->insertItem( "&Animation...", this, SLOT(toggleAnimation()), ALT+CTRL+Key_A );
    file->insertSeparator();
    file->insertItem( "&Quit", qApp, SLOT(quit()) );

    menu->insertItem( "&File", file );

    viewMenu = new QPopupMenu( this );
    viewMenu->setCheckable( true );
    cursorId = viewMenu->insertItem( "Show &Cursor", this, SLOT(toggleCursor()) );
    if ( view )	
	enableCursor(TRUE);
    viewMenu->insertItem( "&Refresh Rate...", this, SLOT(changeRate()) );
    viewMenu->insertSeparator();
    viewMenu->insertItem( "Zoom scale &0.5", this, SLOT(setZoomHalf()) );
    viewMenu->insertItem( "Zoom scale 0.75", this, SLOT(setZoom075()) );
    viewMenu->insertItem( "Zoom scale &1", this, SLOT(setZoom1()) );
    viewMenu->insertItem( "Zoom scale &2", this, SLOT(setZoom2()) );
    viewMenu->insertItem( "Zoom scale &3", this, SLOT(setZoom3()) );
    viewMenu->insertItem( "Zoom scale &4", this, SLOT(setZoom4()) );
    viewMenu->insertSeparator();
    viewMenu->insertItem( "Zoom scale...", this, SLOT(setZoom()) );

    menu->insertItem( "&View", viewMenu );

    QPopupMenu *help = new QPopupMenu( this );
    help->insertItem("About...", this, SLOT(about()));
    menu->insertSeparator();
    menu->insertItem( "&Help", help );
}

void QVFb::setZoom(double z)
{
    view->setZoom(z,z*skinscaleV/skinscaleH);
    if (skin) {
	skin->setZoom(z/skinscaleH);
	view->setFixedSize(
	    int(view->displayWidth()*z),
	    int(view->displayHeight()*z*skinscaleV/skinscaleH));
    }
}

void QVFb::setZoomHalf()
{
    setZoom(0.5);
}

void QVFb::setZoom075()
{
    setZoom(0.75);
}

void QVFb::setZoom1()
{
    setZoom(1);
}

void QVFb::setZoom()
{
    if ( !zoomer )
	zoomer = new Zoomer(this);
    zoomer->show();
}

void QVFb::setZoom2()
{
    setZoom(2);
}

void QVFb::setZoom3()
{
    setZoom(3);
}

void QVFb::setZoom4()
{
    setZoom(4);
}

void QVFb::saveImage()
{
    QImage img = view->image();
    if ( imagesave->exec() ) {
	QString filename = imagesave->selectedFile();
	if ( !!filename )
	    img.save(filename,"PNG");
    }
}

void QVFb::toggleAnimation()
{
    static AnimationSaveWidget *animWidget = 0;
    if ( !animWidget )
	animWidget = new AnimationSaveWidget(view);
    if ( animWidget->isVisible() )
	animWidget->hide();
    else
	animWidget->show();
}

void QVFb::toggleCursor()
{
    enableCursor( !viewMenu->isItemChecked( cursorId ) );
}

void QVFb::changeRate()
{
    if ( !rateDlg ) {
	rateDlg = new QVFbRateDialog( view->rate(), this );
	connect( rateDlg, SIGNAL(updateRate(int)), view, SLOT(setRate(int)) );
    }

    rateDlg->show();
}

void QVFb::about()
{
    QMessageBox::about(this, "About QVFB",
	"<h2>The Qt/Embedded Virtual X11 Framebuffer</h2>"
	"<p>This application runs under Qt/X11, emulating a simple framebuffer, "
	"which the Qt/Embedded server and clients can attach to just as if "
	"it was a hardware Linux framebuffer. "
	"<p>With the aid of this development tool, you can develop Qt/Embedded "
	"applications under X11 without having to switch to a virtual console. "
	"This means you can comfortably use your other development tools such "
	"as GUI profilers and debuggers."
    );
}

void QVFb::findSkins(const QString &currentSkin)
{
    skinnames.clear();
    skinfiles.clear();
    QDir dir(".","*.skin");
    const QFileInfoList *l = dir.entryInfoList();
    QFileInfo* fi;
    int i = 1; // "None" is already in list at index 0
    for (QFileInfoListIterator it(*l); (fi=*it); ++it) {
	skinnames.append(fi->baseName()); // should perhaps be in file
	skinfiles.append(fi->filePath());
	if ((fi->baseName() + ".skin") == currentSkin)
	    currentSkinIndex = i;
	i++;
    }
}

void QVFb::configure()
{
    config = new Config(this,0,TRUE);

    int w = view->displayWidth();
    int h = view->displayHeight();

    // Need to block signals, because we connect to animateClick(),
    // since QCheckBox doesn't have setChecked(bool) in 2.x.
    chooseSize(QSize(w,h));
    config->skin->insertStringList(skinnames);
    if (currentSkinIndex > 0)
	config->skin->setCurrentItem(currentSkinIndex);
    config->touchScreen->setChecked(view->touchScreenEmulation());
    config->lcdScreen->setChecked(view->lcdScreenEmulation());
    config->depth_1->setChecked(view->displayDepth()==1);
    config->depth_4gray->setChecked(view->displayDepth()==4);
    config->depth_8->setChecked(view->displayDepth()==8);
    config->depth_12->setChecked(view->displayDepth()==12);
    config->depth_16->setChecked(view->displayDepth()==16);
    config->depth_18->setChecked(view->displayDepth()==18);
    config->depth_32->setChecked(view->displayDepth()==32);
    connect(config->skin, SIGNAL(activated(int)), this, SLOT(skinConfigChosen(int)));
    if ( view->gammaRed() == view->gammaGreen() && view->gammaGreen() == view->gammaBlue() ) {
	config->gammaslider->setValue(int(view->gammaRed()*400));
	config->rslider->setValue(100);
	config->gslider->setValue(100);
	config->bslider->setValue(100);
    } else {
	config->gammaslider->setValue(100);
	config->rslider->setValue(int(view->gammaRed()*400));
	config->gslider->setValue(int(view->gammaGreen()*400));
	config->bslider->setValue(int(view->gammaBlue()*400));
    }
    connect(config->gammaslider, SIGNAL(valueChanged(int)), this, SLOT(setGamma400(int)));
    connect(config->rslider, SIGNAL(valueChanged(int)), this, SLOT(setR400(int)));
    connect(config->gslider, SIGNAL(valueChanged(int)), this, SLOT(setG400(int)));
    connect(config->bslider, SIGNAL(valueChanged(int)), this, SLOT(setB400(int)));
    updateGammaLabels();

    double ogr=view->gammaRed(), ogg=view->gammaGreen(), ogb=view->gammaBlue();
    hide();
    if ( config->exec() ) {
	int id = view->displayId(); // not settable yet
	if ( config->size_176_220->isChecked() ) {
	    w=176; h=220;
	} else if ( config->size_240_320->isChecked() ) {
	    w=240; h=320;
	} else if ( config->size_320_240->isChecked() ) {
	    w=320; h=240;
	} else if ( config->size_640_480->isChecked() ) {
	    w=640; h=480;
	} else {
	    w=config->size_width->value();
	    h=config->size_height->value();
	}
	int d;
	if ( config->depth_1->isChecked() )
	    d=1;
	else if ( config->depth_4gray->isChecked() )
	    d=4;
	else if ( config->depth_8->isChecked() )
	    d=8;
	else if ( config->depth_12->isChecked() )
	    d=12;
	else if ( config->depth_16->isChecked() )
	    d=16;
	else if ( config->depth_18->isChecked() )
	    d=18;
	else
	    d=32;
	int skinIndex = config->skin->currentItem();
	if ( w != view->displayWidth() || h != view->displayHeight()
		|| d != view->displayDepth() || skinIndex != currentSkinIndex ) {
	    QVFbView::Rotation rot = view->displayRotation();
	    int r = ((rot == QVFbView::Rot90)  ?  90 :
		    ((rot == QVFbView::Rot180) ? 180 :
		    ((rot == QVFbView::Rot270) ? 270 : 0 )));
	    currentSkinIndex = skinIndex;
	    init( id, w, h, d, r, skinIndex > 0 ? skinfiles[skinIndex-1] : QString::null, false );
	}
	view->setTouchscreenEmulation( config->touchScreen->isChecked() );
	bool lcdEmulation = config->lcdScreen->isChecked();
	view->setLcdScreenEmulation( lcdEmulation );
	if ( lcdEmulation )
	    setZoom3();
    } else {
	view->setGamma(ogr, ogg, ogb);
    }
    show();
    delete config;
    config=0;
}

void QVFb::chooseSize(const QSize& sz)
{
    config->size_width->blockSignals(TRUE);
    config->size_height->blockSignals(TRUE);
    config->size_width->setValue(sz.width());
    config->size_height->setValue(sz.height());
    config->size_width->blockSignals(FALSE);
    config->size_height->blockSignals(FALSE);
    config->size_custom->setChecked(TRUE); // unless changed by settings below
    config->size_176_220->setChecked(sz == QSize(176,220));
    config->size_240_320->setChecked(sz == QSize(240,320));
    config->size_320_240->setChecked(sz == QSize(320,240));
    config->size_640_480->setChecked(sz == QSize(640,480));
}

void QVFb::skinConfigChosen(int i)
{
    if ( i ) {
	chooseSize(Skin::screenSize(skinfiles[i-1]));
    }
}

void QVFb::setGamma400(int n)
{
    double g = n/100.0;
    view->setGamma(config->rslider->value()/100.0*g,
                   config->gslider->value()/100.0*g,
                   config->bslider->value()/100.0*g);
    updateGammaLabels();
}

void QVFb::setR400(int n)
{
    double g = n/100.0;
    view->setGamma(config->rslider->value()/100.0*g,
                   view->gammaGreen(),
                   view->gammaBlue());
    updateGammaLabels();
}

void QVFb::setG400(int n)
{
    double g = n/100.0;
    view->setGamma(view->gammaRed(),
                   config->gslider->value()/100.0*g,
                   view->gammaBlue());
    updateGammaLabels();
}

void QVFb::setB400(int n)
{
    double g = n/100.0;
    view->setGamma(view->gammaRed(),
                   view->gammaGreen(),
                   config->bslider->value()/100.0*g);
    updateGammaLabels();
}

void QVFb::updateGammaLabels()
{
    config->rlabel->setText(QString::number(view->gammaRed(),'g',2));
    config->glabel->setText(QString::number(view->gammaGreen(),'g',2));
    config->blabel->setText(QString::number(view->gammaBlue(),'g',2));
}

QSize QVFb::sizeHint() const
{
    return QSize(int(view->displayWidth()*view->zoomH()),
	    int(menuBar()->height()+view->displayHeight()*view->zoomV()));
}

// =====================================================================

AnimationSaveWidget::AnimationSaveWidget(QVFbView *v) :
	QVBox((QWidget*)0,0),
	view(v), recording(false), animation(0),
	timerId(-1), progressTimerId(-1),
	recOn(red_on_led_xpm), recOff(red_off_led_xpm),
	imageNum(0)
{
    // Create the animation record UI dialog
    QHBox *hbox = new QHBox( this );
    recBt = new QPushButton( tr("Record"), hbox );
    resetBt = new QPushButton( tr("Reset"), hbox );
    saveBt = new QPushButton( tr("Save"), hbox );
    recBt->setFixedWidth( 100 );
    resetBt->setFixedWidth( 100 );
    saveBt->setFixedWidth( 100 );
    timeDpy = new QLabel( "00:00", hbox );
    recLED = new QLabel( hbox );
    recLED->setPixmap( recOff );
    timeDpy->setMargin( 5 );
    connect( recBt, SIGNAL(clicked()), this, SLOT(toggleRecord()) );
    connect( resetBt, SIGNAL(clicked()), this, SLOT(reset()) );
    connect( saveBt, SIGNAL(clicked()), this, SLOT(save()) );
    elapsed = 0;
    setMargin( 5 );
    setLineWidth( 5 );
    haveMpeg = detectPpmtoMpegCommand();
    mpegSave = new QCheckBox( tr("Save in MPEG format (requires netpbm package installed)"), this );
    mpegSave->setChecked( haveMpeg );
    mpegSave->setEnabled( haveMpeg );
    savingAsMpeg = haveMpeg;
    QHBox *hbox2 = new QHBox( this );
    statusText = new QLabel( tr("Click record to begin recording."), hbox2 );
    progressBar = new QProgressBar( hbox2 );
    progressBar->setProgress( 0 );
    progressBar->hide();
}

AnimationSaveWidget::~AnimationSaveWidget()
{
    // clean up
    removeTemporaryFiles();
    delete animation;
}

// returns true if we have ppmtompeg command, else returns false
bool AnimationSaveWidget::detectPpmtoMpegCommand()
{
    // search the PATH for the ppmtompeg command to test we can record to mpeg
    QStringList paths = QStringList::split(QChar(':'),QString(::getenv("PATH")));
    for ( unsigned int i = 0; i < paths.count(); i++ )
	if ( QFile::exists( paths[i] + "/" + "ppmtompeg" ) )
	    return true;
    return false;
}

void AnimationSaveWidget::timerEvent( QTimerEvent *te )
{
    QString str;

    // Recording timer
    if ( te->timerId() == timerId ) {

	// Add a frame to the animation
	if ( savingAsMpeg && view )
	    view->image().save( str.sprintf("/tmp/qvfb_tmp_image_%04d.ppm", imageNum), "PPM");
	else if ( animation && view )
	    animation->appendFrame(view->image());//QPoint(0,0));
	imageNum++;

	// Update the display of number of seconds that have been recorded.
	int tmMsec = tm.elapsed();
	timeDpy->setText( str.sprintf("%02d:%02d", tmMsec/60000, (tmMsec%60000)/1000) );
	QObject::timerEvent( te );

	// Make the recording LED blink
	static int tick = 0;
	static bool on = false;
	if ( tick > 10 ) {
	    tick = 0;
	    if ( on )
		recLED->setPixmap( recOff );
	    else
		recLED->setPixmap( recOn );
	    on = !on;
	}
	tick++;
    }

    // Saving progress timer
    if ( te->timerId() == progressTimerId ) {
	// Parse output log file to work out the encoding progress.
	QFile f("/tmp/qvfb_tmp_output.log");
	f.open(IO_ReadOnly);
	int largestNum = 0;
	bool done = false;
	while ( !f.atEnd() ) {
	    // example of the output log entries
	    //   During each frame:
	    //      "FRAME 764 (B):  I BLOCKS:  0......
	    //   When complete:
	    //      "======FRAMES READ:  766"
	    f.readLine(str, 1024);
	    if ( str.left(6) == "FRAME " ) {
		int num = str.mid(6, str.find(QChar(' '), 6) - 6).toInt();
		if ( num > largestNum )
		    largestNum = num;
	    } else if ( str.left(18) == "======FRAMES READ:" ) {
		done = true;
	    }
	}
	f.close();

	// Update the progress bar with the frame we are up to
	progressBar->setProgress( largestNum );

	// Finished saving
	if ( done ) {
	    progressBar->hide();
	    statusText->setText( tr("Finished saving."));
	    removeTemporaryFiles();
	    killTimer( progressTimerId );
	    progressTimerId = -1;
	    reset();
	}
    }
}

// Takes the saved ppm files and converts them to a mpeg file named filename
void AnimationSaveWidget::convertToMpeg(QString filename)
{
    recLED->setPixmap( recOff );
    killTimer( timerId );

    progressBar->show();
    progressBar->setTotalSteps( imageNum );
    progressBar->setProgress( 0 );

    // Build parameter file required by ppmtompeg
    QFile file("/tmp/qvfb_tmp_ppmtompeg.params");
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream t( &file );
	t << "PATTERN IBBPBBPBBPBBPBB\n";
	t << "OUTPUT " << filename << "\n";
	t << "INPUT_DIR /tmp\n";
	t << "INPUT\n";
	QString str;
	str = str.sprintf("%04d", imageNum - 1);
	t << "qvfb_tmp_image_*.ppm [0000-" << str << "]\n";
	t << "END_INPUT\n";
	t << "BASE_FILE_FORMAT PPM\n";
	t << "INPUT_CONVERT *\n";
	t << "GOP_SIZE 15\n";
	t << "SLICES_PER_FRAME 1\n";
	t << "PIXEL HALF\n";
	t << "RANGE 5\n";
	t << "PSEARCH_ALG LOGARITHMIC\n";
	t << "BSEARCH_ALG SIMPLE\n";
	t << "IQSCALE 1\n";
	t << "PQSCALE 1\n";
	t << "BQSCALE 1\n";
	t << "REFERENCE_FRAME DECODED\n";
	t << "ASPECT_RATIO 1\n";
	t << "FRAME_RATE 24\n";
	t << "BIT_RATE 64000\n";	    // Quality
	t << "BUFFER_SIZE 2048\n";
    }
    file.close();

    // ### can't use QProcess, not in Qt 2.3
    // Execute the ppmtompeg command as a seperate process to do the encoding
    pid_t pid = ::fork();
    if ( !pid ) {
	// Child process
	// redirect stdout to log file
	freopen("/tmp/qvfb_tmp_output.log", "w", stdout);
	// ppmtompeg tool is from the netpbm package
	::execlp("ppmtompeg", "ppmtompeg", "/tmp/qvfb_tmp_ppmtompeg.params", 0);
	exit(0);
    }

    // Update the saving progress bar every 200ms
    progressTimerId = startTimer( 200 );
}

// Cleanup temporary files created during creating a mpeg file
void AnimationSaveWidget::removeTemporaryFiles()
{
    QString str;
    for ( int i = 0; i < imageNum; i++ )
	QFile::remove( str.sprintf("/tmp/qvfb_tmp_image_%04d.ppm", i) );
    QFile::remove("/tmp/qvfb_tmp_ppmtompeg.params");
    QFile::remove("/tmp/qvfb_tmp_output.log");
    imageNum = 0;
}

// toggles between recording and paused (usually when record button clicked)
void AnimationSaveWidget::toggleRecord()
{
    if ( recording ) {
	recLED->setPixmap( recOff );
	recBt->setText( tr("Record") );
	statusText->setText( tr("Paused. Click record to resume, or save if done."));
	killTimer( timerId );
	timerId = -1;
	elapsed = tm.elapsed();
    } else {
	recLED->setPixmap( recOn );
	recBt->setText( tr("Pause") );
	statusText->setText( tr("Recording..."));
	tm.start();
	if ( elapsed == 0 ) {
	    savingAsMpeg = mpegSave->isChecked();
	    if ( !savingAsMpeg ) {
		delete animation;
		animation = new QAnimationWriter("/tmp/qvfb_tmp_animation.mng","MNG");
		animation->setFrameRate(24);
		if ( view )
		    animation->appendFrame(view->image());
	    }
	}
	tm = tm.addMSecs(-elapsed);
	elapsed = 0;
	timerId = startTimer(1000 / 24);
    }
    recording = !recording;
}

// Reset everything to initial state of not recording
void AnimationSaveWidget::reset()
{
    if ( recording ) {
	toggleRecord();
	statusText->setText( tr("Click record to begin recording."));
	removeTemporaryFiles();
    }
    progressBar->setProgress( 0 );
    timeDpy->setText( "00:00" );
    elapsed = 0;
    imageNum = 0;
    delete animation;
    animation = 0;
}

// Prompt for filename to save to and put animation in that file
void AnimationSaveWidget::save()
{
    if ( recording )
	toggleRecord(); // pauses
    statusText->setText( tr("Saving... "));

    QFileDialog *imagesave = new QFileDialog(".", "*.mpg;*.mng", this, 0, true);
    QString filename;
    if ( savingAsMpeg ) {
	filename = imagesave->getSaveFileName("animation.mpg", "*.mpg", this, "", tr("Save animation..."));
	if ( !!filename )
	    convertToMpeg(filename);
    } else {
	filename = imagesave->getSaveFileName("animation.mng", "*.mng", this, "", tr("Save animation..."));
	if ( !!filename ) {
	    // This might be the simplest posix way to move a file.
	    // For windows, might need to do something else
	    ::link("/tmp/qvfb_tmp_animation.mng", filename.latin1());
	    ::unlink("/tmp/qvfb_tmp_animation.mng");
	    statusText->setText( tr("Finished saving."));
	    reset();
	}
    }
}

#include "qvfb.moc"

