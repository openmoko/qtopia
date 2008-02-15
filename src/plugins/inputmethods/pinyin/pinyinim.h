/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PINYINIM_H
#define PINYINIM_H
#include <qwindowsystem_qws.h>
#include <inputmethodinterface.h>
#include <qtopia/inputmatch/wordpicker.h>
#include <qtopia/inputmatch/symbolpicker.h>
#include <QDawg>
#include <QStack>

class PinyinMatch
{
public:
    PinyinMatch(const QString &path, const QDawg::Node * node);
    ~PinyinMatch();

    QString path() const { return currentpath; }
    const QDawg::Node * node() const { return currentnode; }

private:
    QString currentpath;                //letters used to get to the current node (including letter of current node)
    const QDawg::Node * currentnode;    //position in the dawg
};


class PinyinIM : public QWSInputMethod
{
    Q_OBJECT
public:
    PinyinIM();
    ~PinyinIM();

    void reset();
    using QWSInputMethod::filter; // Don't hide these symbols!
    bool filter(int unicode, int keycode, int modifiers, 
			    bool isPress, bool autoRepeat);

    void setActive(bool);
    bool active() const { return isActive; }
    
    void setMicroFocus( int x, int y );
    virtual void queryResponse( int property, const QVariant & result );

private slots:
    void pinyinSelected(const QString &s);
    void charSelected(const QString &s);
    void symbolSelected(int unicode, int keycode);
    
private:
    bool loadPy2Uni();
    void processDigitKey(int keycode);
    
    bool isActive;
    int microX;
    int microY;
    
    QHash<int,QString> key2let;
    QDawg pinyinDawg;                   //dawg of all possible pinyin syllables
    const QDawg::Node * rootNode;       //root of pinyinDawg
    QMultiHash<QString,QChar> py2uni;
    QStack< QList<PinyinMatch> > matchHistory;
    QPointer<WordPicker> pinyinPicker;
    QPointer<WordPicker> charPicker;
    QPointer<SymbolPicker> symbolPicker;
};

#endif
