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
#ifndef LAUNCHERVIEW_H
#define LAUNCHERVIEW_H

#include <qtopia/storage.h>
#include <qtopia/applnk.h>

#include <qvbox.h>

class CategorySelect;
class LauncherIconView;
class QIconView;
class QIconViewItem;
class QLabel;
class QWidgetStack;
class MenuButton;
class QComboBox;

class LauncherView : public QVBox
{
    Q_OBJECT

public:
    LauncherView( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LauncherView();

    void hideIcons();

    bool removeLink(const QString& linkfile);
    void addItem(AppLnk* app, bool resort=TRUE);
    void removeAllItems();
    void setSortEnabled(bool);
    void setUpdatesEnabled(bool);
    void sort();

    void setToolsEnabled(bool);
    void updateTools();

    void setBusy(bool);

    enum ViewMode { Icon, List };
    void setViewMode( ViewMode m );
    ViewMode viewMode() const { return vmode; }

    enum BackgroundType { Ruled, SolidColor, Image };
    void setBackgroundType( BackgroundType t, const QString & );
    BackgroundType backgroundType() const { return bgType; }

    void setStaticBackground(const QPixmap &pm);

    void setTextColor( const QColor & );
    QColor textColor() const { return textCol; }

    void setViewFont( const QFont & );
    void clearViewFont();

    void relayout(void);
    void resetSelection();

    QStringList mimeTypes() const;
    const AppLnk *currentItem() const;

    uint count() const;

public slots:
    void showType(const QString &);
    void showCategory( int );

signals:
    void clicked( const AppLnk * );
    void rightPressed( const AppLnk * );

protected slots:
    void selectionChanged();
    void returnPressed( QIconViewItem *item );
    void itemClicked( int, QIconViewItem * );
    void itemPressed( int, QIconViewItem * );
    void sortBy(int);
    void showType(int);
    void resizeEvent(QResizeEvent *);
    void flushBgCache();

protected:
    void paletteChange( const QPalette & );
    void fontChange(const QFont &);

private:
    static bool bsy;
    QWidget* tools;
    LauncherIconView* icons;
    QComboBox *typemb;
    QStringList typelist;
    CategorySelect *catmb;
    ViewMode vmode;
    BackgroundType bgType;
    QString bgValue;
    QString bgName;
    QColor textCol;

    QImage loadBackgroundImage(QString &fname);
};

#endif // LAUNCHERVIEW_H
