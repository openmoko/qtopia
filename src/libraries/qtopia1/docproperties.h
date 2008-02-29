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

#ifndef LNKPROPERTIES_H
#define LNKPROPERTIES_H

#include <qtopia/qpeglobal.h>

#ifdef Q_WS_QWS

#include <qstringlist.h>
#include <qdialog.h>
#include <qcombobox.h>

class AppLnk;
class QListViewItem;
class DocLnk;

class DocPropertiesWidgetPrivate;

class QTOPIA_EXPORT LocationCombo : public QComboBox
{
    Q_OBJECT
public:
    LocationCombo( AppLnk * lnk, QWidget *parent );

    QString pathName() const;
    
    bool isChanged() const;
signals:
    void newPath( const QString& );
    //public slots:    
    //    void apply();
private:
    QStringList locations;
    int currentLocation;
    
    
};


class QTOPIA_EXPORT DocPropertiesWidget : public QWidget
{
    Q_OBJECT
public:
    DocPropertiesWidget( AppLnk* lnk, QWidget* parent = 0 );
    ~DocPropertiesWidget();

public slots:
    void applyChanges();

signals:
    void deleted(); //?????
    void done();
    
protected slots:
    void beamLnk();
    void unlinkLnk();
    void duplicateLnk();
    
    
    
private:
    bool moveLnk();
    bool copyFile( DocLnk &newdoc );

    AppLnk* lnk;
    int fileSize;
    
    DocPropertiesWidgetPrivate *d;
};

class QTOPIA_EXPORT DocPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    DocPropertiesDialog( AppLnk* lnk, QWidget* parent = 0 );
    ~DocPropertiesDialog();

    void done(int);


private:
    DocPropertiesWidget *d;
};

#endif // QWS
#endif // LNKPROPERTIES_H
