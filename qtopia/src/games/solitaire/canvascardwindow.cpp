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

#include "canvascardwindow.h"
#include "patiencecardgame.h"
#include "freecellcardgame.h"
#include "cardmetrics.h"
#ifdef QTOPIA_PHONE
#include "phone_settings.h"
#else
#include "pda_settings.h"
#endif

#include <qtopia/resource.h>
#include <qtopia/global.h>
#ifdef QTOPIA_PHONE
# include <qtopia/contextmenu.h>
# include <qtopia/contextbar.h>
#endif

#include <qapplication.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qaction.h>


enum GameType
{
    SolitaireGame,
    FreecellGame
};


class ZoomView : public QWidget
{
public:
    ZoomView(QWidget *v, QWidget *p) : QWidget(p), view(v) {
	setMask(QBitmap(Resource::findPixmap("cards/mask")));
	lensePixmap = Resource::loadPixmap("cards/lense");
    }
    void updatePosition(QPoint p) {
	pos = p;
	repaint(false);
    }

protected:
    void paintEvent(QPaintEvent *) {
	QPixmap pix = QPixmap::grabWidget(view, pos.x(), pos.y(), width()*3/5, height()*3/5);
	pix.convertFromImage(pix.convertToImage().smoothScale(width(), height()));
	QPainter tmpP(&pix);
	tmpP.drawPixmap(0, 0, lensePixmap);
	QPainter p(this);
	p.drawPixmap(0, 0, pix);
    }

private:
    QPoint pos;
    QPixmap lensePixmap;
    QWidget *view;
};

void CanvasCardView::showWinText(bool b) {
    if (b) {
	wonText1->move((canvas()->width() - wonText1->boundingRect().width()) / 2,
	    (canvas()->height() - wonText1->boundingRect().height()) / 2);
	wonText2->move((canvas()->width() - wonText2->boundingRect().width()) / 2,
	    (canvas()->height() / 2));
	wonText1->show();
	wonText2->show();
    } else {
	wonText1->hide();
	wonText2->hide();
    }
}

CanvasCardView::CanvasCardView(CanvasCardWindow *w, QCanvas *c, QWidget *parent) : QCanvasView(c,parent)
{
    canvasWindow = w;
    cardGame = 0;
    setHScrollBarMode(QScrollView::AlwaysOff);
    setVScrollBarMode(QScrollView::AlwaysOff);
    setMargin(0);
    setLineWidth(0);
    setFocusPolicy(StrongFocus);
    setFocus();
    setFrameStyle(NoFrame);

#if defined(QTOPIA_PHONE)
    if( !Global::mousePreferred() )
	setModalEditing(true);

    ContextBar::setLabel(this, Qt::Key_Back, ContextBar::Cancel, ContextBar::Modal);
    zoomView = new ZoomView(this,parent);
    zoomView->setFixedSize(60,40);
    zoomView->hide();
#else
    zoomView = 0;
#endif
    QFont fnt(QApplication::font());
    fnt.setPointSize( 16 );
    fnt.setBold(TRUE);
    wonText1 = new QCanvasText(tr("Congratulations!\n"),c);
    wonText1->setFont( fnt );
    wonText1->setColor(Qt::red);
    wonText2 = new QCanvasText(tr("You won!\n"),c);
    wonText2->setFont( fnt );
    wonText2->setColor(Qt::red);
}


void CanvasCardView::setCardGame(CanvasCardGame *game)
{
    cardGame = game;
    if ( game )
	connect(game, SIGNAL(moveFinished()), this, SLOT(updateZoomView()));
}


void CanvasCardView::setZoomViewEnabled(bool b)
{
    if (zoomView) {
	if (b)
	    zoomView->show();
	else
	    zoomView->hide();
    }
}


void CanvasCardView::updateZoomPos()
{
    if ( zoomView )
	zoomView->move((width()-zoomView->width())/2, height()-zoomView->height());
}


void CanvasCardView::updateZoomView()
{
    if ( zoomView ) {
	QRect r = cardGame->selectionRect();
	QPoint pos = r.topLeft() - QPoint(7,6);
	zoomView->move((width()-zoomView->width())/2, height()-zoomView->height());
	if ( r.intersects(zoomView->geometry()) ) {
	    if ( pos.x() > width() / 2 ) 
		zoomView->move(20, height()-zoomView->height());
	    else 
		zoomView->move(width()-(zoomView->width()+20), height()-zoomView->height());
	}
	zoomView->updatePosition(pos);
    }
}


void CanvasCardView::keyPressEvent(QKeyEvent *ke)
{
    if ( cardGame ) {
	cardGame->keyPressEvent(ke);
	updateZoomView();
    } else {
	ke->ignore();
    }
}


void CanvasCardView::contentsMousePressEvent(QMouseEvent *e)
{
    if ( cardGame )
	cardGame->contentsMousePressEvent(e);
}


void CanvasCardView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if ( cardGame )
	cardGame->contentsMouseReleaseEvent(e);
}


