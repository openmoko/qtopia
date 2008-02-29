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
#ifndef MEDIA_SELECTOR_H
#define MEDIA_SELECTOR_H


#include <qlistview.h>
#include <qtopia/applnk.h>


class MediaSelectorData;
class MediaSelectorItem;
class QComboBox;


class ContainsCategoryInterface {
    public:
	virtual bool containsCategoryInterface( const QArray<int>& categories ) = 0;
	virtual void connectSelectedToSlot(QObject *obj, const char *slot) = 0;
	virtual int currentCategoryId() = 0;
	virtual void setCurrentCategoryId(int id) = 0;
};


class MediaSelectorItem : public QCheckListItem {
public:
    MediaSelectorItem( QListView *parent, const DocLnk &f );
    const DocLnk &file() const;
    void setFile( const DocLnk& f );
    QString key(int column, bool ascending) const;
    
    void setSelectable( bool b ) { selectable = b; }
    
protected:
    virtual void paintCell( QPainter *p, const QColorGroup &cg,
                               int column, int width, int align );
private:
    bool selectable;
    DocLnk fl;
    QString fileSizeSortString;
};


class MediaSelector : public QListView {
    Q_OBJECT
public:
    MediaSelector( QWidget *parent, const char *name=0 );
    ~MediaSelector();

    const DocLnk *current(); // retrieve the current playlist entry (media file link)
    
    // Display check box for all new items
    void setSelectable( bool b );

    void setCategoryFilter( ContainsCategoryInterface* );
    void setLocationFilter( QComboBox* );
    bool advanceSelector();

    MediaSelectorItem *newItem( const DocLnk& lnk );

    QBrush altBrush() const;

public slots:
    void removeAll();
    void addLink( const DocLnk& );
    void removeLink( const DocLnk& );
    void changeLink( const DocLnk&, const DocLnk& );

    void addToSelector( const DocLnk& ); // Add a media file to the playlist
    void removeSelected();   // Remove a media file from the playlist
    void moveSelectedUp();   // Move the media file up the playlist so it is played earlier
    void moveSelectedDown(); // Move the media file down the playlist so it is played later
    bool prev();
    bool next();
    bool first();
    bool last();

    void categorize();
    void selectAll();
    void addSelectionToCategory(int catId);

    void showItemProperties( QListViewItem *i );

protected:
    virtual void viewportResizeEvent( QResizeEvent* );
    virtual void contentsMouseMoveEvent( QMouseEvent* );
    virtual void contentsMousePressEvent( QMouseEvent* );
    virtual void contentsMouseReleaseEvent( QMouseEvent* );
    virtual void paletteChange( const QPalette &);
    virtual void keyPressEvent(QKeyEvent *ke);

private:
    bool filtersMatch( const DocLnk& lnk );
    void deleteItem( MediaSelectorItem *item );
    bool sbVis;
    QBrush stripebrush;

    MediaSelectorData *d;
};


#endif // MEDIA_SELECTOR_H


