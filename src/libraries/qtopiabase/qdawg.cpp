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
#include "qdawg.h"
#include <QHash>
#include <qlist.h>
#include <qtextstream.h>
#include <qfile.h>
// #include <qtl.h>
#include <qiodevice.h>

#include <limits.h>
#include <stdio.h>

#include "qmemoryfile_p.h"

class QDawgPrivate;
class QTrie;

typedef QList<QTrie*> TrieClub;
typedef QHash<int, TrieClub *> TrieClubDirectory;

class TriePtr {
public:
    QChar letter;
    QTrie* p;
    int operator <(const TriePtr& o) const;
    int operator >(const TriePtr& o) const;
    int operator <=(const TriePtr& o) const;
};

class TrieList : public QList<TriePtr> {
    bool sorted;
public:
    TrieList()
    {
        sorted=true;
    }

    QTrie* findAdd(QChar c);
    bool equal(TrieList& l);

    void sort()
    {
        if ( !sorted ) {
            qSort(*this);
            sorted = true;
        }
    }
};

// A fast but memory-wasting temporary class.  The Dawg is the goal.
class QTrie {
public:
    QTrie();
    ~QTrie();

    void insertWord(const QString& s, uint index=0);
    bool equal(QTrie* o);
    void dump(int indent=0);

private:
    TrieList children;
    bool isword;

    friend class QDawgPrivate;
    int maxdepth;
    int decendants;
    int key;
    void distributeKeys(TrieClubDirectory& directory);
    QTrie* clubLeader(TrieClubDirectory& directory);
    int collectKeys();
    friend class TriePtr;
    friend class TrieList;
};

QTrie::QTrie()
{
    key = 0;
    isword = false;
}

QTrie::~QTrie()
{
    // NOTE: we do not delete the children - after conversion to DAWG
    // it's too difficult.  The QTrie's are deleted via the directory.
}

void QTrie::insertWord(const QString& s, uint index)
{
    if ( (int)index == s.length() ) {
        isword = true;
    } else {
        QTrie* t = children.findAdd(s[(int)index]);
        t->insertWord(s,index+1);
    }
}

bool QTrie::equal(QTrie* o)
{
    if ( o == this ) return true;
    if ( isword != o->isword )
        return false;
    return children.equal(o->children);
}

void QTrie::dump(int indent)
{
    for (TrieList::Iterator it=children.begin(); it!=children.end(); ++it) {
        QTrie* s = (*it).p;
        for (int in=0; in<indent; in++)
            fputc(' ',stderr);
        fprintf(stderr," %c %d %s %p\n",(*it).letter.unicode(),
            s->key,s->isword?"word":"",s); // No tr
        s->dump(indent+2);
    }
}

void QTrie::distributeKeys(TrieClubDirectory& directory)
{
    // Put every subtree in the directory, hashed to a key
    // that is based on the contents of the subtree. Identical
    // subtrees will have identical hashes.

    maxdepth = INT_MIN;
    decendants = children.count();
    key = 0;
    for (TrieList::Iterator it=children.begin(); it!=children.end(); ++it) {
        QTrie* s = (*it).p;
        QChar l = (*it).letter;
        s->distributeKeys(directory);
        key = key*64+l.unicode()+s->key*5;
        decendants += s->decendants;
        if ( s->maxdepth+1 > maxdepth )
            maxdepth = s->maxdepth+1;
    }
    if ( decendants ) {
        key += decendants + maxdepth*256 + children.count() * 65536;
        if ( !key ) key++; // unlikely
    }
    TrieClub *c = 0;
    if (!directory.contains(key)) {
        directory.insert(key, (c= new TrieClub));
    } else
        c = directory.value(key);
    c->prepend(this);
}

