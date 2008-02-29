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

#ifndef PROPARSER_H
#define PROPARSER_H

#include "profileevaluator.h"

// Subclass it to intercept the logMessage method
class ProFileTranslationsScanner : public ProFileEvaluator {
public:
    ProFileTranslationsScanner(bool verbose) : ProFileEvaluator() 
    {
        m_verbose = verbose;
    }

    ProFile *queryProFile(const QString &filename) {
        return ProFileEvaluator::queryProFile(filename);
    }

private:
    /* reimp */
    void logMessage(const LogMessage &msg) {
        if (m_verbose && (msg.m_type == ProFileEvaluator::MT_DebugLevel1
            || msg.m_type == ProFileEvaluator::MT_Error)) {
            ProFileEvaluator::logMessage(msg);
        }
    }

private:
    bool m_verbose;
};

#endif

void removeDuplicates(QStringList *strings, bool alreadySorted = true);

bool evaluateProFile(const QString &fileName, bool verbose, QMap<QByteArray, QStringList> *varMap);
