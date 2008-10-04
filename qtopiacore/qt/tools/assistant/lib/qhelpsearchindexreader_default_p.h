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

#ifndef QHELPSEARCHINDEXREADERDEFAULT_H
#define QHELPSEARCHINDEXREADERDEFAULT_H

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

#include "qhelpsearchindex_default_p.h"
#include "qhelpsearchengine.h"

#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QByteArray>
#include <QtCore/QStringList>
#include <QtCore/QWaitCondition>

QT_BEGIN_NAMESPACE

struct Entry;
struct PosEntry;

namespace qt {
    namespace fulltextsearch {
        namespace std {

class Reader
{
    typedef QList<QStringList> DocumentList;
    typedef QHash<QString, Entry*> EntryTable;
    typedef QPair<EntryTable, DocumentList> Index;
    typedef QHash<QString, Index> IndexTable;

public:
    Reader();
    ~Reader();

    bool readIndex();
    bool initCheck() const;
    void setIndexPath(const QString &path);
    void filterFilesForAttributes(const QStringList &attributes);
    void setIndexFile(const QString &namespaceName, const QString &attributes);
    bool splitSearchTerm(const QString &searchTerm, QStringList *terms, 
        QStringList *termSeq, QStringList *seqWords);
    
    void searchInIndex(const QStringList &terms);
    QVector<DocumentInfo> hits();
    bool searchForPattern(const QStringList &patterns, 
        const QStringList &words, const QByteArray &data);

private:
    QVector<Document> setupDummyTerm(const QStringList &terms, const EntryTable &entryTable);
    QStringList getWildcardTerms(const QString &term, const EntryTable &entryTable);
    void buildMiniIndex(const QString &string);
    void reset();
    void cleanupIndex(EntryTable &entryTable);

private:
    uint wordNum;
    QString indexPath;
    QString indexFile;
    QString documentFile;

    IndexTable indexTable;
    QList<TermInfo> termList;
    IndexTable searchIndexTable;
    QHash<QString, PosEntry*> miniIndex;
};


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

private:
    QMutex mutex;
    Reader m_reader;
    QWaitCondition waitCondition;
    QList<QHelpSearchEngine::SearchHit> hitList;

    bool m_cancel;
    QList<QHelpSearchQuery> m_query;
    QString m_collectionFile;
    QString m_indexFilesFolder;
};

        }   // namespace std
    }   // namespace fulltextsearch
}   // namespace qt

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXREADERDEFAULT_H