QTrie* QTrie::clubLeader(TrieClubDirectory& directory)
{
    // Return the canonical subtree of this subtree,
    // i.e. the first subtree in the directory that
    // is identical to this subtree.

    if ( !key ) return directory[0]->first();
    for (TrieList::Iterator itList=children.begin();
         itList!=children.end(); ++itList) {
        QTrie* t= (*itList).p->clubLeader(directory);
        (*itList).p = t;
    }
    TrieClub *club = directory[key];
    for (TrieClub::Iterator itClub = club->begin();
         itClub != club->end(); ++itClub) {
        QTrie* o = *itClub;
        if ( o->equal(this) )
            return o;
    }
    return this;
}

int QTrie::collectKeys()
{
    // Zero-out the stored keys, counting the number of children
    // as we go. Number of decendents is returned.

    int n=0;
    if ( key ) key=0,n+=children.count();
    for (TrieList::Iterator it=children.begin(); it!=children.end(); ++it)
        n += (*it).p->collectKeys();
    return n;
}

int TriePtr::operator <(const TriePtr& o) const
    { return letter < o.letter; }
int TriePtr::operator >(const TriePtr& o) const
    { return letter > o.letter; }
int TriePtr::operator <=(const TriePtr& o) const
    { return letter <= o.letter; }

bool TrieList::equal(TrieList& l)
{
    if ( count() != l.count() )
        return false;
    sort(); l.sort();
    ConstIterator it2 = begin();
    ConstIterator it = l.begin();
    for( ; it != l.end(); ++it, ++it2 )
        if ( (*it).letter != (*it2).letter || ! (*it).p->equal((*it2).p) )
            return false;
    return true;
}
QTrie* TrieList::findAdd(QChar c)
{
    for (Iterator it=begin(); it!=end(); ++it) {
        if ( (*it).letter == c )
            return (*it).p;
    }
    TriePtr p;
    p.p = new QTrie;
    p.letter = c;
    prepend(p);
    sorted=false;
    sort();
    return p.p;
}

static const char* dawg_sig32 = "QDAWG100"; //32 bit Node - old format (not supported anymore)
static const char* dawg_sig64 = "QDAWG200"; //64 bit Node

class QDawgPrivate {
public:
    QDawgPrivate(QIODevice* dev)
    {
        memoryFile = 0;
        QDataStream ds(dev);
        char sig[8];
        ds.readRawData(sig,8);
        if ( !strncmp(dawg_sig32,sig,8) ) {
            qWarning("Old QDawg format. Please generate a new QDawg file.");
            node = 0;
        } else if ( !strncmp(dawg_sig64,sig,8) ) {
            uint n;
            char* nn;
            ds.readBytes(nn,n);

            // #### endianness problem ignored.
            node = (QDawg::Node*)nn;
            nodes = n / sizeof(QDawg::Node);
        } else {
            node = 0;
        }
    }

    bool ok() const { return node; }

    ~QDawgPrivate()
    {
        delete memoryFile;
    }

private:
    QMemoryFile *memoryFile;

public:

    QDawgPrivate(const QString &fileName)
    {
        node = 0;
        nodes = 0;
        uchar* mem = 0;
        memoryFile = new QMemoryFile(fileName);
        if (memoryFile)
            mem = (uchar*)memoryFile->data();

        if (mem) {
            if (!strncmp(dawg_sig32, (char*)mem,8)) {
                qWarning("Old QDawg format. Please generate a new QDawg file.");
                node = 0;
            } else if (!strncmp(dawg_sig64, (char*)mem, 8)) {
                mem += 8; //skip signature
                int n = memoryFile->size() - 12;
                mem += 4; //skip file size

                // #### endianness problem ignored.
                node = (QDawg::Node*)((char*)mem);
                nodes = n / sizeof(QDawg::Node);
            }
        }
    }

