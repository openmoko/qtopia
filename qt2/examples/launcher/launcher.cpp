/****************************************************************************
** $Id: qt/examples/launcher/launcher.cpp   2.3.12   edited 2005-10-27 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qimage.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qvbuttongroup.h>
#include <qwindowsystem_qws.h>

#include <stdlib.h>
#ifndef _OS_WIN32_
#include <unistd.h>
#endif


#ifdef _OS_WIN32_
void unsetenv( const char * ) {}
void setenv( const char *, const char *, int ) {}
#endif

/*
    We should really have more specific #ifdef checks for some of these
    example, this rough partitioning is sufficient though for the sample
    qconfig-*.h files.
*/

struct {
    const char* label;
    const char* file;
} command[] = {
#ifndef QT_NO_TEXTBROWSER
    { "Help Text Browser", "(cd ../helpviewer; exec ../helpviewer/helpviewer)" },
#endif
#ifndef QT_NO_CANVAS
    { "Canvas - alpha-blending", "( cd ../canvas; exec ./canvas )" },
#endif
#ifndef QT_NO_WIDGETS
    { "Text Editor", "exec ../qwerty/qwerty ../qwerty/unicode.txt" },
#endif
#ifndef QT_NO_FILEDIALOG
    { "Scribble Editor", "exec ../scribble/scribble" },
#endif
#ifndef QT_NO_TRANSLATION
    { "Internationalization", "( cd ../i18n; exec ./i18n all )" },
#endif
#ifndef QT_NO_TRANSFORMATIONS
    { "Magnifier", "exec ../qmag/qmag" },
#endif
    { 0, 0 }
};

struct {
    const char* label;
    const char* file;
} other_command[] = {
#ifndef QT_NO_TRANSFORMATIONS
    { "aclock", "( cd ../aclock; exec ./aclock; )" },
#endif
#if !defined(QT_NO_TABDIALOG) && !defined(QT_NO_FILEDIALOG)
    { "addressbook", "( cd ../addressbook; exec ./addressbook; )" },
#endif
#ifndef QT_NO_WIDGETS
    { "buttongroups", "( cd ../buttongroups; exec ./buttongroups; )" },
    { "checklists", "( cd ../checklists; exec ./checklists; )" },
    { "cursor", "( cd ../cursor; exec ./cursor; )" },
    { "customlayout", "( cd ../customlayout; exec ./customlayout; )" },
    { "dclock", "( cd ../dclock; exec ./dclock; )" },
#endif
#ifndef QT_NO_DRAGANDDROP
    { "dirview", "( cd ../dirview; exec ./dirview; )" },
#endif
    //{ "drawdemo", "( cd ../drawdemo; exec ./drawdemo; )" },
    { "drawlines", "( cd ../drawlines; exec ./drawlines; )" },
    //{ "forever", "( cd ../forever; exec ./forever; )" },
    { "hello", "( cd ../hello; exec ./hello; )" },
#ifndef QT_NO_WIDGETS
    { "layout", "( cd ../layout; exec ./layout; )" },
    { "life", "( cd ../life; exec ./life; )" },
    { "lineedits", "( cd ../lineedits; exec ./lineedits; )" },
    { "listbox", "( cd ../listbox; exec ./listbox; )" },
    { "listboxcombo", "( cd ../listboxcombo; exec ./listboxcombo; )" },
#endif
#ifndef QT_NO_WORKSPACE
    { "mdi", "( cd ../mdi; exec ./mdi; )" },
#endif
#ifndef QT_NO_WIDGETS
    { "menu", "( cd ../menu; exec ./menu; )" },
#endif
#ifndef QT_NO_MOVIE
    { "movies", "( cd ../movies; exec ./movies; )" },
#endif
#ifndef QT_NO_WIDGETS
    //{ "picture", "( cd ../picture; exec ./picture; )" },
    { "popup", "( cd ../popup; exec ./popup; )" },
#endif
#ifndef QT_NO_PROGRESSDIALOG
    { "progress", "( cd ../progress; exec ./progress; )" },
#endif
#ifndef QT_NO_WIDGETS
    { "progressbar", "( cd ../progressbar; exec ./progressbar; )" },
    { "qfd", "( cd ../qfd; exec ./qfd; )" },
    { "rangecontrols", "( cd ../rangecontrols; exec ./rangecontrols; )" },
    { "richtext", "( cd ../richtext; exec ./richtext; )" },
    { "scrollview", "( cd ../scrollview; exec ./scrollview; )" },
#endif
#ifndef QT_NO_FILEDIALOG
    { "showimg", "( cd ../showimg; exec ./showimg; )" },
#endif
#ifndef QT_NO_WIDGETS
    //{ "sound", "( cd ../sound; exec ./sounds; )" },
    { "splitter", "( cd ../splitter; exec ./splitter; )" },
    { "tabdialog", "( cd ../tabdialog; exec ./tabdialog; )" },
#endif
#ifndef QT_NO_TRANSFORMATIONS
    { "table", "( cd ../table; exec ./table; )" },
#endif
#ifndef QT_NO_WIDGETS
    { "tetrix", "( cd ../tetrix; exec ./tetrix; )" },
    { "tictac", "( cd ../tictac; exec ./tictac; )" },
    { "tooltip", "( cd ../tooltip; exec ./tooltip; )" },
    { "validator", "( cd ../validator; exec ./validator; )" },
#endif
#if !defined(QT_NO_MOVIE) && !defined(QT_NO_TRANSFORMATIONS) // (etc.!)
    { "widgets", "( cd ../widgets; exec ./widgets; )" },
#endif
#ifndef QT_NO_WIZARD
    { "wizard", "( cd ../wizard; exec ./wizard; )" },
    //{ "xform", "( cd ../xform; exec ./xform; )" },
#endif
    { 0, 0 }
};


