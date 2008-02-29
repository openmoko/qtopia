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

#ifndef QIMPENPROFILE_H_
#define QIMPENPROFILE_H_

#include "char.h"

#include <qtopiaglobal.h>

class QTOPIAHW_EXPORT QIMPenProfile
{
public:
    explicit QIMPenProfile( const QString &fn );
    ~QIMPenProfile();

    const QString name() const { return pname; }
    const QString identifier() const;
    const QString description() const { return pdesc; }

    bool canSelectStyle() const { return tstyle; }
    bool canIgnoreStroke() const { return istyle; }

    enum Style { ToggleCases, BothCases };

    Style style() const { return pstyle; }
    void setStyle( Style s );

    int multiStrokeTimeout() const { return msTimeout; }
    void setMultiStrokeTimeout( int t );

    int ignoreStrokeTimeout() const { return isTimeout; }
    void setIgnoreStrokeTimeout( int t );

    bool matchWords() const { return wordMatch; }

    // shouldn't use, overly restricts usage of set
    // returns first char set of this type.
    QIMPenCharSet *uppercase();
    QIMPenCharSet *lowercase();
    QIMPenCharSet *numeric();
    QIMPenCharSet *punctuation();
    QIMPenCharSet *symbol();
    QIMPenCharSet *shortcut();
    QIMPenCharSet *find( QIMPenCharSet::Type t );

    // this is more generic, and translateable.
    QIMPenCharSet *charSet( const QString & ); // internal (not translated)
    QString title( const QString & ); // translated
    QString description( const QString & ); // translated

    QStringList charSets(); // internal (not translated)

    void save() const;
private:
    void load();
    QString userConfig() const;
    void loadData();
    void saveData() const;

private:
    struct ProfileSet {
        ProfileSet() : set(0) {}
        ProfileSet(QIMPenCharSet *s) : set(s) {}
        ~ProfileSet() { if (set) delete set; }

        QString id;
        QIMPenCharSet *set;
    };

    typedef QList<ProfileSet *> ProfileSetList;
    typedef QList<ProfileSet *>::iterator ProfileSetListIterator;
    typedef QList<ProfileSet *>::const_iterator ProfileSetListConstIterator;
    ProfileSetList sets;

    QString filename;
    QString pname;
    QString pdesc;
    Style pstyle;
    bool tstyle;
    bool istyle;
    int msTimeout;
    int isTimeout;
    bool wordMatch;
};

#endif
