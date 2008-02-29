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
#ifndef MEDIA_SELECTOR_WIDGET_H
#define MEDIA_SELECTOR_WIDGET_H


#include <qmainwindow.h>
#include "mediaplayerstate.h"
#include "mediaselectorbase.h"
#include <qvbox.h>


class MediaSelectorWidgetPrivate;


class MediaSelectorWidget : public QMainWindow, public MediaSelectorBase {
    Q_OBJECT
public:
    MediaSelectorWidget( QWidget *parent, const char *name, QString filter, bool videoButtons, WFlags fl=0 );
    ~MediaSelectorWidget();

    // Return true if able to parse playlist
    bool isPlaylist( const QString& file );
    
    // Parse and construct playlist
    void setPlaylist( const QString& file );

    void loadFiles();

    // retrieve the current playlist entry (media file link)
    const DocLnk *current();

    // Little helper function to test if we should handle a key press or not
    bool listHasFocus();

    QWidget *widget() { return this; }

public slots:
    void setDocument( const QString& fileref );
    void setActiveWindow(); // need to handle this to show the right view
    void setView( View );
    void viewClosed();
    void next();
    void prev();
    void showInfo();
    void skinChanged(int id);
    void scanProgress(int percent);
    void selectCategory();
    void sort();
    void shuffle();
    void selectAll();
    void categorize();

protected:
    void resizeEvent( QResizeEvent *re );

private:
    enum Mode { Document, Playlist };

    void setMode( Mode );
    
    // Toggle actions dependent on media items
    void toggleActions();

    void initializeStates();
    MediaSelectorWidgetPrivate *d; // Private implementation data
};


#endif // MEDIA_SELECTOR_WIDGET_H