class Launcher : public QHBox {
    Q_OBJECT
public:
    Launcher() :
	QHBox(0,0,WStyle_Tool | WStyle_Customize)
    {
	setSpacing(10);
	setMargin(10);
	QMimeSourceFactory::defaultFactory()
	    ->setImage("qtlogo",QImage("qtlogo.png"));
	QMimeSourceFactory::defaultFactory()
	    ->setImage("face",QImage("face.png"));
	QVBox* vb;
        vb = new QVBox(this);
	setStretchFactor(vb,1);
	QLabel* label = new QLabel(
	    "<center><h1><img src=qtlogo><img src=face><br>"
	    "Qt/Embedded</h1>"
	    "</center>"
"This display is a simple Qt/Embedded launcher application, running directly\n"
"on the Linux console. The buttons and listbox\n"
"to the right invoke additional applications.<p><br><hr>"
		, vb);
	info = new QLabel(vb);
	info->setFont(QFont("smoothtimes",17));
	info->setBackgroundColor(white);
	info->setAlignment(AlignTop);
	nextInfo();
	QTimer* infotimer = new QTimer(this);
	connect(infotimer,SIGNAL(timeout()),this,SLOT(nextInfo()));
	infotimer->start(20000);
	setBackgroundColor(white);
	label->setBackgroundColor(white);

	vb = new QVBox(this);
	vb->setBackgroundColor(white);
	QPushButton *pb=0;
	int i;
	for (i=0; command[i].label; i++) {
	    pb = new QPushButton(command[i].label,vb,command[i].file);
	    connect(pb, SIGNAL(clicked()), this, SLOT(execute()));
	}
	QListBox *lb = new QListBox(vb);
	lb->setFont(QFont("smoothtimes",17));
	for (i=0; other_command[i].label; i++) {
	    lb->insertItem(other_command[i].label);
	}
	lb->setMaximumHeight(pb->height()*8);
	connect(lb, SIGNAL(highlighted(int)), this, SLOT(executeOther(int)));
	connect(lb, SIGNAL(selected(int)), this, SLOT(executeOther(int)));

#if !defined(QT_NO_QWS_TRANSFORMED)
	QRadioButton *rb;
	QVButtonGroup *bg = new QVButtonGroup("Orientation", vb);
	connect( bg, SIGNAL(pressed(int)), this, SLOT(setRotation(int)) );
	bg->setBackgroundColor(white);
	bg->setExclusive( TRUE );
	rb = new QRadioButton( "No Rotation", bg );
	rb->setBackgroundColor(white);
	bg->insert( rb );
	rb = new QRadioButton( "Rotate 90deg", bg );
	rb->setBackgroundColor(white);
	bg->insert( rb );
	rb = new QRadioButton( "Rotate 180deg", bg );
	rb->setBackgroundColor(white);
	bg->insert( rb );
	rb = new QRadioButton( "Rotate 270deg", bg );
	rb->setBackgroundColor(white);
	bg->insert( rb );
	bg->setButton(0);
#endif
	
	QHBox* hb = new QHBox(vb);
	hb->setBackgroundColor(white);
	pb = new QPushButton("Restart",hb);
	connect(pb, SIGNAL(clicked()), this, SLOT(quit3()));
	pb = new QPushButton("Quit",hb);
	connect(pb, SIGNAL(clicked()), qApp, SLOT(quit()));
	hb->setSpacing(10);
	hb->setFixedHeight(hb->sizeHint().height());
    }

private slots:
    void nextInfo()
    {
	static int i = 0;
	static const char* infotext[] = {
	"<h2>What is it?</h2>\n"
	"\n"
	"The Qt/Embedded product provides you with all you need to\n"
	"create stunning graphical user interfaces for embedded devices.\n"
	"Qt/Embedded installs and runs with a very small memory footprint on any\n"
	"device running embedded Linux - without using X11.\n",
	"\n"
	"<h2>Trust the promises</h2>\n"
	"\n"
	"Qt/Embedded features the same API as the excellent\n"
	"Qt/Windows and Qt/X11 versions. Imagine writing your Qt application in\n"
	"your favourite desktop environment and just recompiling to move it to\n"
	"your embedded device. It saves you heaps of development effort and\n"
	"allows you to work productively in your favorite programming\n"
	"environment from day one. You can start today.\n",
	"\n"
	"<h2>Less is more</h2>\n"
	"\n"
	"Qt/Embedded is modular and scalable. You can assemble the\n"
	"Qt features you really need and leave the others out. Since Qt/Embedded\n"
	"is not based on X11 it has substantially lower memory requirements than\n"
	"X11. By picking and choosing features, the memory demands of\n"
	"Qt/Embedded can be tuned from 1 Mb to 3 Mb in ROM (Intel x86).\n"
	"Furthermore,applications written with Qt are known to have a small\n"
	"memory footprint compared to applications written with other toolkits.\n"
	"Qt scales from the smallest embedded device to high-end workstations.\n",
	"\n"
	"<h2>The source will be with you</h2>\n"
	"\n"
	"You know the value of source code availability. All Qt\n"
	"releases are delivered with source code. Qt/Embedded is no exception.\n"
	"You get a better understanding of how Qt works and it helps you debug\n"
	"and tune your code. Customer feedback allows us to continually improve\n"
	"the source code for better usability and performance.\n",
	"\n"
	"<h2>Add your touch</h2>\n"
	"\n"
	"Qt's clean object oriented design makes it easy to extend and\n"
	"enhance the standard widgets. You can create specialized widgets for the\n"
	"limited space offered by embedded screen devices. You can benefit from the\n"
	"wealth of available third-party Qt software. It is easy to add the killer\n"
	"feature you need.\n",
	"\n"
	"<h2>The beauty of it</h2>\n"
	"\n"
	"Qt/Embedded features some great additional functionality compared\n"
	"to X11. You will be stunned by the beauty of anti-aliased text and alpha\n"
	"blended pixmaps. These new features could add an additional touch of class to a\n"
	"user interface. Forget about embedded graphics which looks more like old\n"
	"alphanumeric terminals. Qt/Embedded can utilize hardware graphics acceleration\n"
	"and it is well suited for multimedia and web applications. Let Qt/Embedded\n"
	"impress you and your customers.\n",
	"\n"
	"<h2>Getting there faster</h2>\n"
	"\n"
	"Qt is one of the most popular GUI toolkits in the world.\n"
	"Programmers like the compact code,the powerful API,the ease of use, and the\n"
	"excellent support. It is easy to find existing developer skills with Qt and a\n"
	"lot of quality Qt based code has already been written. Qt/Embedded allows you\n"
	"and your programmers to move seamlessly into the exciting field of embedded\n"
	"systems. Your Qt experts don't need to be retrained. Our customers tell us that\n"
	"even programmers without Qt experience get up to speed sooner with Qt than with\n"
	"other toolkits.\n",
	"\n"
	"<h2>We support you</h2>\n"
	"\n"
	"Embedded devices have other requirements than conventional\n"
	"computers. The Qt/Embedded team at Trolltech is dedicated to support you with\n"
	"new features and widgets. We can offer training, partnerships and first class\n"
	"support. It is straightforward to port Qt/Embedded to new hardware. We can\n"
	"offer you a port to your special device or the embedded operating system of\n"
	"your choice. We look forward to working closely with you to help you succeed.\n",

	0
	};
	QString t=infotext[i];
	info->setText(
	    "<blockquote>"
	    +t+
	    "</blockquote>"
	);
	i++;
	if (!infotext[i]) i=0;
    }

