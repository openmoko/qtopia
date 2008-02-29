#include <qapplication.h>
#include <qstringlist.h>
#include "composeimpl.h"
#include "composeim.h"

/* XPM */
static const char * pix_xpm[] = {
"16 13 3 1",
" 	c #FFFFFFFFFFFF",
"#	c #000000000000",
".	c #FFFFFFFFFFFF",
"                ",
"      ####      ",
"    ##....##    ",
"   #........#   ",
"   #........#   ",
"  #...####...#  ",
"  #..#.......#  ",
"  #..#.......#  ",
"  #..#.......#  ",
"   #..####..#   ",
"   #........#   ",
"    ##....##    ",
"      ####      "};


ComposeImpl::ComposeImpl()
    : input(0), icn(0), statWid(0), ref(0)
{
}

ComposeImpl::~ComposeImpl()
{
    delete input;
    delete icn;
    delete statWid;
}



QRESULT ComposeImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_ExtInputMethod )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( ComposeImpl )
}

//For a composing input method, the keyboardWidget() function returns 0:

QWidget *ComposeImpl::keyboardWidget( QWidget *, Qt::WFlags )
{
    return 0;
}


//The function inputMethod() returns the input method:

QWSInputMethod *ComposeImpl::inputMethod( )
{
    if ( !input )
	input = new ComposeIM( );
    return input;
}


//resetState() resets the state of the input method:

void ComposeImpl::resetState()
{
    if ( input )
	input->reset();
}


//icon() returns the icon.

QPixmap *ComposeImpl::icon()
{
    if (!icn)
	icn = new QPixmap( pix_xpm );
    return icn;
}


//name() returns the name:

QString ComposeImpl::name()
{
    return qApp->translate( "InputMethods", "Compose" );
}

/*
For a composing input method, the widget returned by statusWidget()
will be placed in the taskbar when the input method is selected. This
widget is typically used to display status, and can also be used to
let the user interact with the input method.

In this example, the widget doesn't do anything
*/

QWidget *ComposeImpl::statusWidget( QWidget *parent, Qt::WFlags flags )
{
    if (!statWid) {
	//(void) inputMethod(); //create input before we use it
	//statWid  = new IMStatus( input, parent);
	statWid = new QWidget( parent, 0, flags );
	statWid->setMinimumSize( 8, 8 );
	statWid->setBackgroundColor( Qt::green );
    }
    return statWid;
}



/*
The compatible() function can be used to say that this input method is
only compatible with certain other input methods. In this case, there
are no restrictions:
*/
QStringList ComposeImpl::compatible( )
{
    return QStringList();
}


/*
In qcopReceive(), we get notified when there is an event on the
inputmethod channel:
*/
void ComposeImpl::qcopReceive( const QCString &msg, const QByteArray &data )
{
    //process QCop event
}


