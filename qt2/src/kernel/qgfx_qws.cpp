/****************************************************************************
** $Id: qt/src/kernel/qgfx_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QGfx (graphics context) class
**
** Created : 990721
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qgfx_qws.h"

#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#ifndef QT_NO_QWS_CURSOR
bool qt_sw_cursor=false;
QScreenCursor * qt_screencursor=0;
#endif
QScreen * qt_screen=0;

extern bool qws_screen_is_interlaced; //### hack, from qapplication_qws.cpp

ClearCacheFunc QScreen::clearCacheFunc = 0;

QGfx *QGfx::createGfx( int depth, unsigned char *buffer, int w, int h,
			     int offs )
{
    return qt_screen->createGfx( buffer, w, h, depth, offs );
}

bool QScreen::isTransformed() const
{
    return FALSE;
}

bool QScreen::isInterlaced() const
{
    return qws_screen_is_interlaced;;
}

QSize QScreen::mapToDevice( const QSize &s ) const
{
    return s;
}

QSize QScreen::mapFromDevice( const QSize &s ) const
{
    return s;
}

QPoint QScreen::mapToDevice( const QPoint &p, const QSize & ) const
{
    return p;
}

QPoint QScreen::mapFromDevice( const QPoint &p, const QSize & ) const
{
    return p;
}

QRect QScreen::mapToDevice( const QRect &r, const QSize & ) const
{
    return r;
}

QRect QScreen::mapFromDevice( const QRect &r, const QSize & ) const
{
    return r;
}

QImage QScreen::mapToDevice( const QImage &i ) const
{
    return i;
}

QImage QScreen::mapFromDevice( const QImage &i ) const
{
    return i;
}

QRegion QScreen::mapToDevice( const QRegion &r, const QSize & ) const
{
    return r;
}

QRegion QScreen::mapFromDevice( const QRegion &r, const QSize & ) const
{
    return r;
}

int QScreen::transformOrientation() const
{
    return 0;
}

int QScreen::pixmapDepth() const
{
    return depth();
}

#ifdef QT_LOADABLE_MODULES

// ### needs update after driver init changes

static QScreen * qt_dodriver(char * driver,char * a,unsigned char * b)

{
    char buf[200];
    strcpy(buf,"/etc/qws/drivers/");
    qstrcpy(buf+17,driver);
    qDebug("Attempting driver %s",driver);

    void * handle;
    handle=dlopen(buf,RTLD_LAZY);
    if(handle==0) {
	qFatal("Module load error");
    }
    QScreen *(*qt_get_screen_func)(char *,unsigned char *);
    qt_get_screen_func=dlsym(handle,"qt_get_screen");
    if(qt_get_screen_func==0) {
	qFatal("Couldn't get symbol");
    }
    QScreen * ret=qt_get_screen_func(a,b);
    return ret;
}

static QScreen * qt_do_entry(char * entry)
{
    unsigned char config[256];

    FILE * f=fopen(entry,"r");
    if(!f) {
	return 0;
    }

    int r=fread(config,256,1,f);
    if(r<1)
	return 0;

    fclose(f);

    unsigned short vendorid=*((unsigned short int *)config);
    unsigned short deviceid=*(((unsigned short int *)config)+1);
    if(config[0xb]!=3)
	return 0;

    if(vendorid==0x1002) {
	if(deviceid==0x4c4d) {
	    qDebug("Compaq Armada/IBM Thinkpad's Mach64 card");
	    return qt_dodriver("mach64.so",entry,config);
	} else if(deviceid==0x4742) {
	    qDebug("Desktop Rage Pro Mach64 card");
	    return qt_dodriver("mach64.so",entry,config);
	} else {
	    qDebug("Unrecognised ATI card id %x",deviceid);
	    return 0;
	}
    } else {
	qDebug("Unrecognised vendor");
    }
    return 0;
}

extern bool qws_accel;

/// ** NOT SUPPPORTED **

QScreen * qt_probe_bus()
{
    if(!qws_accel) {
	return qt_dodriver("unaccel.so",0,0);
    }

    DIR * dirptr=opendir("/proc/bus/pci");
    if(!dirptr)
	return qt_dodriver("unaccel.so",0,0);
    DIR * dirptr2;
    dirent * cards;

    dirent * busses=readdir(dirptr);

    while(busses) {
	if(busses->d_name[0]!='.') {
	    char buf[100];
	    strcpy(buf,"/proc/bus/pci/");
	    qstrcpy(buf+14,busses->d_name);
	    int p=strlen(buf);
	    dirptr2=opendir(buf);
	    if(dirptr2) {
		cards=readdir(dirptr2);
		while(cards) {
		    if(cards->d_name[0]!='.') {
			buf[p]='/';
			qstrcpy(buf+p+1,cards->d_name);
			QScreen * ret=qt_do_entry(buf);
			if(ret)
			    return ret;
		    }
		    cards=readdir(dirptr2);
		}
		closedir(dirptr2);
	    }
	}
	busses=readdir(dirptr);
    }
    closedir(dirptr);

    return qt_dodriver("unaccel.so",0,0);
}

#else

const char * qt_qws_hardcoded_slot="/proc/bus/pci/01/00.0";

const unsigned char* qt_probe_bus()
{
    const char * slot;
    slot=getenv("QWS_CARD_SLOT");
    if(!slot)
	slot=qt_qws_hardcoded_slot;
    if ( slot ) {
	static unsigned char config[256];
	FILE * f=fopen(slot,"r");
	if(!f) {
	    slot=0;
	} else {
	    int r=fread((char*)config,256,1,f);
	    fclose(f);
	    if(r<1)
		return 0;
	    else
		return config;
	}
    }
    return 0;
}

#endif
