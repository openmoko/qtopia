/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QHELPSEARCHINDEXREADERCLUCENE_H
#define QHELPSEARCHINDEXREADERCLUCENE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include "qhelpsearchengine.h"
#include "fulltextsearch/qquery_p.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

QT_BEGIN_NAMESPACE

namespace qt {
    namespace fulltextsearch {
        namespace clucene {

class QHelpSearchIndexReader : public QThread
{
    Q_OBJECT

public:
    QHelpSearchIndexReader();
    ~QHelpSearchIndexReader();

    void cancelSearching();
    void search(const QString &collectionFile,
        const QString &indexFilesFolder,
        const QList<QHelpSearchQuery> &queryList);

    int hitsCount() const;
    QHelpSearchEngine::SearchHit hit(int index) const;

signals:
    void searchingStarted();
    void searchingFinished(int hits);

private:
    void run();
    bool defaultQuery(const QString &term,
        QCLuceneBooleanQuery &booleanQuery);
    bool buildQuery(QCLuceneBooleanQuery &booleanQuery,
        const QList<QHelpSearchQuery> &queryList);
    
private:
    QMutex mutex;
    QList<QHelpSearchEngine::SearchHit> hitList;
    QWaitCondition waitCondition;

    bool m_cancel;
    QString m_collectionFile;
    QList<QHelpSearchQuery> m_query;
    QString m_indexFilesFolder;
};

        }   // namespace clucene
    }   // namespace fulltextsearch
}   // namespace qt

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXREADERCLUCENE_H
