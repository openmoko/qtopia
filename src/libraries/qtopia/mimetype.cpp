/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QTOPIA_INTERNAL_MIMEEXT
#include "mimetype.h"
#include "applnk.h"
#include "resource.h"
#include "qpeapplication.h"
#include "config.h"

#include <qfile.h>
#include <qdict.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qmap.h>


static void cleanupMime()
{
    MimeType::clear();
}

class MimeTypeData {
public:
    MimeTypeData(const QString& i) :
	id(i)
    {
	apps.setAutoDelete(TRUE);
    }
    QString id;
    QString extension;
    QList<AppLnk> apps;

    QString description()
    {
	if ( desc.isEmpty() )
	    desc = QPEApplication::tr("%1 document").arg(apps.first()->name());
	return desc;
    }

    QPixmap regIcon()
    {
	if ( regicon.isNull() )
	    loadPixmaps();
	return regicon;
    }

    QPixmap bigIcon()
    {
	if ( bigicon.isNull() )
	    loadPixmaps();
	return bigicon;
    }

private:
    void loadPixmaps()
    {
	if ( apps.count() ) {
	    QString icon;
	    for (AppLnk* lnk = apps.first(); icon.isNull() && lnk; lnk=apps.next()) {
		QStringList icons = lnk->mimeTypeIcons();
		if ( icons.count() ) {
		    QStringList types = lnk->mimeTypes();
		    for (QStringList::ConstIterator t=types.begin(),i=icons.begin(); t!=types.end() && i!=icons.end(); ++i,++t) {
			if ( *t == id ) {
			    icon = *i;
			    break;
			}
		    }
		}
	    }
	    if ( icon.isNull() ) {
		AppLnk* lnk = apps.first();
		regicon = lnk->pixmap();
		bigicon = lnk->bigPixmap();
	    } else {
		QImage unscaledIcon = Resource::loadImage( icon );
		regicon.convertFromImage( unscaledIcon.smoothScale( AppLnk::smallIconSize(), AppLnk::smallIconSize() ) );
		bigicon.convertFromImage( unscaledIcon.smoothScale( AppLnk::bigIconSize(), AppLnk::bigIconSize() ) );
	    }
	}
    }

    QPixmap regicon;
    QPixmap bigicon;
    QString desc;
};

class MimeType::Private : public QDict<MimeTypeData> {
public:
    Private() {}
    ~Private() {}

    // ...
};

MimeType::Private* MimeType::d=0;
static QMap<QString,QString> *typeFor = 0;
static QMap<QString,QStringList> *extFor = 0;

MimeType::Private& MimeType::data()
{
    if ( !d ) {
	d = new Private;
	d->setAutoDelete(TRUE);
	static bool setCleanup = FALSE;
	if ( !setCleanup ) {
	    qAddPostRoutine( cleanupMime );
	    setCleanup = TRUE;
	}
    }
    return *d;
}

/*!
    \class MimeType mimetype.h
    \brief The MimeType class provides MIME type information.

    A MimeType object is a light-weight value which
    provides information about a MIME type.

    \ingroup qtopiaemb
*/

/*!
    Constructs a MimeType.
    Normally, \a ext_or_id is a MIME type,
    but if \a ext_or_id starts with / or contains no /,
    it is interpretted as a filename and the
    extension (eg. .txt) is used as the
    MIME type.
*/
MimeType::MimeType( const QString& ext_or_id )
{
    init(ext_or_id);
}

/*!
    Constructs a MimeType from the type() of \a lnk.
*/
MimeType::MimeType( const DocLnk& lnk )
{
    init(lnk.type());
}

/*!
    Returns the MIME type identifier.
*/
QString MimeType::id() const
{
    return i;
}

/*!
    Returns a description of the MIME Type. This is usually based 
    on the application() associated with the type.
*/
QString MimeType::description() const
{
    MimeTypeData* d = data(i);
    return d ? d->description() : QString::null;
}

/*!
    Returns a small QPixmap appropriate for the MIME type.
*/
QPixmap MimeType::pixmap() const
{
    MimeTypeData* d = data(i);
    return d ? d->regIcon() : QPixmap();
}

/*!
    Returns a larger QPixmap appropriate for the MIME type.
*/
QPixmap MimeType::bigPixmap() const
{
    MimeTypeData* d = data(i);
    return d ? d->bigIcon() : QPixmap();
}


/*!
    \internal
    This function is not generally available.
*/
QString MimeType::extension() const
{
    return extensions().first();
}

