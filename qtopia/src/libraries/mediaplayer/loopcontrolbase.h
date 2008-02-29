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
#ifndef LOOP_CONTROL_BASE_H
#define LOOP_CONTROL_BASE_H


#include <qobject.h>


class LoopControlBase : public QObject {
public:
    LoopControlBase(QObject *parent, const char *name) : QObject(parent,name) { }
    virtual ~LoopControlBase() { }
    virtual bool init(const QString& filename, const QString& mimetype = QString::null, bool isURL = false) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual long totalPlaytime() const = 0;
    virtual bool hasVideo() const = 0;
};


#endif // LOOP_CONTROL_BASE_H

