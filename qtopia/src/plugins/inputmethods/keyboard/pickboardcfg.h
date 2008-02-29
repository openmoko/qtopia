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
#ifndef PICKBOARDCFG_H
#define PICKBOARDCFG_H

#include <qtopia/qdawg.h>

#include <qpushbutton.h>
#include <qhbuttongroup.h>
#include <qdialog.h>
#include <qlist.h>


// Internal stuff...

class PickboardPicks;

class LetterButton : public QPushButton {
    Q_OBJECT
public:
    LetterButton(const QChar& letter, QWidget* parent);
private slots:
    void toggleCase();
private:
    bool skip;
};

class LetterChoice : public QButtonGroup {
    Q_OBJECT
public:
    LetterChoice(QWidget* parent, const QString& set);

    QChar choice() { return ch; }

signals:
    void changed();

private slots:
    void change();

private:
    QChar ch;
};

class PickboardAdd : public QDialog {
    Q_OBJECT
public:
    PickboardAdd(QWidget* owner, const QStringList& setlist);
    ~PickboardAdd();

    QString word() const;
    bool exec();

private slots:
    void checkAllDone();

private:
    QPushButton *yes;
    LetterChoice **lc;
    int nlc;
};

class PickboardConfig : public QObject {
    Q_OBJECT
public:
    PickboardConfig(PickboardPicks* p) : parent(p), nrows(2), pressx(-1) { }
    virtual ~PickboardConfig();
    virtual void pickPoint(const QPoint& p, bool press);
    virtual void draw(QPainter*)=0;
    virtual void fillMenu(QPopupMenu&);
    virtual void doMenu(int);

protected:
    void updateRows(int from, int to);
    virtual void updateItem(int r, int i);
    virtual void pickInRow(int r, int xpos, bool press)=0;

    void changeMode(int m);
    virtual void generateText(const QString& s);
    void generateKey( int k );

    virtual void pick(bool press, int row, int item)=0;

protected:
    PickboardPicks* parent;
    int nrows;
private:
    int pressrow, pressx;
};

class StringConfig : public PickboardConfig {
    Q_OBJECT
public:
    StringConfig(PickboardPicks* p) : PickboardConfig(p) { }

    void draw(QPainter* p);

protected:
    virtual QString text(int r, int i)=0;
    virtual bool spreadRow(int i)=0;
    virtual QColor rowColor(int) { return Qt::black; }
    virtual void pickInRow(int r, int xpos, bool press);
    virtual void updateItem(int r, int i);
    virtual bool highlight(int,int) const;
};

class CharStringConfig : public StringConfig {
    Q_OBJECT
    QString input;
    QStringList chars;
public:
    CharStringConfig(PickboardPicks* p) : StringConfig(p) { }

    void addChar(const QString& s);
    virtual void doMenu(int);

protected:
    QString text(int r, int i);
    bool spreadRow(int i);
    void pick(bool press, int row, int item);
};

class DictFilterConfig : public StringConfig {
    Q_OBJECT
    QStringList matches;
    QStringList sets_a;
    QStringList sets;
    QStringList othermodes;
    int lit0;
    int lit1;
    int shift;
    QString capitalize(const QString& s);
    QStringList capitalize(const QStringList& s);

public:
    QStringList input;
    DictFilterConfig(PickboardPicks* p) : StringConfig(p)
    {
	shift = 0;
	lit0 = -1;
	lit1 = -1;
    }

    void addSet(const QString& appearance, const QString& set);
    void addMode(const QString& s);

    void fillMenu(QPopupMenu& menu);
    void doMenu(int i);
    
    void add(const QString& set);

protected:
    QString text(int r, int i);

    bool spreadRow(int i);

    void pick(bool press, int row, int item);

    bool scanMatch(const QString& set, const QChar& l) const;
    void scan(const QDawg::Node* n, int ipos, const QString& str, int length, bool extend);
    void scanLengths(const QDawg::Node* n, int ipos, int& bitarray);

    bool highlight(int r,int c) const;
};

class CharConfig : public StringConfig {
    Q_OBJECT
    QStringList chars1;
    QStringList chars2;
public:
    CharConfig(PickboardPicks* p) : StringConfig(p) { }
    void addChar(int r, const QString& s);

protected:
    QString text(int r, int i);
    bool spreadRow(int);
    void pick(bool press, int row, int item);
};

class KeycodeConfig : public PickboardConfig {
    Q_OBJECT
    QValueList<int> keys1;
    QValueList<int> keys2;
    QValueList<QPixmap> keypm1;
    QValueList<QPixmap> keypm2;
    enum { xw = 8, xmarg = 8};

public:
    KeycodeConfig(PickboardPicks* p) : PickboardConfig(p) { }
    void addKey(int r, const QPixmap& pm, int code);
    void addGap(int r, int w);

    void draw(QPainter* p);

protected:
    void pickInRow(int r, int xpos, bool press);
    QValueList<QPixmap> row(int i);

    void pick(bool press, int row, int item);
};


#endif