void CanvasCardView::contentsMouseMoveEvent(QMouseEvent *e)
{
    if ( cardGame )
	cardGame->contentsMouseMoveEvent(e);
}


void CanvasCardView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    if ( cardGame )
	cardGame->contentsMouseDoubleClickEvent(e);
}


CanvasCardWindow::CanvasCardWindow(QWidget* parent, const char* name, WFlags f) :
    QMainWindow(parent, name, f),
    resizeTimeout(this, "resizeTimeout"),
    canvas(1, 1),
    casinoRules(false),
    zoomOn(false),
    snapOn(true),
    drawThree(true),
    cardBack(0),
    gameType(0),
    resizing(false),
    cardGame(0),
    timeout(250)
{
    setIcon( Resource::loadPixmap( "cards" ) );

    connect( &resizeTimeout, SIGNAL( timeout() ), this, SLOT( doResize() ) );

    // Create Playing Area for Games
    if ( QPixmap::defaultDepth() < 12 ) {
	canvas.setBackgroundColor(QColor(0x08, 0x98, 0x2D));
    } else {
        QPixmap bg;
	bg.convertFromImage( Resource::loadImage( "cards/table_pattern" ), ThresholdDither );  
        canvas.setBackgroundPixmap(bg);
    }

#if defined( QT_QWS_CASSIOPEIA )
    canvas.setAdvancePeriod(70);
#else
    canvas.setAdvancePeriod(30);
#endif

    newPatGame  = new QAction( tr("Patience"), tr("New Patience Game"), 0, this );
    newFreGame  = new QAction( tr("Freecell"), tr("New Freecell Game"), 0, this );
    settingsAct = new QAction( tr("Settings"), tr("Settings"), 0, this );

    connect( newPatGame,  SIGNAL(activated()), this, SLOT(initPatience()) );
    connect( newFreGame,  SIGNAL(activated()), this, SLOT(initFreecell()) );
    connect( settingsAct, SIGNAL(activated()), this, SLOT(doSettings()) );

#ifdef QTOPIA_PHONE
    // Create context menu
    ContextMenu *contextMenu = new ContextMenu( this );

    // Add actions to the context menu.
    newPatGame->addTo( contextMenu );
    newFreGame->addTo( contextMenu );
    settingsAct->addTo( contextMenu );
#else
    // Create menus
    QMenuBar* menu = menuBar();
    QPopupMenu* file = new QPopupMenu;
    menu->insertItem(tr("Play", "play game"), file);

    // Add actions to menus.
    newPatGame->addTo(file);
    newFreGame->addTo(file);
    int id = menu->insertItem(tr("Settings"));//, this, SLOT(doSettings()) );
    menu->connectItem( id, this, SLOT(doSettings()) );
    menu->show();
#endif

    canvasView = new CanvasCardView(this, &canvas, this);
    setCentralWidget(canvasView);
}


CanvasCardWindow::~CanvasCardWindow()
{
    closeGame();
}


void CanvasCardWindow::initGame()
{
    Config cfg("Patience");
    cfg.setGroup("GlobalSettings");
    snapOn = cfg.readBoolEntry("SnapOn", TRUE);
    zoomOn = cfg.readBoolEntry("ZoomOn", TRUE);
    casinoRules = cfg.readBoolEntry("CasinoRules", FALSE);
    gameType = cfg.readNumEntry("GameType", -1);
    drawThree = cfg.readBoolEntry("DrawThree", TRUE);
    cardBack = cfg.readNumEntry("CardBack", 0);
    canvasView->setZoomViewEnabled(zoomOn);
    CardMetrics::setCardBack(cardBack);

    if ( gameType >= 0 ) 
	initGame(false, gameType);
    else // Probably broken or no config file, start a new game
	initPatience();
}


void CanvasCardWindow::closeGame()
{
    if ( cardGame ) {
	Config cfg("Patience");
	cfg.setGroup( "GlobalSettings" );
	cfg.writeEntry( "GameType", gameType );
	cfg.writeEntry( "SnapOn", snapOn );
	cfg.writeEntry( "ZoomOn", zoomOn );
	cfg.writeEntry( "CasinoRules", casinoRules );
	cfg.writeEntry( "DrawThree", drawThree);
	cfg.writeEntry( "CardBack", cardBack );
	cardGame->writeConfig( cfg );
	delete cardGame;
	cardGame = 0;
	canvasView->setCardGame(0);
    }
}


void CanvasCardWindow::showEvent( QShowEvent * )
{
    QSize s = canvasView->viewport()->size();
    canvas.resize( s.width(), s.height() );
    CardMetrics::loadMetrics( s.width(), s.height() );
}


void CanvasCardWindow::resizeEvent( QResizeEvent *re )
{
    if ( !resizing ) {
	closeGame();
	resizing = true;
    }
    QMainWindow::resizeEvent( re );
    QSize s = canvasView->viewport()->size();
    canvas.resize( s.width(), s.height() );
    resizeTimeout.start( timeout, TRUE );
    canvasView->updateZoomPos();
}


