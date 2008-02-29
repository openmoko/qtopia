/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <linux/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//PD #include <qmessagebox.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "siputil.h"

static char *dissipate_our_fqdn = NULL;

/* max number of network interfaces*/
#define MAX_IF 5

/* Path to the route entry in proc filesystem */
#define PROCROUTE "/proc/net/route"

/* file containing the hostname of the machine */
/* This is the name for slackware and redhat */

#define HOSTFILE "/etc/HOSTNAME"

/* and this is the name for debian */
/* #define HOSTFILE "/etc/HOSTNAME" */

#ifndef SIOCGIFCOUNT
#define SIOCGIFCOUNT 0x8935
#endif

char iface[16];


bool SipUtil::checkFilename( const char *filename )
{
	int fd;
	fd = open( filename, O_WRONLY );
	if ( fd == -1 ) {
		return false;
	}
	// else
	close( fd );
	return true;
}


/* This searches the proc routing entry for the interface the default gateway
 * is on, and returns the name of that interface.
 */
char *getdefaultdev()
{
	FILE *fp = fopen( PROCROUTE, "r");
	char buff[4096], gate_addr[128], net_addr[128];
	char mask_addr[128];
	int irtt, window, mss, hh, arp, num, metric, refcnt, use;
	unsigned int iflags;
	char i;
	if( !fp ) {
		perror("fopen");
		return NULL;
	}
	i=0;

// cruise through the list, and find the gateway interface
	while( fgets(buff, 1023, fp) ) {
		num = sscanf(buff, "%s %s %s %X %d %d %d %s %d %d %d %d %d\n",
			iface, net_addr, gate_addr, &iflags, &refcnt, &use, &metric,
			mask_addr, &mss, &window, &irtt, &hh, &arp);
		i++;
		if( i == 1) continue;
		if( iflags & RTF_GATEWAY )
			return iface;
	}
	fclose(fp);
/* didn't find a default gateway */
	return NULL;
}


void findFqdn( void )
{
	int sock, err, if_count, i, j = 0;
	struct ifconf netconf;
	char buffer[32*MAX_IF];
	char if_name[10][21];
	char if_addr[10][21];
	char *default_ifName;
	netconf.ifc_len = 32 * MAX_IF;
	netconf.ifc_buf = buffer;
	sock=socket( PF_INET, SOCK_DGRAM, 0 );
	err=ioctl( sock, SIOCGIFCONF, &netconf );
	if ( err < 0 ) printf( "Error in ioctl: %i.\n", errno );
	close( sock );
	if_count = netconf.ifc_len / 32;
	printf( "Found %i interfaces.\n", if_count );

//#test
	if ( if_count == 1 ) {
		strncpy( if_name[j], netconf.ifc_req[0].ifr_name, 20 );
		strncpy( if_addr[j], inet_ntoa(((struct sockaddr_in*)(&netconf.ifc_req[0].ifr_addr))->sin_addr), 20 );
		j++;
	} else {
		for ( i = 0; i < if_count; i++ ) {
			if ( strcmp( netconf.ifc_req[i].ifr_name, "lo" ) != 0 
					&& strncmp( netconf.ifc_req[i].ifr_name, "vmnet", 5) != 0) {
				strncpy( if_name[j], netconf.ifc_req[i].ifr_name, 20 );
				strncpy( if_addr[j], inet_ntoa(((struct sockaddr_in*)(&netconf.ifc_req[i].ifr_addr))->sin_addr), 20 );
				j++;
			}
		}
	}
	if( j == 1 ) {
		dissipate_our_fqdn = strdup( if_addr[0] );
	} else {
		default_ifName = getdefaultdev();
		if( default_ifName != NULL) {
			for( i = 0; i < j; i++ ) {
				if( strcmp( if_name[i], default_ifName ) == 0 ) {

                                        // use default (could have user choose)
					dissipate_our_fqdn = strdup( if_addr[i] );
					return;
				}
			}
		} else {
			default_ifName = "";
		}
		for( i = 0; i < j; i++ ) {
			if( strcmp( if_name[i], default_ifName ) != 0 ) {

                                // use first one (could have user choose)
				dissipate_our_fqdn = strdup( if_addr[i] );
                                return;
			}
		}
	}
}

char *SipUtil::getLocalFqdn( void )
{
	if ( dissipate_our_fqdn == NULL ) {
		findFqdn();
	}
	return dissipate_our_fqdn;
}

void SipUtil::setLocalFqdn( const char *s )
{
	dissipate_our_fqdn = (char *) malloc(strlen(s) + 2);
	strcpy(dissipate_our_fqdn, s);
}
