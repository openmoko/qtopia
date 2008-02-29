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
#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <qtopia/applnk.h>
#include <qobject.h>
#include <qimage.h>

class ValueServiceData;
class ServiceRequest;
class QCopEnvelope;

class ValueServiceServer
{
public:
    ValueServiceServer();
    ValueServiceServer( const QString &n, const QCString &c, const QCString &a,
			    const QPixmap &p, const int paramCount = -1 );
    ValueServiceServer( const ValueServiceServer &copy );
    ValueServiceServer &operator=( const ValueServiceServer &copy );

    bool isNull() const;
    QString name() const; 
    QCString channel() const;
    QCString actionName() const;
    QPixmap pixmap() const;
    int paramCount() const;
    
private:
    QString mName;
    QCString mChannel;
    QCString mAction;
    QPixmap mPix;
    int mParamCount;
};

#if defined(Q_OS_WIN32) && _MSC_VER < 1300
enum Type
{
    Get = 0,
    Set 
};
#define TYPE Type
#else
#define TYPE ValueService::Type
#endif

class ValueService : public QObject {
    Q_OBJECT
public:
    enum Type
    {
	Get = 0,
	Set 
    };
    ValueService(QObject* parent, TYPE st, const char* type, const char* params, const char* returns);
    ~ValueService();

    void changeArgs( const char* params );
    int serverCount() const;
    ValueServiceServer server(int serverindex) const;
    void connectToServer(QObject*, const char* signl, int serverindex);

public slots:
    void request(int);

protected:
    virtual void valueSupplied(QDataStream&)=0;
    virtual void sendParameters(QCopEnvelope &);

private slots:
    void valueSupplied(const QString&,QDataStream&);

private:
    ValueServiceData* d;
};

class ImageServerManager : public ValueService {
    Q_OBJECT
public:
    ImageServerManager(QObject* parent);
    ~ImageServerManager();

    void setMaximumSize(int,int);
    void setDefaultImage(const QImage &img);

signals:
    void imageSupplied(const QImage&);
    void pixmapSupplied(const QPixmap&);

protected:
    void valueSupplied(QDataStream&);
    void sendParameters(QCopEnvelope &);

private:
    int maxw,maxh;
    QImage defaultImage;
};

#endif

