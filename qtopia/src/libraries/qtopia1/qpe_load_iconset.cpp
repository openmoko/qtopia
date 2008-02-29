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

// This is #included by qtopia/resource.cpp and
//                      qtopia1/resource1.cpp

extern bool qpe_fast_findPixmap;

/*
  Returns a QIconSet for the pixmap named \a pix.
  You should avoid including any filename type extension (eg. .png, .xpm).
  The icon size is determined by the Qtopia font setting.

  First availability: Qtopia 1.6
*/
static QIconSet qpe_loadIconSet( const QString &pix ) 
{
    static QString iconSmallSize;
    static QString iconLargeSize;

    if ( iconSmallSize.isEmpty() ) {
	Config config( "qpe" );
	config.setGroup( "Appearance" );
	int size = config.readNumEntry("IconSize",0);
	config = Config( "IconSizes" );
	config.setGroup( "IconSizes" );
	QStringList sl = config.readListEntry( "Available", ' ' );

	QStringList::Iterator it;
	QValueList<QSize> available;
	for (it = sl.begin(); it != sl.end(); ++it) {
	    int isRect = (*it).find("x");
	    QSize s;
	    if (isRect > -1) {
		s = QSize((*it).toInt(), (*it).mid(isRect+1).toInt());
	    } else {
		s = QSize((*it).toInt(), (*it).toInt());
	    }
	    if (s.width() > 0 && s.height() > 0)
		available.append(s);
	}

	// probably not that good an idea.
	if (available.isEmpty()) {
	    available.append(QSize(14,14));
	    available.append(QSize(16,16));
	    available.append(QSize(22,22));
	}

	// small is size just smaller than 'size', and large is the one just larger than it.
	// pick largest or smallest if need be.
	QValueList<QSize>::Iterator sit = available.begin();
	QSize large = *sit;
	QSize small = *sit;
	++sit;
	while (sit != available.end()) {
	    QSize candidate = (*sit);
	    if (large.height() < size) {
		if (candidate.height() > large.height())
		    large = candidate;
	    } else {
		if (candidate.height() < large.height() && candidate.height() > size)
		    large = candidate;
	    }
	    if (small.height() > size) {
		if (candidate.height() < small.height())
		    small = candidate;
	    } else {
		if (candidate.height() > small.height() && candidate.height() <= size)
		    small = candidate;
	    }
	    ++sit;
	}
	iconSmallSize.sprintf("%dx%d", small.width(), small.height()); // no tr
	iconLargeSize.sprintf("%dx%d", large.width(), large.height()); // no tr
	qDebug("Choose icon sizes %s and %s", (const char *)iconSmallSize.local8Bit(), (const char *)iconLargeSize.local8Bit());
    }

    //qDebug("Asked to load iconset: %s", pix.latin1());

    QString iname(pix);
    QString bname;
    int sep = pix.findRev('/');
    if (sep != -1) {
	iname = pix.mid(sep+1);
	bname = pix.left(sep+1);
    }

    QString sname = bname + "icons/" + iconSmallSize + "/" + iname;
    QString lname = bname + "icons/" + iconLargeSize + "/" + iname;

    bool oldFast = qpe_fast_findPixmap;
    qpe_fast_findPixmap = TRUE;

    QPixmap spm = Resource::loadPixmap(sname);
    if (spm.isNull())
	qWarning("Cannot load small icon: %s", sname.latin1());
    QPixmap lpm = Resource::loadPixmap(lname);
    if (lpm.isNull())
	qDebug("Cannot load large icon: %s", lname.latin1());

    qpe_fast_findPixmap = oldFast;

    QIconSet is(spm, lpm);
    return is;
}