    QDawgPrivate(QTrie* t) // destroys the QTrie.
    {
        memoryFile = 0;
        TrieClubDirectory directory;
        directory.reserve(9973);
        t->distributeKeys(directory);
        QTrie* l = t->clubLeader(directory);
        Q_ASSERT(l==t);
        Q_UNUSED(l);
        generateArray(t);

        foreach(TrieClub* club, directory) {
            for (TrieClub::Iterator it = club->begin(); it != club->end(); ++it) {
                delete *it;
            }
            delete club;
        }
    }

    bool write(QIODevice* dev)
    {
        QDataStream ds(dev);
        if (ds.writeRawData(dawg_sig64,8) != 8)
            return false;
        // #### endianness problem ignored.
        //always write new style
        ds.writeBytes((char*)node,sizeof(QDawg::Node)*nodes);
        return true;
    }

    void dumpWords(int nid=0, int index=0)
    {
        static char word[256]; // ick toLatin1
        int i=0;
        do {
            QDawg::Node& n = node[nid+i];
            word[index] = n.let;
            if ( n.isword )
                fprintf(stderr,"%.*s\n",index+1,word);
            if ( n.offset ) dumpWords(n.offset+nid+i,index+1);
        } while (!node[nid+i++].islast);
    }

    void dump(int nid=0, int indent=0)
    {
        int i=0;
        do {
            QDawg::Node& n = node[nid+i];
            fprintf(stderr,"%d: ",nid+i);
            for (int in=0; in<indent; in++)
                fputc(' ',stderr);
            fprintf(stderr," %c %d %d %d\n",n.let,
                n.isword,n.islast,n.offset);
            if ( n.offset ) dump(n.offset+nid+i,indent+2);
        } while (!node[nid+i++].islast);
    }

    int countWords(int nid=0)
    {
        int t=0;
        int i=0;
        do {
            QDawg::Node& n = node[nid+i];
            if ( n.isword )
                t++;
            if ( n.offset )
                t+=countWords(n.offset+nid+i);
        } while (!node[nid+i++].islast);
        return t;
    }

    bool contains(const QString& s, int nid=0, int index=0) const
    {
        int i=0;
        if ( index < (int)s.length() ) {
            do {
                QDawg::Node& n = node[nid+i];
                if ( s[index] == QChar((ushort)n.let) ) {
                    if ( n.isword && index == (int)s.length()-1 )
                        return true;
                    if ( n.offset )
                        return contains(s,n.offset+nid+i,index+1);
                }
            } while (!node[nid+i++].islast);
        }
        return false;
    }

    void appendAllWords(QStringList& list, int nid=0, QString s="") const
    {
        int i=0;
        int next = s.length();
        do {
            QDawg::Node& n = node[nid+i];
            s[next] = QChar((ushort)n.let);
            if ( n.isword )  {
                list.append(s);
            }
            if ( n.offset )
                appendAllWords(list, n.offset+nid+i, s);
        } while (!node[nid+i++].islast);
    }

    const QDawg::Node* root() { return node; }

private:
    void generateArray(QTrie* t)
    {
        nodes = 0;
        int n = t->collectKeys();
        node = new QDawg::Node[n];
        appendToArray(t);
        Q_ASSERT(n == nodes);
    }

    int appendToArray(QTrie* t)
    {
        if ( !t->key ) {
            if ( !t->children.count() )
                return 0;
            t->key = nodes;
            nodes += t->children.count();
            QDawg::Node* n = &node[t->key-1];
            int here = t->key;
            for (TrieList::Iterator it=t->children.begin(); it!=t->children.end(); ++it) {
                QTrie* s = (*it).p;
                ++n;
                n->let = (*it).letter.unicode();
                n->isword = s->isword;
                n->islast = 0;
                n->offset = appendToArray(s);
                if ( n->offset ) {
                    int t = n->offset-here;
                    n->offset=t;
                    if ( n->offset != t )
                        qWarning("Overflow: too many words");
                }
                here++;
            }
            n->islast = 1;
        }
        return t->key;
    }

private:
    int nodes;
    QDawg::Node *node;
};

