/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PHRASE_H
#define PHRASE_H

#include <QString>
#include <QList>

class Phrase
{
public:
    Phrase() { }
    Phrase(const QString &source, const QString &target,
	    const QString &definition, int sc = -1);

    QString source() const {return s;}
    void setSource(const QString &ns) {s = ns;}
    QString target() const {return t;}
    void setTarget(const QString &nt) {t = nt;}
    QString definition() const {return d;}
    void setDefinition (const QString &nd) {d = nd;}
    int shortcut() const {return shrtc;}

private:
    int shrtc;
    QString s;
    QString t;
    QString d;
};

bool operator==(const Phrase &p, const Phrase &q);
inline bool operator!=(const Phrase &p, const Phrase &q) {
    return !(p == q);
}

class PhraseBook : public QList<Phrase>
{
public:
    PhraseBook() { }

    bool load(const QString &filename);
    bool save(const QString &filename) const;
    QString fileName() const {return fn;}

    QString friendlyPhraseBookName() const;

private:
    QString fn;
};

#endif
