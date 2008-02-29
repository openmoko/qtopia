/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qnamespace.h>
#include <qtopia/qpeglobal.h>
#if !defined(Q_QDOC) && (QT_VERSION-0 >= 0x030000)
#include <private/qcom_p.h>
#else

#ifndef QCOM_H
#define QCOM_H
#include <qstringlist.h>

#ifndef QT_NO_COMPONENT

#include <qtopia/quuid.h>

#define QRESULT         unsigned long
#define QS_OK           (QRESULT)0x00000000
#define QS_FALSE        (QRESULT)0x00000001

#define QE_NOTIMPL      (QRESULT)0x80000001
#define QE_OUTOFMEMORY  (QRESULT)0x80000002
#define QE_INVALIDARG   (QRESULT)0x80000003
#define QE_NOINTERFACE  (QRESULT)0x80000004
#define QE_NOCOMPONENT  (QRESULT)0x80000005

// {1D8518CD-E8F5-4366-99E8-879FD7E482DE}
#ifndef IID_QUnknown
#define IID_QUnknown QUuid(0x1d8518cd, 0xe8f5, 0x4366, 0x99, 0xe8, 0x87, 0x9f, 0xd7, 0xe4, 0x82, 0xde)
#endif

#if (defined(Q_OS_WIN32) || defined(Q_OS_WIN64) )

#  if defined(QTOPIA_PLUGIN_MAKEDLL)   /* create a Qt DLL library for qtopia */
#    if defined(QTOPIA_PLUGIN_DLL)
#      undef QTOPIA_PLUGIN_DLL
#    endif
#    define QTOPIA_PLUGIN_EXPORT  __declspec(dllexport)
#    define QTOPIA_PLUGIN_TEMPLATE_EXTERN
#    define QTOPIA_PLUGIN_TEMPLATEDLL
#    undef  QTOPIA_PLUGIN_DISABLE_COPY /* avoid unresolved externals */
#  elif defined(QTOPIA_PLUGIN_DLL) || defined (QT_DLL)         /* use a Qt DLL library */
#    define QTOPIA_PLUGIN_EXPORT  __declspec(dllimport)
#    define QTOPIA_PLUGIN_TEMPLATE_EXTERN /*extern*/
#    define QTOPIA_PLUGIN_TEMPLATEDLL
#    undef  QTOPIA_PLUGIN_DISABLE_COPY /* avoid unresolved externals */
#  else
#    define QTOPIA_PLUGIN_EXPORT	
#  endif
#else
#    define QTOPIA_PLUGIN_EXPORT
#endif

struct QTOPIA_PLUGIN_EXPORT QUnknownInterface
{
    virtual QRESULT queryInterface( const QUuid&, QUnknownInterface** ) = 0;
    virtual ulong   addRef() = 0;
    virtual ulong   release() = 0;
};

// {D16111D4-E1E7-4C47-8599-24483DAE2E07}
#ifndef IID_QLibrary
#define IID_QLibrary QUuid( 0xd16111d4, 0xe1e7, 0x4c47, 0x85, 0x99, 0x24, 0x48, 0x3d, 0xae, 0x2e, 0x07)
#endif
 
struct QTOPIA_PLUGIN_EXPORT QLibraryInterface : public QUnknownInterface
{
    virtual bool    init() = 0;
    virtual void    cleanup() = 0;
    virtual bool    canUnload() const = 0;
};

#ifndef SINGLE_EXEC

#ifndef Q_OS_WIN32 
#define Q_EXPORT_INTERFACE() \
	extern "C" QUnknownInterface* ucm_instantiate()
#else
#define Q_EXPORT_INTERFACE() \
	extern "C" QTOPIA_PLUGIN_EXPORT QUnknownInterface* ucm_instantiate()
#endif

#define Q_CREATE_INSTANCE( IMPLEMENTATION )         \
	IMPLEMENTATION *i = new IMPLEMENTATION; \
	QUnknownInterface* iface = 0;                   \
	i->queryInterface( IID_QUnknown, &iface );      \
	return iface;

#else // SINGLE_EXEC

#include <qdict.h>

#define Q_CREATE_INSTANCE(T) \
	} \
	typedef QDict<QUnknownInterface> PluginNameDict; \
	typedef QDict<PluginNameDict> PluginTypeDict; \
	extern PluginTypeDict *ptd; \
	class T##t {\
	public: \
	    T##t() {\
		if (!ptd)\
		    ptd = new PluginTypeDict();\
		PluginNameDict *quil = ptd->find(QTOPIA_PLUGIN_TYPE); \
		if (!quil) { \
		    quil = new PluginNameDict();\
		    ptd->insert(QTOPIA_PLUGIN_TYPE,quil);\
		}\
		T *t = new T;\
		QUnknownInterface *iface = 0;\
		t->queryInterface( IID_QUnknown, &iface);\
		if (iface)\
		    quil->insert(QTOPIA_PLUGIN_NAME,iface);\
	    }; \
	};\
	static T##t T##tvar;\
	static void dummy2() {

#define Q_EXPORT_INTERFACE \
	static void dummy

#endif


#define Q_REFCOUNT  ulong addRef() {return ref++;}ulong release() {if(!--ref){delete this;return 0;}return ref;}

#else // QT_NO_COMPONENT

struct Q_EXPORT QUnknownInterface
{
};

#endif // QT_NO_COMPONENT

#endif // QCOM_H

#endif // qt >= 3.0