    void quit3()
    {
	qApp->exit(3);
    }

    void run(const char* cmd)
    {
	QString c = cmd;
	// c += " 2>/dev/null >/dev/null";
	c += " &";
	system(c.latin1());
    }
    void execute()
    {
	run(sender()->name());
    }
    void executeOther(int i)
    {
	run(other_command[i].file);
    }

    void setAntiAliasedText(bool on)
    {
	if ( on ) {
	    unsetenv("QWS_NO_SMOOTH_FONTS");
	} else {
	    setenv("QWS_NO_SMOOTH_FONTS","1",1);
	}
    }
    void setAcceleration(bool on)
    {
	if ( on ) {
	    unsetenv("QWS_NOACCEL");
	} else {
	    setenv("QWS_NOACCEL","1",1);
	}
    }
    void setRotation( int r )
    {
#if !defined(QT_NO_QWS_TRANSFORMED)
	switch ( r ) {
	    case 0:
		unsetenv( "QWS_DISPLAY" );
		break;
	    case 1:
		setenv( "QWS_DISPLAY", "Transformed:Rot90:0", 1 );
		break;
	    case 2:
		setenv( "QWS_DISPLAY", "Transformed:Rot180:0", 1 );
		break;
	    case 3:
		setenv( "QWS_DISPLAY", "Transformed:Rot270:0", 1 );
		break;
	}
#endif
    }
private:
    QLabel* info;
};

