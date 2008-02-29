/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "metatranslator.h"

#include <QMap>
#include <stdio.h>

typedef QMap<QByteArray, MetaTranslatorMessage> TMM;
typedef QList<MetaTranslatorMessage> TML;

/*
  Augments a MetaTranslator with trivially derived translations.

  For example, if "Enabled:" is consistendly translated as "Eingeschaltet:" no
  matter the context or the comment, "Eingeschaltet:" is added as the
  translation of any untranslated "Enabled:" text and is marked Unfinished.

  Returns the number of additional messages that this heuristic translated.
*/

int applySameTextHeuristic( MetaTranslator *tor )
{
    TMM translated;
    TMM avoid;
    TMM::Iterator t;
    TML untranslated;
    TML::Iterator u;
    TML all = tor->messages();
    TML::Iterator it;
    int inserted = 0;

    for ( it = all.begin(); it != all.end(); ++it ) {
        if ( (*it).type() == MetaTranslatorMessage::Unfinished ) {
            if ( !(*it).isTranslated() )
                untranslated.append( *it );
        } else {
            QByteArray key = (*it).sourceText();
            t = translated.find( key );
            if ( t != translated.end() ) {
                /*
                  The same source text is translated at least two
                  different ways. Do nothing then.
                */
                if ( (*t).translations() != (*it).translations() ) {
                    translated.remove( key );
                    avoid.insert( key, *it );
                }
            } else if ( !avoid.contains(key) && (*it).isTranslated() ) {
                translated.insert( key, *it );
            }
        }
    }

    for ( u = untranslated.begin(); u != untranslated.end(); ++u ) {
        QByteArray key = (*u).sourceText();
        t = translated.find( key );
        if ( t != translated.end() ) {
            MetaTranslatorMessage m( *u );
            m.setTranslations( (*t).translations() );
            tor->insert( m );
            inserted++;
        }
    }
    return inserted;
}