void CanvasCardWindow::doResize()
{
    if ( resizing ) {
	QSize s = canvasView->viewport()->size();
	CardMetrics::loadMetrics( s.width(), s.height() );
	initGame();
	resizing = false;
    }
}


void CanvasCardWindow::initGame( bool newGame, int type )
{
    canvasView->setZoomViewEnabled(zoomOn);
    canvasView->showWinText(FALSE);

    // Create New Game 
    if ( cardGame ) 
	delete cardGame;

    if ( type == SolitaireGame )
        cardGame = new PatienceCardGame(&canvas, canvasView, snapOn, casinoRules);
    else
	cardGame = new FreecellCardGame(&canvas, canvasView, snapOn, casinoRules);

    canvasView->setCardGame(cardGame);

    cardGame->setCardsDrawn(drawThree ? 3 : 1);
    gameType = type;
    setCaption(cardGame->gameName());

    if ( newGame )
	cardGame->newGame();
    else {
	Config cfg("Patience");
	cfg.setGroup( "GlobalSettings" );
	cardGame->readConfig( cfg );
    }
    setCardBacks();

    cardGame->resetCardSelection(CanvasCardGame::CurrentCard);
    canvasView->updateZoomView();
}


void CanvasCardWindow::doSettings()
{
    int origCardBack = cardBack;

    Settings settings(this, 0, WType_Modal);
    settings.Draw3Cards->setChecked(drawThree);
    settings.SnapCheckBox->setChecked(snapOn);
#ifdef QTOPIA_PHONE
    settings.ZoomCheckBox->setChecked(zoomOn);
#endif
    settings.CasinoRulesCheckBox->setChecked(casinoRules);

    CardMetrics::setCardBack(0);
    settings.CardBackLabel1->setPixmap(*CardMetrics::backPixmap());
    settings.CardBackButton1->setChecked(cardBack==0);
    CardMetrics::setCardBack(1);
    settings.CardBackLabel2->setPixmap(*CardMetrics::backPixmap());
    settings.CardBackButton2->setChecked(cardBack==1);
    CardMetrics::setCardBack(2);
    settings.CardBackLabel3->setPixmap(*CardMetrics::backPixmap());
    settings.CardBackButton3->setChecked(cardBack==2);
    CardMetrics::setCardBack(3);
    settings.CardBackLabel4->setPixmap(*CardMetrics::backPixmap());
    settings.CardBackButton4->setChecked(cardBack==3);
    CardMetrics::setCardBack(4);
    settings.CardBackLabel5->setPixmap(*CardMetrics::backPixmap());
    settings.CardBackButton5->setChecked(cardBack==4);
    CardMetrics::setCardBack(cardBack);

    if ( !Global::mousePreferred() )
        settings.SnapCheckBox->hide();

#if 0
    // ### Previous it has been reported as a bug that
    // when playing freecell, the settings menus contained
    // unapplicable settings for changing the card backs etc.
    if ( gameType == FreecellGame ) {
	settings.Draw1Card->hide();
	settings.Draw3Cards->hide();
	settings.SnapCheckBox->hide();
	settings.CardBackButton1->hide();
	settings.CardBackButton2->hide();
	settings.CardBackButton3->hide();
	settings.CardBackButton4->hide();
	settings.CardBackButton5->hide();
    }
#endif

    settings.showMaximized();
    
    if ( settings.exec() ) {
	drawThree = settings.Draw3Cards->isChecked();
	cardGame->setCardsDrawn(drawThree ? 3 : 1);

	snapOn = settings.SnapCheckBox->isChecked();
	cardGame->setSnap(snapOn);

	casinoRules = settings.CasinoRulesCheckBox->isChecked();
	cardGame->setCasinoRules(casinoRules);

#ifdef QTOPIA_PHONE
	zoomOn = settings.ZoomCheckBox->isChecked();
#else
	zoomOn = false;
#endif
	canvasView->setZoomViewEnabled(zoomOn);

	if ( settings.CardBackButton1->isChecked() )
	    cardBack = 0;
	else if ( settings.CardBackButton2->isChecked() )
	    cardBack = 1;
	else if ( settings.CardBackButton3->isChecked() )
	    cardBack = 2;
	else if ( settings.CardBackButton4->isChecked() )
	    cardBack = 3;
	else if ( settings.CardBackButton5->isChecked() )
	    cardBack = 4;

	if (cardBack != origCardBack) {
	    CardMetrics::setCardBack( cardBack );
	    setCardBacks();
	}
    }
}


void CanvasCardWindow::initPatience()
{
    initGame( true, SolitaireGame );
}


void CanvasCardWindow::initFreecell()
{
    initGame( true, FreecellGame );
}


void CanvasCardWindow::setCardBacks()
{
    QCanvasItemList l = canvas.allItems();
    for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it) {
	if ( (*it)->rtti() == canvasCardId )
	    ((CanvasCard *)(*it))->cardBackChanged();
    }
}

