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

#ifndef __CATEGORYCOMBO_H__
#define __CATEGORYCOMBO_H__

#include <qtopia/qpeglobal.h>
#include <qcombobox.h>
#include <qtoolbutton.h>
#include <qhbox.h>
#include <qstring.h>
#include <qarray.h>

QTOPIA_EXPORT extern QString categoryFileName();

class CategoryComboPrivate;
class QTOPIA_EXPORT CategoryCombo : public QComboBox
{
    Q_OBJECT

public:
    CategoryCombo( QWidget *parent, const char* name = 0, int width=0);

    ~CategoryCombo();

    int currentCategory() const;
    void setCurrentCategory( int id );
    // depreciated.
    void initCombo( const QArray<int> &recCats, const QString &appName );
    void initCombo( const QArray<int> &recCats, const QString &appName,
		    const QString &visibleName /* = appName */ );

    QArray<int> initComboWithRefind( const QArray<int> &recCats, const QString &appName );

signals:
    void sigCatChanged( int newUid );

private slots:
    void slotValueChanged( int );

private:
    void setCurrentText( const QString &str );
    CategoryComboPrivate *d;
};

class CategorySelectPrivate;
class QTOPIA_EXPORT CategorySelect : public QHBox
{
    Q_OBJECT
public:
    // we need two constructors, the first gets around designer limitations

    CategorySelect( QWidget *parent = 0, const char *name = 0, int width = 0 );

    CategorySelect( const QArray<int> &vlCats, const QString &appName,
		    QWidget *parent = 0, const char *name = 0,
		    int width = 0);
    CategorySelect( const QArray<int> &vlCats, const QString &appName,
		    const QString &visibleName, QWidget *parent = 0,
		    const char *name = 0 , int width = 0);
    CategorySelect( const QArray<int> &vlCats, const QString &appName,
		    const QString &visibleName, bool allCategories, QWidget *parent = 0,
		    const char *name = 0 , int width = 0); // libqtopia

    ~CategorySelect();

    enum SelectorWidget { ComboBox, ListView };
    SelectorWidget widgetType() const; // libqtopia

    bool containsCategory( const QArray<int>& categories ); // libqtopia

    // these were added for find dialog.
    void setRemoveCategoryEdit( bool remove );
    void setAllCategories( bool add );
    void setCurrentCategory( int newCatUid );
    int currentCategory() const;

    const QArray<int> &currentCategories() const;

    // pretty much if you don't set it the constructor, you need to
    // call it here ASAP!
    // however this call is depreciated...

    QString setCategories( const QArray<int> &vlCats, const QString &appName );
    QString setCategories( const QArray<int> &vlCats, const QString &appName,
			const QString &visibleName );

    void setFixedWidth(int width);
signals:
    void signalSelected( int );
    void editCategoriesClicked(QWidget*);

private slots:
    void slotDialog();
    void categoriesChanged();

public slots:
    void slotNewCat( int id );

private:
    void init(int width);
    QString mStrAppName;
    CategoryCombo *cmbCat;
    QToolButton *cmdCat;
    CategorySelectPrivate *d;
};


#define ALL_CATEGORIES_VALUE	    -2
#define UNFILED_CATEGORIES_VALUE    -1

#endif

