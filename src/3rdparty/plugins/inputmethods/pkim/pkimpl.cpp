#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <QStringList>
#include <QLabel>
#include "pkimpl.h"
#include "pkim.h"
#include <QDebug>

/* XPM */
static const char * pix_xpm[] = {
"16 13 3 1",
" 	c None None",
"#	c #000000000000",
".	c #FFFFFFFFFFFF",
"                ",
"      ####      ",
"    ##....##    ",
"   #........#   ",
"   #.#####..#   ",
"  #..##..##..#  ",
"  #..##..##..#  ",
"  #..#####...#  ",
"  #..##......#  ",
"   #.##.....#   ",
"   #........#   ",
"    ##....##    ",
"      ####      "};


PkImpl::PkImpl(QObject *parent)
    : QtopiaInputMethod(parent), input(0), statWid(0), ref(0)
{
    qLog(Input) << "PkImpl::PkImpl(QObject *"<<parent<<")";
}

PkImpl::~PkImpl()
{
    delete input;
    delete statWid;
}


/*
QRESULT PkImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_CoopInputMethod )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}
*/

/*
Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( PkImpl )
}
*/

//The function Modifier input() returns the input method:

QWSInputMethod *PkImpl::inputModifier( )
{
    if ( !input ) {
        input = new PkIM( );
        qLog(Input) << "PkImpl::inputModifier";
//    connect (input, SIGNAL(modeChanged()), this, SIGNAL(stateChanged()));
    }
    return input;
}


//resetState() resets the state of the input method:

void PkImpl::reset()
{
    if ( input )
	input->reset();
}

QtopiaInputMethod::State PkImpl::state() const
{
    if (input && input->isActive()) 
	return QtopiaInputMethod::Ready;
    else
	return QtopiaInputMethod::Sleeping;
}

//icon() returns the icon.

QIcon PkImpl::icon() const
{
    if (icn.isNull())
	icn = QIcon(QPixmap((const char **)pix_xpm));
    return icn;
}


//name() returns the name:
// TODO: determine a more appropriate name for new combined input methods
QString PkImpl::name() const
{
    return qApp->translate( "InputMethods", "Phone Keys" );
}

// TODO: determine a more appropriate identifier for new combined input methods
QString PkImpl::identifier() const
{
    return "http://trolltech.com/Qtopia/PhoneKeyInputMethod";
}

QString PkImpl::version() const
{
    return "4.1.0";
}

/*
For a composing input method, the widget returned by statusWidget()
will be placed in the taskbar when the input method is selected. This
widget is typically used to display status, and can also be used to
let the user interact with the input method.
*/

QWidget *PkImpl::statusWidget( QWidget *parent )
{
    if (!statWid) {
	statWid = new QLabel( parent );
	statWid->setAlignment(Qt::AlignCenter);
	(void)inputModifier();
	input->setStatusWidget(statWid);
    }
    return statWid;
}


// TODO: find out what this properties should more properly return
int PkImpl::properties() const
{
    return ( InputModifier);
}

void PkImpl::setHint(const QString &hint, bool r)
{
    if (input) {
	input->setHint(hint);
	input->setRestrictToHint(r);
    }
}

bool PkImpl::restrictedToHint() const
{
    if ( !input )
	return false;
    return input->restrictToHint();
}

QTOPIA_EXPORT_PLUGIN(PkImpl);
