#include <qapplication.h>
#include <qpixmap.h>
#include "cardmon.h"
#include "cardmonimpl.h"


CardMonitorImpl::CardMonitorImpl()
    : cardMonitor(0), icn(0), ref(0)
{
}

CardMonitorImpl::~CardMonitorImpl()
{
    delete cardMonitor;
    delete icn;
}

QWidget *CardMonitorImpl::applet( QWidget *parent )
{
    if ( !cardMonitor )
	cardMonitor = new CardMonitor( parent );
    return cardMonitor;
}

int CardMonitorImpl::position() const
{
    return 7;
}

QString CardMonitorImpl::name() const
{
    return qApp->translate( "Taskbar Applet", "Card Monitor" );
}

QPixmap CardMonitorImpl::icon() const
{
    if ( !icn ) {
	CardMonitorImpl *that = (CardMonitorImpl *)this;
	that->icn = new QPixmap();
    }
    return *icn;
}

QRESULT CardMonitorImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_TaskbarApplet )
	*iface = this;
    else if ( uuid == IID_TaskbarNamedApplet )
	*iface = this;

    if ( *iface )
	(*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( CardMonitorImpl )
}
