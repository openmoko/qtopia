/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef __UNI_SELECT__
#define __UNI_SELECT__

#include <qwidget.h>
#include <qmap.h>
#include <qpoint.h>
#include <qstring.h>

class QComboBox;
class CharacterView;

class UniSelect : public QWidget
{
    Q_OBJECT
    friend class CharacterView;
public:
    UniSelect(QWidget *parent, const char *name = 0, Qt::WFlags f = 0);
    ~UniSelect();

    uint character() const;
    QString text() const;

    // uint must be > 0xffff, name must be already translated.
    void addSpecial(uint, const QString &);
    void clearSpecials();
signals:
    void selected(uint);
    void selected(const QString &);

private:
    QComboBox *mSetSelect;
    CharacterView *mGlyphSelect;
};

#endif
