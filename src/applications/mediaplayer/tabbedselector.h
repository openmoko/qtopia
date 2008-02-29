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
#include <qpe/applnk.h>
#include <qtabwidget.h>


class TabbedSelectorPrivate;
class PlayListSelection;
class FileSelector;


class TabbedSelector : public QTabWidget {
Q_OBJECT
    public:
	TabbedSelector( QWidget *parent = 0, const char *name = 0 );
	virtual ~TabbedSelector();

	PlayListSelection *audioFiles();
	PlayListSelection *videoFiles();
	PlayListSelection *selectedFiles();

	void startLoading();
	void pauseLoading();
	void resumeLoading();

	enum AVTab {
	    AudioTab = 0,
	    VideoTab,
	    PlaylistTab
	};

	AVTab tab();

    public slots:
	void showPlaylistTab();
	void showNextTab();
	void showPrevTab();

	void addLink( const DocLnk& doc );
	void removeAll();
	void categoryChanged();
	void locationChanged();

	void moveSelectedUp();
	void moveSelectedDown();

	void addSelected();
	void addASelected();
	void addVSelected();
	void addAllAudio();
	void addAllVideo();
	void addAll();
	void clearList();

    private:
	TabbedSelectorPrivate *d;
};


