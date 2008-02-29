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

#define QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_LANGLIST

#include "localtr_p.h"

#include <qtopia/qpeapplication.h>

#include <qdir.h>
#include <qlist.h>
#include <qdict.h>
#include <qtranslator.h>

static QDict< QList<QTranslator> > *trmap=0;

#include "global_qtopiapaths.cpp"

static QList<QTranslator>* Local_translations(const QString& key)
{
    // Like qtopia_loadTranslations(), but not installed into qApp,
    // but rather just stored locally, and only loaded once.

    if ( !trmap )
	trmap = new QDict< QList<QTranslator> >;

    QList<QTranslator>* l = trmap->find(key);
    if ( l )
	return l;

    l = new QList<QTranslator>;

    QStringList langs = Global::languageList();

    QStringList qpepaths = global_qtopiapaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
	for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
	    QString lang = *it;

	    QString d = *qit + "i18n/" + lang + "/";
	    QDir dir(d, key + ".qm", QDir::Unsorted, QDir::Files);
            if (!dir.exists())
                continue;
	    for ( int i=0; i<(int)dir.count(); i++ ) {
		QTranslator * trans = new QTranslator(qApp);
		if ( trans->load( d+dir[i] ))
		    l->append(trans);
		else
		    delete trans;
	    }
	}
    }

    trmap->insert(key,l);

    return l;
}

/*!
  Translate \a str in the context \a c, using only the translation files defined by \a key.
  The \a key may be either a single name, such as "QtopiaSettings", or it may be a
  'wildcard', such as "Categories-*". The \a key thus defines the set of translation files
  in which to search for a translation.
*/
QString LocalTranslator::translate(const QString& key, const QCString& c, const QString& str)
{
    QList<QTranslator>* l = Local_translations(key);
    if ( !l )
	return str;
    QTranslator *t;
    for (QListIterator<QTranslator> it(*l); (t=*it); ++it) {
	QTranslatorMessage msg = t->findMessage(c.data(),str.utf8().data(),0);
	if ( !msg.translation().isNull() )
	    return msg.translation();
    }
    return str;
}