/*!
    \internal
    This function is not generally available.
*/
QStringList MimeType::extensions() const
{
    loadExtensions();
    return *(*extFor).find(i);
}

/*!
    \internal
    This function is not generally available.
*/
QList<AppLnk> MimeType::applications() const
{
    MimeTypeData* d = data(i);
    return d ? d->apps : QList<AppLnk>();
}

/*!
    Returns the AppLnk defining the application associated
    with this MIME type, or 0 if none is associated.

    The caller must not retain the pointer,
    but of course you can dereference it to take a copy if needed.

    \sa Service::binding()
*/
const AppLnk* MimeType::application() const
{
    MimeTypeData* d = data(i);
    return d ? d->apps.first() : 0;
}

static QString serviceBinding(const QString& service)
{
    // Copied from qtopiaservices
    QString svrc = service;
    for (int i=0; i<(int)svrc.length(); i++)
        if ( svrc[i]=='/' ) svrc[i] = '-';
    return "Service-"+svrc;
}

/*!
    \internal
*/
void MimeType::registerApp( const AppLnk& lnk )
{
    QStringList list = lnk.mimeTypes();
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
	MimeTypeData* cur = data()[*it];
	AppLnk* l = new AppLnk(lnk);
	if ( !cur ) {
	    cur = new MimeTypeData( *it );
	    data().insert( *it, cur );
	    cur->apps.append(l);
	} else if ( cur->apps.count() ) {
	    Config binding(serviceBinding("Open/"+*it));
	    binding.setGroup("Service");
	    QString def = binding.readEntry("default");
	    if ( l->exec() == def )
		cur->apps.prepend(l);
	    else
		cur->apps.append(l);
	} else {
	    cur->apps.append(l);
	}
    }
}

/*!
    \internal
*/
void MimeType::clear()
{
    delete d;
    d = 0;
}

void MimeType::loadExtensions()
{
    if ( !typeFor ) {
	extFor = new QMap<QString,QStringList>;
	typeFor = new QMap<QString,QString>;
	loadExtensions("/etc/mime.types");
	loadExtensions(QPEApplication::qpeDir()+"etc/mime.types");
    }
}

void MimeType::loadExtensions(const QString& filename)
{
    QFile file(filename);
    if ( file.open(IO_ReadOnly) ) {
	QTextStream in(&file);
	QRegExp space("[ \t]+");
	while (!in.atEnd()) {
	    QStringList tokens = QStringList::split(space, in.readLine());
	    QStringList::ConstIterator it = tokens.begin();
	    if ( it != tokens.end() ) {
		QString id = *it; ++it;
		// new override old (though left overrides right)
		QStringList exts = (*extFor)[id];
		QStringList newexts;
		if ( it != tokens.end() ) {
		    exts.remove(*it);
		    if ( !newexts.contains(*it) )
			newexts.append(*it);
		    while (it != tokens.end()) {
			(*typeFor)[*it] = id;
			++it;
		    }
		}
		(*extFor)[id] = newexts + exts;
	    }
	}
    }
}

void MimeType::init( const QString& ext_or_id )
{
    if ( ext_or_id[0] != '/' && ext_or_id.contains('/') ) {
	i = ext_or_id.lower();
    } else {
	loadExtensions();
	int dot = ext_or_id.findRev('.');
	QString ext = dot >= 0 ? ext_or_id.mid(dot+1) : ext_or_id;
	i = (*typeFor)[ext.lower()];
	if ( i.isNull() )
	    i = "application/octet-stream";
    }
    static bool appsUpdated = FALSE;
    if ( !appsUpdated ) {
	appsUpdated = TRUE;
	updateApplications();
    }
}

MimeTypeData* MimeType::data(const QString& id)
{
    MimeTypeData* d = data()[id];
    if ( !d ) {
	int s = id.find('/');
	QString idw = id.left(s)+"/*";
	d = data()[idw];
    }
    return d;
}

/*!
    Returns a Qtopia folder containing application definitions.
*/
QString MimeType::appsFolderName()
{
    return QPEApplication::qpeDir() + "apps";
}

/*!
    Reloads application definitions.
*/
void MimeType::updateApplications()
{
    clear();
    AppLnkSet apps( appsFolderName() );
    updateApplications(&apps);
}

void MimeType::updateApplications(AppLnkSet* folder)
{
    for ( QListIterator<AppLnk> it( folder->children() ); it.current(); ++it ) {
	registerApp(*it.current());
    }
}
