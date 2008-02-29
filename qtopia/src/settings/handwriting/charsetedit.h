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

#include <qlist.h>
#include <qdialog.h>
#include <qtopia/mstroke/profile.h>

#include "charseteditbase.h"

class QTabWidget;
class QListBox;
class QPushButton;
class QComboBox;
class QIMPenWidget;
class QIMPenEdit;
class QIMPenPrefBase;
class UniSelect;

class QIMPenInputCharDlg : public QDialog
{
    Q_OBJECT
public:
    QIMPenInputCharDlg( QWidget *parent = 0, const char *name = 0,
	    bool modal = FALSE, bool fs = FALSE, int WFlags = 0 );

    unsigned int unicode() const { return uni; }

protected:

protected slots:
    void setCharacter( uint sp );

private:
    void addSpecial(bool isFS );
    uint uni;
    UniSelect *u;
};

class CharSetEdit : public CharSetEditBase
{
    Q_OBJECT
public:
    CharSetEdit( QWidget *parent=0, const char *name=0 );
    CharSetEdit( QIMPenCharSet *c, QWidget *parent=0,
		const char *name=0 );

    void setCharSet( QIMPenCharSet *c );

    QIMPenCharSet *charSet() const;

    void checkStoreMatch();

    void setIsFS(bool b) {
	mIsFS=b;
    }
protected:
    void fillCharList();
    void enableButtons();

    QIMPenChar *findPrev();
    QIMPenChar *findNext();

    void setCurrentChar( QIMPenChar * );

protected slots:
    void prevMatch();
    void nextMatch();

    void selectCode( int );

    void addChar();
    void removeChar();

    void addMatch();
    void removeMatch();
    void resetMatches();
    void clearMatch();

    void newStroke( QIMPenStroke * );

protected:

    void appendMatch();
    void init();
    void updateLabel();
    void firstMatch();
    void lastMatch();

    // current code, not nearly detailed enough eh?
    uint currentCode;
    QIMPenChar *currentChar;
    QIMPenChar *inputChar;
    QIMPenCharSet *currentSet;
    int lastCs;
    int lastCh;
    bool addFlag;
    uint matchCount;
    uint matchIndex;
    bool mIsFS;
};

class CharSetDlg : public QDialog
{
    Q_OBJECT
public:
    CharSetDlg( QWidget *parent = 0, const char *name = 0,
	    bool modal = FALSE, int WFlags = 0 );
    ~CharSetDlg();

    void setCharSet( QIMPenCharSet *c );

    QIMPenCharSet *charSet() const;

    void setIsFS(bool b) {
	edit->setIsFS(b);
    }
protected:
    void accept();
    void reject();

private:
    CharSetEdit *edit;
};

