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

#ifndef CONTEXTKEYMANAGER_H
#define CONTEXTKEYMANAGER_H

#include <qmap.h>
#include <qvaluelist.h>
#include <qarray.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/contextbar.h>

class ContextKeyManager : public QObject
{
    Q_OBJECT
public:
    ContextKeyManager();

    void updateContextLabels();
    QWidget *findTargetWidget(QWidget *w, int key, bool modal);
    bool haveLabelForWidget(QWidget *w, int key, bool modal);

    void setContextText(QWidget *w, int id, const QString &t, ContextBar::EditingState state);
    void setContextPixmap(QWidget *w, int id, const QString &p, ContextBar::EditingState state);
    void setContextStandardLabel(QWidget *w, int id, ContextBar::StandardLabel label, ContextBar::EditingState state);
    void clearContextLabel(QWidget *w, int key, ContextBar::EditingState state);
    void setClassText(const QCString &className, int id, const QString &t, ContextBar::EditingState);
    void setClassPixmap(const QCString &className, int id, const QString &p, ContextBar::EditingState);
    void setClassStandardLabel(const QCString &className, int id, ContextBar::StandardLabel label, ContextBar::EditingState);

    struct ModalState {
	ModalState() : nmType(NoLabel), mType(NoLabel) {}

	enum Type { NoLabel, Custom, Standard };

	Type type(bool modal) const { return modal ? mType : nmType; }
	QString text(bool modal) const { return modal ? mText : nmText; }
	QString pixmap(bool modal) const { return modal ? mPixmap : nmPixmap; }
	ContextBar::StandardLabel label(bool modal) const { return modal ? mStandard : nmStandard; }
    
	QString nmText, mText;
	QString nmPixmap, mPixmap;
	ContextBar::StandardLabel nmStandard, mStandard; 
	Type nmType, mType;
    };

    typedef QMap<int,ModalState> KeyMap;

    struct ClassModalState {
	ClassModalState(const QCString &c)
	    : className(c) {}
	ClassModalState() {}

	bool operator==(const ClassModalState &c) const {
	    return className == c.className;
	}

	QCString className;
	KeyMap keyMap;
    };

    void setText(int key, const QString &text);
    void setPixmap(int key, const QString &pm);
    void setStandard(int key, ContextBar::StandardLabel label);
    void clearLabel(int key);

    static ContextKeyManager *instance();
    static QString standardText(ContextBar::StandardLabel label);
    static QString standardPixmap(ContextBar::StandardLabel label);

    const QArray<int> &keys() const { return buttons; }

private slots:
    void removeSenderFromWidgetContext();
    void updateLabelsForFocused();

private:
    bool updateContextLabel(QWidget *w, bool modal, int key);

private:
    QValueList<ClassModalState> contextClass;
    QMap<QWidget*,KeyMap> contextWidget;
    QArray<int> buttons;
    QTimer *timer;
};

#endif