#include "launcher.moc"



#ifdef _WS_QWS_

#include <qmap.h>
#include <qfile.h>
#include <qtextstream.h>

class SimpleIM : public QWSServer::KeyboardFilter
{
public:
    SimpleIM( const QString &fn );
    virtual  ~SimpleIM();
    bool filter(int unicode, int keycode, int modifiers, bool isPress,
		      bool autoRepeat);

private:
    int backspaces( const QString &s );
    bool active;
    unsigned int maxlen;
    QMap<QString,QString> map;
    QString soFar;
};




SimpleIM::SimpleIM( const QString &fn )
{
    active = FALSE;
    maxlen = 0;
    QFile f( fn );
    if ( f.open(IO_ReadOnly) ) {
	
	QTextStream t( &f );        // use a text stream
	QString s;
	while ( !t.eof() ) {        // until end of file...
	    s = t.readLine();       // line of text excluding '\n'
	    int p = s.find( '\t' );
	    if ( p >= -1 ) {
		QString s1 = s.left( p );
		QString s2 = s.mid( p+1 );
		map[s1] = s2;
		maxlen = QMAX( maxlen, s1.length() );
	    }
	}
	f.close();
    }
    qDebug( "maxlen = %d", maxlen );
}

SimpleIM::~SimpleIM()
{
}


