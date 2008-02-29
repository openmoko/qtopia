/* ---------------------------------------------------------------------- */
/*                                                                        */
/* [main.C]                        Konsole                                */
/*                                                                        */
/* ---------------------------------------------------------------------- */
/*                                                                        */
/* Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>        */
/*                                                                        */
/* This file is part of Konsole, an X terminal.                           */
/*                                                                        */
/* The material contained in here more or less directly orginates from    */
/* kvt, which is copyright (c) 1996 by Matthias Ettrich <ettrich@kde.org> */
/*                                                                        */
/* ---------------------------------------------------------------------- */
//
// KDE's Konsole, ported to Qt/Embedded
//
// Copyright (C) 2000 by John Ryland <jryland@trolltech.com>
//  some enhancements added by L.J. Potter <ljp@llornkcor.com>
//

/*! \class Konsole
  \brief The Konsole class implements the Konsole application.

  \legalese

  EmbeddedKonsole is a port of the KDE program Konsole. The primary copyright holder
  is Lars Doelle <lars.doelle@on-line.de>.  It is distributed under the terms of the 
  GNU General Public License.
*/


#include <qtopia/resource.h>

#include <qdir.h>
#include <qevent.h>
#include <qdragobject.h>
#include <qobjectlist.h>
#include <qtoolbutton.h>
#include <qtopia/qpetoolbar.h>
#include <qpushbutton.h>
#include <qfontdialog.h>
#include <qglobal.h>
#include <qpainter.h>
#include <qtopia/qpemenubar.h>
#include <qmessagebox.h>
#include <qaction.h>
#include <qapplication.h>
#include <qfontmetrics.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qtabwidget.h>
#include <qtabbar.h>
#include <qtopia/config.h>
#include <qstringlist.h>
#include <qpalette.h>

#ifndef Q_OS_WIN32 
#include <sys/wait.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "konsole.h"
#include "keytrans.h"

class EKNumTabBar : public QTabBar {
public:
    void numberTabs()
    {
  // Yes, it really is this messy. QTabWidget needs functions
  // that provide acces to tabs in a sequential way.
  int m=INT_MIN;
  for (int i=0; i<count(); i++) {
      QTab* left=0;
      QListIterator<QTab> it(*tabList());
      int x=INT_MAX;
      for( QTab* t; (t=it.current()); ++it ) {
    int tx = t->rect().x();
    if ( tx<x && tx>m ) {
        x = tx;
        left = t;
    }
      }
      if ( left ) {
    left->setText(QString::number(i+1));
    m = left->rect().x();
      }
  }
    }
};

class EKNumTabWidget : public QTabWidget {
public:
    EKNumTabWidget(QWidget* parent) : QTabWidget(parent)
    {
    }

    void addTab(QWidget* w)
    {
  QTab* t = new QTab(QString::number(tabBar()->count()+1));
  QTabWidget::addTab(w,t);
    }

    void removeTab(QWidget* w)
    {
  removePage(w);
  ((EKNumTabBar*)tabBar())->numberTabs();
    }
};

// This could be configurable or dynamicly generated from the bash history
// file of the user
static const char *commonCmds[] =
{
    "ls ", // I left this here, cause it looks better than the first alpha
    "cardctl eject", // No tr
    "cat ", // No tr
    "cd ",
    "chmod ",
    "cp ",
    "dc ",
    "df ",
    "dmesg",
    "echo ", // No tr
    "find ", // No tr
    "free", // No tr
    "grep ",
    "ifconfig ",
    "ipkg ",
    "mkdir ",
    "mv ",
    "nc localhost 7776",
    "nc localhost 7777",
    "nslookup ",
    "ping ", // No tr
    "ps aux",
    "pwd ",
    "rm ",
    "rmdir ",
    "route ", // No tr
    "set ", // No tr
    "traceroute",

/*
    "gzip",
    "gunzip",
    "chgrp",
    "chown",
    "date",
    "dd",
    "df",
    "dmesg",
    "fuser",
    "hostname",
    "kill",
    "killall",
    "ln",
    "ping",
    "mount",
    "more",
    "sort",
    "touch",
    "umount",
    "mknod",
    "netstat",
*/

    "exit", // No tr
    NULL
};

