#include "sipclient.h"
#include "sipuser.h"

SipUser::SipUser( SipClient *parent, QString fullname, QString username, QString athostname )
{
	client = parent;
	myuri.setFullname( fullname );
	myuri.setUsername( username );
	myuri.setHostname( athostname );
	client->setUser( this );
}

SipUser::SipUser( SipClient *parent, const SipUri &inituri )
{
	client = parent;
	myuri = inituri;
	client->setUser( this );
}

SipUser::~SipUser( void )
{
}

void SipUser::addServer( SipRegister *server )
{
	servers.append( server );
}

void SipUser::removeServer( SipRegister *server )
{
	servers.removeAll( server );
}

void SipUser::setUri( const SipUri &newuri )
{
	myuri = newuri;
}