/*!
  \class QDawg
  \mainclass
  \brief The QDawg class provides storage of words in a Directed Acyclic Word Graph.

  A DAWG provides very fast look-up of words in a word list given incomplete or
  ambiguous letters in those words.

  In Qtopia, global DAWGs are maintained for the current locale. See
  Qtopia::dawg(). Various Qtopia components use these DAWGs, most notably
  input methods, such as handwriting recognition that use the word lists
  to choose among the likely interpretations of the user input.

  To create your own a QDawg, construct an empty one with the default constructor,
  then add all the required words to it by a single call to createFromWords().

  Created QDawgs can be stored with write() and retrieved with
  read() or readFile().

  The data structure is such that adding words incrementally is not an efficient
  operation and can only be done by creating a new QDawg, using allWords() to get
  the existing words.

  The structure of a DAWG is a graph of \l {Node}{Nodes}, each representing a letter
  (retrieved by QDawg::Node::letter()). Paths through the graph
  represent partial or whole words. Nodes on paths that are whole words are flagged 
  as such by QDawg::Node::isWord(). Nodes are connected to a list of other nodes.
  The alphabetically first such node is retrieved by QDawg::Node::jump() and subsequent
  nodes are retrieved from the earlier by QDawg::Node::next(), with the last child
  returning 0 for that (and false for QDawg::Node::isLast()).

  There are no cycles in the graph as there are no inifinitely repeating words.

  For example, the DAWG below represents the word list:
  ban, band, can, cane, cans, pan, pane, pans.

  \image qdawg.png

  In the graph above, the \c root() node has the letter 'b', the \c root()->jump()
  node has the letter 'a', and the \c root()->next() node has the letter 'c'.
  Also, the root() node is not a word - \c !Node::isWord(), but \c root()->next()->jump()->jump()
  is a word (the word "can").

  This structuring not only provides O(1) look-up of words in the word list,
  but also produces a smaller compressed storage file than a plain text file word list.

  A simple algorithm that traverses the QDawg to see if a word is included would be:

  \code
    bool isWord(const QDawg::Node *n, const QString& s, int index)
    {
        int i=0;
        if ( index < (int)s.length() ) {
            while (n) {
                if ( s[index] == n->letter() ) {
                    if ( n->isWord() && index == (int)s.length()-1 )
                        return true;
                    return isWord(n->jump(),s,index+1);
                }
                n = n->next();
            }
        }
        return false;
    }
  \endcode

  In addition to simple look-up of a single word, the QDawg can be traversed to
  find lists of words with certain sets of characters, such as the characters associated
  with phone keys or handwriting. For example, given a QStringList where each string
  is a list of letter in decreasing order of likelihood, an efficient algorithm could
  be written for finding the best word by traversing the QDawg just once.

  \ingroup userinput
*/

/*!
    Constructs a new empty QDawg. The next step is usually to add all words
    with createFromWords() or use readFile() to retrieve existing words.
*/
QDawg::QDawg()
{
    d = 0;
}

/*!
    Destroys the QDawg. If it was attached to a file with readFile(), it is detached.
*/
QDawg::~QDawg()
{
    delete d;
}

/*!
    \overload
    Replaces all the words in the QDawg with words read by QIODevice::readLine() from \a dev.
*/
bool QDawg::createFromWords(QIODevice* dev)
{
    delete d;

    QTextStream i(dev);
    QTrie* trie = new QTrie;
    int n=0;
    while (!i.atEnd()) {
        trie->insertWord(i.readLine());
        n++;
    }
    if ( n )
        d = new QDawgPrivate(trie);
    else
        d = 0;
    return true;
}

/*!
  Replaces all the words in the QDawg with the words in the \a list.
*/
void QDawg::createFromWords(const QStringList& list)
{
    delete d;

    if ( list.count() ) {
        QTrie* trie = new QTrie;
        for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
            QString word = (*it).toUtf8();
            //trie->insertWord(word);
            trie->insertWord(*it);
        }
        d = new QDawgPrivate(trie);
    } else {
        d = 0;
    }
}

