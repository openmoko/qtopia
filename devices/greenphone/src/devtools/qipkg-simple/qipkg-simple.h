/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIPKG_H
#define QIPKG_H
#include <qtopiaabstractservice.h>
#include "ui_qipkgbase.h"

class QipkgBase : public QWidget, public Ui_QipkgBase
{
public:
    QipkgBase( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~QipkgBase();
};

class Qipkg : public QipkgBase
{ 
    Q_OBJECT
public:
    Qipkg( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~Qipkg();

public slots:
    void setDocument( const QString& ipkg );

private slots:
    void start();
    void finished();

private:
    QString filename;
};

class QipkgService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class Qipkg;
private:
    QipkgService( Qipkg *parent )
        : QtopiaAbstractService( "Qipkg", parent )
        { this->parent = parent; publishAll(); }

public:
    ~QipkgService();

public slots:
    void setDocument( const QString& ipkg );

private:
    Qipkg *parent;
};


#endif // QIPKG_H