/*
  \class Konsole konsole.h
  \brief The Konsole class is the top-level widget.
  \legalese

  The Terminal (embeddedkonsole) is based on the KDE Konsole
  application and is distributed under the terms of the GNU
  General Public License. A primary copyright holder of the
  code is Lars Doelle &lt;lars.doelle@on-line.de&gt;.
*/

Konsole::Konsole(QWidget* parent, const char* name, WFlags fl) :
    QMainWindow(parent, name, fl)
{
    QStrList args;
    init("/bin/sh",args);
}

Konsole::Konsole(const char* name, const char* _pgm, QStrList & _args, int)
 : QMainWindow(0, name)
{
    init(_pgm,_args);
}

void Konsole::init(const char* _pgm, QStrList & _args)
{
  b_scroll = TRUE; // histon;
  n_keytab = 0;
  n_render = 0;

  setCaption( tr("Terminal") );
  setIcon( Resource::loadPixmap( "konsole" ) );

  setBackgroundMode( PaletteButton );

  Config cfg("Konsole");
  cfg.setGroup("Konsole");
  QString tmp;
  // initialize the list of allowed fonts ///////////////////////////////////
  cfont = cfg.readNumEntry("FontID", 1);
  QFont f = QFont("Micro", 4, QFont::Normal);
  f.setFixedPitch(TRUE);
  fonts.append(new VTFont(tr("Micro"), f));

  f = QFont("Fixed", 7, QFont::Normal);
  f.setFixedPitch(TRUE);
  fonts.append(new VTFont(tr("Small Fixed"), f));

  f = QFont("Fixed", 12, QFont::Normal);
  f.setFixedPitch(TRUE);
  fonts.append(new VTFont(tr("Medium Fixed"), f));

  // create terminal emulation framework ////////////////////////////////////
  nsessions = 0;

  tab = new EKNumTabWidget(this);

  connect(tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(switchSession(QWidget*)));

     // create terminal toolbar ////////////////////////////////////////////////
  setToolBarsMovable( FALSE );
  QPEToolBar *menuToolBar = new QPEToolBar( this );
  menuToolBar->setHorizontalStretchable( TRUE );

  QPEMenuBar *menuBar = new QPEMenuBar( menuToolBar );
  uint i;
  fontList = new QPopupMenu( this );
  for(i = 0; i < fonts.count(); i++) {
    VTFont *fnt = fonts.at(i);
    fontList->insertItem(fnt->getName(), i);
  }
  fontChanged(cfont);

  configMenu = new QPopupMenu( this);
  colorMenu = new QPopupMenu( this);

  bool listHidden;
  cfg.setGroup("Menubar");
  if( cfg.readEntry("Hidden","FALSE") == "TRUE")  {
      configMenu->insertItem(tr("Show command list"));
      listHidden=TRUE;
  } else {
      configMenu->insertItem(tr("Hide command list"));
      listHidden=FALSE;
  }

  cfg.setGroup("Tabs");
  tmp=cfg.readEntry("Position","Bottom");
  if(tmp=="Top") { // No tr
      tab->setTabPosition(QTabWidget::Top);
      configMenu->insertItem(tr("Tabs on Bottom"));
  } else {
      tab->setTabPosition(QTabWidget::Bottom);
      configMenu->insertItem(tr("Tabs on Top"));
  }
  configMenu->insertSeparator(2);

  colorMenu->insertItem(tr("Green on Black"));
  colorMenu->insertItem(tr("Black on White"));
  colorMenu->insertItem(tr("White on Black"));
  colorMenu->insertItem(tr("Black on Transparent"));
  colorMenu->insertItem(tr("Black on Red"));
  colorMenu->insertItem(tr("Red on Black"));
  colorMenu->insertItem(tr("Green on Yellow"));
  colorMenu->insertItem(tr("Blue on Magenta"));
  colorMenu->insertItem(tr("Magenta on Blue"));
  colorMenu->insertItem(tr("Cyan on White"));
  colorMenu->insertItem(tr("White on Cyan"));
  colorMenu->insertItem(tr("Blue on Black"));
  configMenu->insertItem(tr("Colors"),colorMenu);

  connect( fontList, SIGNAL( activated(int) ), this, SLOT( fontChanged(int) ));
  connect( configMenu, SIGNAL( activated(int) ), this, SLOT( configMenuSelected(int) ));
  connect( colorMenu, SIGNAL( activated(int) ), this, SLOT( colorMenuSelected(int) ));

  menuBar->insertItem( tr("Font"), fontList );
  menuBar->insertItem( tr("Options"), configMenu );

  QPEToolBar *toolbar = new QPEToolBar( this );

  QAction *a;

  // Button Commands
  a = new QAction( tr("New"), Resource::loadIconSet( "konsole" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( newSession() ) ); a->addTo( toolbar );
  a = new QAction( tr("Enter"), Resource::loadIconSet( "konsole/enter" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitEnter() ) ); a->addTo( toolbar );
  a = new QAction( tr("Space"), Resource::loadIconSet( "konsole/space" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitSpace() ) ); a->addTo( toolbar );
  a = new QAction( tr("Tab"), Resource::loadIconSet( "konsole/tab" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitTab() ) ); a->addTo( toolbar );
  a = new QAction( tr("Up"), Resource::loadIconSet( "konsole/up" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitUp() ) ); a->addTo( toolbar );
  a = new QAction( tr("Down"), Resource::loadIconSet( "konsole/down" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitDown() ) ); a->addTo( toolbar );
  a = new QAction( tr("Paste"), Resource::loadIconSet( "paste" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitPaste() ) ); a->addTo( toolbar );
/*
  a = new QAction( tr("Up"), Resource::loadIconSet( "up" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitUp() ) ); a->addTo( toolbar );
  a = new QAction( tr("Down"), Resource::loadIconSet( "down" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitDown() ) ); a->addTo( toolbar );
*/

  secondToolBar = new QPEToolBar( this );

  commonCombo = new QComboBox( secondToolBar );

  if( listHidden) 
      secondToolBar->hide();
  configMenu->insertItem(tr("Edit Command List"));

  cfg.setGroup("Commands");
  commonCombo->setInsertionPolicy(QComboBox::AtCurrent);

  for (i = 0; commonCmds[i] != NULL; i++) {
      commonCombo->insertItem( commonCmds[i], i );
      tmp = cfg.readEntry( QString::number(i),"");
      if(tmp != "")
          commonCombo->changeItem( tmp,i );
  }

  connect( commonCombo, SIGNAL( activated(int) ), this, SLOT( enterCommand(int) ));

      // create applications /////////////////////////////////////////////////////
  setCentralWidget(tab);

  // load keymaps ////////////////////////////////////////////////////////////
  KeyTrans::loadAll();
  for (int i = 0; i < KeyTrans::count(); i++)
  { KeyTrans* s = KeyTrans::find(i);
    assert( s );
  }

  se_pgm = _pgm;
  se_args = _args;

  // read and apply default values ///////////////////////////////////////////
  resize(321, 321); // Dummy.
  QSize currentSize = size();
  if (currentSize != size())
     defaultSize = size();

  if(cfg.readEntry("EditEnabled","FALSE")=="TRUE") {
      configMenu->setItemChecked(-20,TRUE);
      commonCombo->setEditable( TRUE );
  } else {
      configMenu->setItemChecked(-20,FALSE);
      commonCombo->setEditable( FALSE );
  }
  configMenu->setItemEnabled(-20,!secondToolBar->isHidden());
}

void Konsole::show()
{
  if ( !nsessions ) {
    newSession();
  }
  QMainWindow::show();
}

void Konsole::initSession(const char*, QStrList &)
{
  QMainWindow::show();
}

Konsole::~Konsole()
{
    while (nsessions > 0) {
      doneSession(getTe()->currentSession, 0);
    }

  Config cfg("Konsole");
  cfg.setGroup("Konsole");
  cfg.writeEntry("FontID", cfont);
}

void Konsole::fontChanged(int f)
{
    VTFont* font = fonts.at(f);
    if (font != 0) {
        for(uint i = 0; i < fonts.count(); i++) {
            fontList->setItemChecked(i, (i == (uint) f) ? TRUE : FALSE);
        }

        cfont = f;

        TEWidget* te = getTe();
        if (te != 0) {
            te->setVTFont(font->getFont());
        }
    }
}


void Konsole::enterCommand(int c)
{
    TEWidget* te = getTe();
    if (te != 0) {
        if(!commonCombo->editable()) {
            QString text = commonCombo->text(c); //commonCmds[c];
            te->emitText(text);
        } else {
            changeCommand( commonCombo->text(c), c);            
        }
    }
}

void Konsole::hitEnter()
{
    TEWidget* te = getTe();
    if (te != 0) {
    te->emitText(QString("\r"));
    }
}

void Konsole::hitSpace()
{
    TEWidget* te = getTe();
    if (te != 0) {
    te->emitText(QString(" "));
    }
}

void Konsole::hitTab()
{
    TEWidget* te = getTe();
    if (te != 0) {
    te->emitText(QString("\t"));
    }
}

void Konsole::hitPaste()
{
    TEWidget* te = getTe();
    if (te != 0) {
    te->pasteClipboard();
    }
}

void Konsole::hitUp()
{
    TEWidget* te = getTe();
    if (te != 0) {
    QKeyEvent ke( QKeyEvent::KeyPress, Qt::Key_Up, 0, 0);
    QApplication::sendEvent( te, &ke );
    }
}

void Konsole::hitDown()
{
    TEWidget* te = getTe();
    if (te != 0) {
    QKeyEvent ke( QKeyEvent::KeyPress, Qt::Key_Down, 0, 0);
    QApplication::sendEvent( te, &ke );
    }
}

/**
   This function calculates the size of the external widget
   needed for the internal widget to be
 */
QSize Konsole::calcSize(int columns, int lines) {
    TEWidget* te = getTe();
    if (te != 0) {
    QSize size = te->calcSize(columns, lines);
    return size;
    } else {
  QSize size;
  return size;
    }
}

/**
    sets application window to a size based on columns X lines of the te
    guest widget. Call with (0,0) for setting default size.
*/

void Konsole::setColLin(int columns, int lines)
{
  if ((columns==0) || (lines==0))
  {
    if (defaultSize.isEmpty()) // not in config file : set default value
    {
      defaultSize = calcSize(80,24);
      // notifySize(24,80); // set menu items (strange arg order !)
    }
    resize(defaultSize);
  } else {
    resize(calcSize(columns, lines));
    // notifySize(lines,columns); // set menu items (strange arg order !)
  }
}

/*
void Konsole::setFont(int fontno)
{
  QFont f;
  if (fontno == 0)
    f = defaultFont = QFont( "Helvetica", 12  );
  else
  if (fonts[fontno][0] == '-')
    f.setRawName( fonts[fontno] );
  else
  {
    f.setFamily(fonts[fontno]);
    f.setRawMode( TRUE );
  }
  if ( !f.exactMatch() && fontno != 0)
  {
    QString msg = i18n("Font `%1' not found.\nCheck README.linux.console for help.").arg(fonts[fontno]);
    QMessageBox(this,  msg);
    return;
  }
  if (se) se->setFontNo(fontno);
  te->setVTFont(f);
  n_font = fontno;
}
*/

// --| color selection |-------------------------------------------------------

void Konsole::changeColumns(int columns)
{
  TEWidget* te = getTe();
  if (te != 0) {
  setColLin(columns,te->Lines());
  te->update();
  }
}

//FIXME: If a child dies during session swap,
//       this routine might be called before
//       session swap is completed.

void Konsole::doneSession(TESession*, int )
{
  TEWidget *te = getTe();
  if (te != 0) {
    te->currentSession->setConnect(FALSE);
    tab->removeTab(te);
    delete te->currentSession;
    delete te;
    nsessions--;
  }

  if (nsessions == 0) {
  close();
  }
}

void Konsole::newSession() {
    if(nsessions < 15) {    // seems to be something weird about 16 tabs on the Zaurus.... memory?
        TEWidget* te = new TEWidget(tab);
//  te->setBackgroundMode(PaletteBase); //we want transparent!!
    te->setVTFont(fonts.at(cfont)->getFont());
    tab->addTab(te);
    TESession* se = new TESession(this, te, se_pgm, se_args, "xterm");
    te->currentSession = se;
    connect( se, SIGNAL(done(TESession*,int)), this, SLOT(doneSession(TESession*,int)) );
    se->run();
    se->setConnect(TRUE);
    se->setHistory(b_scroll);
    tab->setCurrentPage(nsessions);
    nsessions++;
    setColor();
    }
}

TEWidget* Konsole::getTe() {
  if (nsessions) {
    return (TEWidget *) tab->currentPage();
  } else {
    return 0;
  }
}

void Konsole::switchSession(QWidget* w) {
  TEWidget* te = (TEWidget *) w;

  QFont teFnt = te->getVTFont();
  for(uint i = 0; i < fonts.count(); i++) {
    VTFont *fnt = fonts.at(i);
    bool cf = fnt->getFont() == teFnt;
    fontList->setItemChecked(i, cf);
    if (cf) {
      cfont = i;
    }
  }
}

void Konsole::colorMenuSelected(int iD)
{ // this is NOT pretty, elegant or anything else besides functional
//        QString temp;
//        temp.sprintf("%d", iD);
//      qDebug(temp);
    TEWidget* te = getTe();
    Config cfg("Konsole");
    cfg.setGroup("Colors");
    QColor foreground;
    QColor background;
    colorMenu->setItemChecked(lastSelectedMenu,FALSE);
    ColorEntry m_table[TABLE_COLORS];
    const ColorEntry * defaultCt=te->getdefaultColorTable();
      /////////// fore back
    int i;
    if(iD==-8) { // default default
        for (i = 0; i < TABLE_COLORS; i++)  {
            m_table[i].color = defaultCt[i].color;
            if(i==1 || i == 11)
                m_table[i].transparent=1;
            cfg.writeEntry("Schema","8");
            colorMenu->setItemChecked(-8,TRUE);
        }
    } else {
        if(iD==-5) { // green black
            foreground.setRgb(0x18,255,0x18);
            background.setRgb(0x00,0x00,0x00);
            cfg.writeEntry("Schema","5");
            colorMenu->setItemChecked(-5,TRUE);
        }
        if(iD==-6) { // black white
            foreground.setRgb(0x00,0x00,0x00);
            background.setRgb(0xFF,0xFF,0xFF);
            cfg.writeEntry("Schema","6");
            colorMenu->setItemChecked(-6,TRUE);
        }
        if(iD==-7) { // white black
            foreground.setRgb(0xFF,0xFF,0xFF);
            background.setRgb(0x00,0x00,0x00);
            cfg.writeEntry("Schema","7");
            colorMenu->setItemChecked(-7,TRUE);
        }
        if(iD==-9) {// Black, Red
            foreground.setRgb(0x00,0x00,0x00);
            background.setRgb(0xB2,0x18,0x18);
            cfg.writeEntry("Schema","9");
            colorMenu->setItemChecked(-9,TRUE);
        }
        if(iD==-10) {// Red, Black
            foreground.setRgb(230,31,31); //0xB2,0x18,0x18
            background.setRgb(0x00,0x00,0x00);
            cfg.writeEntry("Schema","10");
            colorMenu->setItemChecked(-10,TRUE);
        }
        if(iD==-11) {// Green, Yellow - is ugly
//            foreground.setRgb(0x18,0xB2,0x18);
            foreground.setRgb(36,139,10);
//            background.setRgb(0xB2,0x68,0x18);
            background.setRgb(255,255,0);
            cfg.writeEntry("Schema","11");
            colorMenu->setItemChecked(-11,TRUE);
        }
        if(iD==-12) {// Blue,  Magenta
//            foreground.setRgb(0x18,0xB2,0xB2);
//            background.setRgb(0x18,0x18,0xB2);
	    foreground.setRgb(0x18,0x18,0xB2);
            background.setRgb(0xB2,0x00,0xB2);
            cfg.writeEntry("Schema","12");
            colorMenu->setItemChecked(-12,TRUE);
        }
        if(iD==-13) {// Magenta, Blue
//            foreground.setRgb(0x18,0x18,0xB2);
//            background.setRgb(0x18,0xB2,0xB2);
            foreground.setRgb(0xB2,0x00,0xB2);
            background.setRgb(0x18,0x18,0xB2);
            cfg.writeEntry("Schema","13");
            colorMenu->setItemChecked(-13,TRUE);
        }
        if(iD==-14) {// Cyan,  White
            foreground.setRgb(0x18,0xB2,0xB2);
            background.setRgb(0xFF,0xFF,0xFF);
            cfg.writeEntry("Schema","14");
            colorMenu->setItemChecked(-14,TRUE);
        }
        if(iD==-15) {// White, Cyan
            background.setRgb(0x18,0xB2,0xB2);
            foreground.setRgb(0xFF,0xFF,0xFF);
            cfg.writeEntry("Schema","15");
            colorMenu->setItemChecked(-15,TRUE);
        }
        if(iD==-16) {// Black, Blue
            background.setRgb(0x00,0x00,0x00);
            foreground.setRgb(0x18,0xB2,0xB2);
            cfg.writeEntry("Schema","16");
            colorMenu->setItemChecked(-16,TRUE);
        }

        for (i = 0; i < TABLE_COLORS; i++)  {
            if(i==0 || i == 10) {
                m_table[i].color = foreground;
            }
            else if(i==1 || i == 11) {
                m_table[i].color = background; m_table[i].transparent=0;
            }
            else
                m_table[i].color = defaultCt[i].color;
        }
    }
    lastSelectedMenu = iD;
    te->setColorTable(m_table);
    update();
}

void Konsole::configMenuSelected(int iD)
{
//      QString temp;
//      temp.sprintf("%d",iD);
//      qDebug(temp);
    TEWidget* te = getTe();
    Config cfg("Konsole");
    cfg.setGroup("Menubar");
    if( iD  == -2) {
        if(!secondToolBar->isHidden()) {
            secondToolBar->hide();
            configMenu->changeItem( iD,tr("Show Command List"));
            cfg.writeEntry("Hidden","TRUE");
            configMenu->setItemEnabled(-20 ,FALSE);
        } else {
            secondToolBar->show();
            configMenu->changeItem( iD,tr("Hide Command List"));
            cfg.writeEntry("Hidden","FALSE");
            configMenu->setItemEnabled(-20 ,TRUE);
        }
    }
    if( iD  == -3) {
        cfg.setGroup("Tabs");
        QString tmp=cfg.readEntry("Position","Top");

        if(tmp=="Top") { // No tr
            tab->setTabPosition(QTabWidget::Bottom);
            configMenu->changeItem( iD,tr("Tabs on Top"));
            cfg.writeEntry("Position","Bottom");
        } else {
            tab->setTabPosition(QTabWidget::Top);
            configMenu->changeItem( iD,tr("Tabs on Bottom"));
            cfg.writeEntry("Position","Top");
        }
    }
    if( iD  == -20) {
        cfg.setGroup("Commands");
//        qDebug("enableCommandEdit");
        if( !configMenu->isItemChecked(iD) ) {
            commonCombo->setEditable( TRUE );
            configMenu->setItemChecked(iD,TRUE);
            commonCombo->setCurrentItem(0);
            cfg.writeEntry("EditEnabled","TRUE");
        } else {
            commonCombo->setEditable( FALSE );
            configMenu->setItemChecked(iD,FALSE);
            cfg.writeEntry("EditEnabled","FALSE");
            commonCombo->setFocusPolicy(QWidget::NoFocus);
            te->setFocus();
        }
    }
}

void Konsole::changeCommand(const QString &text, int c)
{
    Config cfg("Konsole");
    cfg.setGroup("Commands");
    if(commonCmds[c] != text) {
        cfg.writeEntry(QString::number(c),text);
        commonCombo->clearEdit();
        commonCombo->setCurrentItem(c);        
    }
}

void Konsole::setColor()
{
    Config cfg("Konsole");
    cfg.setGroup("Colors");
    int scheme = cfg.readNumEntry("Schema",1); 
    if(scheme != 1) colorMenuSelected( -scheme); 

}