static void sendKey( int unicode, int keycode )
{
    QWSServer::sendKeyEvent( unicode, keycode, 0, TRUE, FALSE );
    QWSServer::sendKeyEvent( unicode, keycode, 0, FALSE, FALSE );
}

static void sendString( const QString &s )
{
    for ( int i = 0; i < int(s.length()); i++ )
	sendKey( s[i].unicode(), 0 );
}


/*
  How many backspaces do I need to send in order to remove \a str?
*/
int SimpleIM::backspaces( const QString &str )
{
    int r = 0;
    QString s = str;
    while ( !s.isEmpty() ) {
	qDebug( "testing %s", s.latin1() );
	int n = s.length();
	while ( n > 0 ) {
	    QString sub = s.right( n );
	    if ( map.contains( sub ) ) {
		r += map[sub].length();
		break;
	    }
	    n--;
	}
	if ( n == 0 ) {
	    n = 1;
	    r++;
	}
	s = s.left(s.length()-n);
    }
    
    return r;
}
/*
  Watch all keypresses. Toggle active mode when shift+space is seen.

  In active mode: Put each key pressed into the buffer soFar. When the
  contents of soFar match an entry in map, send backspace keypresses
  to remove the source characters and then send the mapped string.
  
  This is just a simple example. Production code would have to handle
  key releases as well.
 */
bool SimpleIM::filter(int unicode, int keycode, int modifiers, bool isPress,
		      bool /*autoRepeat*/ )
{
    if ( modifiers == Qt::ShiftButton && keycode == Qt::Key_Space ) {
	if ( !isPress ) {
	    active = !active;
	}
	return TRUE; //filter
    }
    if ( active  && !(modifiers&~Qt::ShiftButton) ) {
	if ( isPress && keycode != Qt::Key_Shift ) {
	    soFar = soFar + QChar( unicode );
	    soFar = soFar.right( maxlen );
	
	    int n = soFar.length();
	    while ( n > 0 ) {
		QString candidate = soFar.right( n );
		if ( map.contains( candidate ) ) {
		    int bc = backspaces( candidate.left(n-1) );
		    for ( int i = 0; i < bc; i++ )
			sendKey( 0, Qt::Key_Backspace);
		    sendString( map[candidate] );
		    soFar=soFar.right(n);
		    return TRUE; //filter
		}
		n--;
	    }
	}
	return FALSE; //don't filter
    }
    soFar = "";
    return FALSE; //don't filter
}

#endif

void silent(QtMsgType, const char *)
{
}


int main(int argc, char** argv)
{
    QApplication app(argc,argv, QApplication::GuiServer);

    qInstallMsgHandler(silent);

    app.setFont(QFont("smoothtimes",22));
#ifdef _WS_QWS_
    if ( QString(argv[1]) == "-im" && argv[2] ) {
	SimpleIM* im = new SimpleIM( argv[2] );
	QWSServer::setKeyboardFilter( im );
    }
/*
    // Custom 40 grey palette
    QRgb cols[40];
    for ( int i = 0; i < 40; i++ ) {
	int c = i*256/40;
	cols[i] = qRgb( c, c, c );
    }
    app.qwsSetCustomColors( cols, 0, 40 );
*/
#endif

    Launcher l;
    app.setMainWidget(&l);
    l.showMaximized();
    return app.exec();
}
