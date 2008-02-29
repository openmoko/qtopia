/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 * Copyright (c) 2004 Wirlab <kphone@wirlab.net>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */

#ifndef SIPVIA_H_INCLUDED
#define SIPVIA_H_INCLUDED

#include <qstring.h>
#include "parameterlist.h"

/**
 * This class represents information for a single SIP Via header.
 */
class SipVia
{
public:
	/**
	 * Creates a blank @ref SipVia object.
	 */
	SipVia( void );

	/**
	 * Parses the input as a SIP Via.
	 */
	SipVia( const QString &parseinput );

	/**
	 * SipVia destructor.
	 */
	~SipVia( void );

	enum Transport {
		UDP,
		TCP,
		TLS,
		BadTransport };

	/**
	 * Static method to convert a transport parameter into a string.
	 */
	static const QString getTransportString( Transport t );

	/**
	 * Static method to convert a transport parameter into the
	 * SipVia::Transport enum.
	 */
	static Transport matchTransport( const QString t );

	enum ViaParam {
		Hidden,
		Ttl,
		Maddr,
		Received,
		Branch,
		BadViaParam,
		Rport
	};

	/**
	 * Static method to convert a Via parameter into a string.
	 */
	static const QString getViaParamString( ViaParam v );

	/**
	 * Static method to convert a Via parameter into a SipVia::ViaParam
	 * enum.
	 */
	static ViaParam matchViaParam( const QString v );

	/**
	 * Returns true if the parsed Via is valid.
	 */
	bool isValid( void ) const { return isvalid; }

	/**
	 * Sets the hostname to place in the Via.
	 */
	void setHostname( const QString& hname );

	/**
	 * Returns the hostname in this via.
	 */
	QString getHostname( void ) const { return hostname; }

	/**
	 * Sets the transport type for this Via, TCP or UDP.
	 */
	void setTransport( Transport t );

	/**
	 * Generates and returns the Via header data.
	 */
	QString via( void );

	/**
	 * Returns the protocol name listed in the via.
	 */
	QString getProtocolName( void ) const { return protocolname; }

	/**
	 * Returns the protocol version listed in the via.
	 */
	QString getProtocolVer( void ) const { return protocolver; }

	/**
	 * Returns the transport for this Via, TCP or UDP.
	 */
	Transport getTransport( void ) const { return trans; }

	/**
	 * Sets the port number listed in this via.
	 */
	void setPortNumber( unsigned int p );

	/**
	 * Returns the port number listed for this via, using 5060 as the
	 * default.
	 */
	unsigned int getPortNumber( void ) const { return port; }

	/**
	 * Returns true if the via has a received tag.
	 */
	bool hasReceivedParam( void ) const { return hasreceived; }

	/**
	 * Returns the address in the received tag of the Via, if present.
	 */
	const QString &getReceivedParam( void ) const { return received; }

	/**
	 * Sets the address in the received tag of the Via.  If QString::null
	 * is given, then there is no received.  This is the default.
	 */
	void setReceivedParam( const QString &newreceived );

	/**
	 * Returns true if the via has an rport tag.
	 */
	bool hasRportParam( void ) const { return hasrport; }

        /**
	 * Returns the port in the rport tag of the Via, if present.
	 */
	const QString &getRportParam( void ) const { return rport; }

	/**
	 * Sets the port in the rport tag of the Via. If QString::null
	 * is given, then there is no rport. This is the default.
	 */
	void setRportParam( const QString &newrport );

	/**
	 * Returns true if the via is hidden.
	 */
	bool isHidden( void ) const { return ishidden; }

	/**
	 * Sets the 'hidden' flag for this via.  By default, Via headers are
	 * not hidden.
	 */
	void setHidden( bool hidden );

	/**
	 * Returns true if this via has a ttl parameter.
	 */
	bool hasTtlParam( void ) const { return hasttl; }

	/**
	 * Returns the TTL for this Via.
	 */
	const QString &getTtlParam( void ) const { return ttl; }

	/**
	 * Sets the ttl parameter for this via.  If QString::null is given,
	 * there is no ttl.  This is the default.
	 */
	void setTtlParam( const QString &newttl );

	/**
	 * Returns true if the via has a maddr parameter.
	 */
	bool hasMaddrParam( void ) const { return hasmaddr; }

	/**
	 * Returns the maddr for this Via.
	 */
	const QString &getMaddrParam( void ) const { return maddr; }

	/**
	 * Sets the maddr parameter for this via.  If QString::null is given,
	 * there is no maddr.  This is the default.
	 */
	void setMaddrParam( const QString &newmaddr );

	/**
	 * Returns true if the via has a branch parameter.
	 */
	bool hasBranchParam( void ) const { return hasbranch; }

	/**
	 * Returns the branch for this Via.
	 */
	const QString &getBranchParam( void ) const { return branch; }

	/**
	 * Sets the branch parameter for this via.  If QString::null is given,
	 * there is no branch.  This is the default.
	 */
	void setBranchParam( const QString &newbranch );

	/**
	 * Generate a random branch parameter.
	 */
	void generateBranchParam( void );

	SipVia &operator=( const SipVia &v );

	bool operator==( const SipVia &v ) const;
	bool operator!=( const SipVia &v ) const;
	bool operator==( const QString &v ) const;
	bool operator!=( const QString &v ) const;

private:
	bool isvalid;

	QString hostname;

	Transport trans;

	QString protocolname;
	QString protocolver;

	bool hasreceived;
	QString received;

	bool hasrport;
	QString rport;

	bool ishidden;

	bool hasttl;
	QString ttl;

	bool hasmaddr;
	QString maddr;

	bool hasbranch;
	QString branch;

	unsigned int port;
	bool hasport; 

	void clear( void );
	void parseVia( const QString& parseinput );

	//rest of via params
	ParameterList restOfParams;

};

#endif // SIPVIA_H_INCLUDED