/*!
  Returns a list of all the words in the QDawg, in alphabetical order.
*/
QStringList QDawg::allWords() const
{
    QStringList result;
    if ( d ) d->appendAllWords(result);
    return result;
}


/*!
  Replaces all the words in the QDawg with the QDawg in \a filename.
  Note that the file is memory-mapped if possible.

  Returns true if successful. If not successful, the QDawg is left unchanged and false is returned.

  \sa write()
*/
bool QDawg::readFile(const QString& filename)
{
    delete d;

    d = new QDawgPrivate(filename);
    if (!d->ok()) {
        // failed to load file into memory
        delete d;
        d = 0;
    }
    return d;
}

/*!
  Replaces all the words in the QDawg with the QDawg read from \a dev.
  The file is \i not memory-mapped. Use readFile() wherever possible, for better performance.

  Returns true if successful. If not successful, the QDawg is left unchanged and false is returned.

  \sa write()
*/
bool QDawg::read(QIODevice* dev)
{
    delete d;
    d = new QDawgPrivate(dev);
    if ( d->ok() )
        return true;
    delete d;
    d = 0;
    return false;
}

/*!
  Writes the QDawg to \a dev, in a custom QDAWG format.

  Returns true if successful.

  \warning QDawg memory maps QDAWG files.
  The safe method for writing to QDAWG files is to
  write the data to a new file and move the new
  file to the old file name. QDawg objects using the old
  file will continue using that file.
*/
bool QDawg::write(QIODevice* dev) const
{
    return d ? d->write(dev) : true;
}

/*!
  Iterates over the whole graph and returns the total number of words found in the QDawg.
*/
int QDawg::countWords() const
{
    return d ? d->countWords() : 0;
}

/*!
  Returns the root \l {Node}{Node} of the QDawg, or 0 if the QDawg is empty.

  The root is the starting point for all traversals.

  Note that this root node has a Node::letter(),
  and subsequent nodes returned by Node::next(), just like any other Node.
*/
const QDawg::Node* QDawg::root() const
{
    return d ? d->root() : 0;
}

/*!
  Returns true if the QDawg contains the word \a s; otherwise returns
  false.
*/
bool QDawg::contains(const QString& s) const
{
    return d ? d->contains(s) : false;
}

/*!
  \internal

  For debugging: prints out the QDawg contents.
*/
void QDawg::dump() const
{
    if ( d ) d->dump();
}

/*!
  \class QDawg::Node
  \mainclass
  \brief The Node class of the QDawg class is one node of a QDawg.

  Each Node in a QDawg represents a letter().
  Paths through the QDawg graph
  represent partial or whole words. Nodes on paths that are whole words are flagged 
  as such by isWord(). Nodes are connected to a list of other nodes.
  The alphabetically first such node is retrieved by jump() and subsequent
  nodes are retrieved from the earlier by next(), with the last child
  returning 0 for that (and false for isLast()).

  See the QDawg documentation for usage of this class.
*/

/*!
  \fn QChar QDawg::Node::letter() const

  Returns this Node's letter.
*/
/*!
  \fn bool QDawg::Node::isWord() const

  Returns true if this Node (including its letter()) is the end of a word;
  otherwise returns false.
*/
/*!
  \fn bool QDawg::Node::isLast() const

  Returns true if this Node is the last in the child list;
  otherwise returns false.
*/
/*!
  \fn const Node* QDawg::Node::next() const

  Returns the next "child" Node in the child list or 0 if this Node
  isLast().

  \sa jump()
*/
/*!
  \fn const Node* QDawg::Node::jump() const

  Returns the node connected to this Node, which is the first "child"
  Node.

  \sa next()
*/
