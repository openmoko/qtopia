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

#ifndef _QIMPENINPUT_H_
#define _QIMPENINPUT_H_

#include <qtopia/mstroke/profile.h>

#include <qtopia/qdawg.h>

#include <qframe.h>
#include <qlist.h>
#include <qguardedptr.h>

class QPushButton;
class QTimer;
class QIMPenWidget;
class QIMPenWordPick;
class QIMPenMatch;
class HandwritingHelp;

class QIMPenInput : public QFrame
{
    Q_OBJECT
public:
    QIMPenInput( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~QIMPenInput();

    void resetState();

    QSize sizeHint() const;

signals:
    void key( ushort, ushort, ushort, bool, bool );
    
private slots:
    void wordPicked( const QString & );
    void selectCharSet( int );
    void beginStroke();
    void strokeEntered( QIMPenStroke *st );
    void matchedCharacters( const QIMPenCharMatchList &cl );
    void keypress( uint scan_uni );
    void erase();
    void help();
    void setup();
    void backspace();
    void enter();

    void pluginMessage(const QCString &, const QByteArray &);

private:
    void loadProfiles();
    void selectProfile( const QString &name );
    void handleExtended( const QString & );
    void updateWordMatch( QIMPenCharMatchList &ml );
    void matchWords();
    void scanDict( const QDawg::Node* n, int ipos, const QString& str, int error );

    enum Mode { Normal, Switch, SwitchLock };

private:
    Mode mode;
    QRect prefRect;
    QIMPenWidget *pw;
    QPushButton *helpBtn;
    QPushButton *setupBtn;
    QIMPenMatch *matcher;
    QGuardedPtr<HandwritingHelp> helpDlg;
    QIMPenProfile *profile;
    QList<QIMPenProfile> profileList;
    QIMPenCharSet *shortcutCharSet;
    QIMPenCharSetList baseSets;
    int currCharSet;
    QIMPenWordPick *wordPicker;
};

#endif // _QIMPENINPUT_H_
