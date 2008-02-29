/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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
#include <longstring.h>
#include <qstring.h>
#include <qfile.h>
#include <limits.h> // for CHAR_BIT
#include <QFileInfo>

// mmap
#include <unistd.h>
#include <sys/mman.h>

// LongString: A string/array class for processing IETF documents such as
// RFC(2)822 messages in a memory efficient method.

// The LongString string/array class implemented in this file provides 2
// primary benefits over the QString/QByteArray string/array classes namely:
//
// 1) Inbuilt support for mmap'ing a file so that a string can parsed without
//    requiring all bytes of that string to be loaded into physical memory.
//
// 2) left, mid and right methods that don't create deep copies of the
//    string data, this is achieved by using ref counting and
//    QByteArray::fromRawData
//
// Normal QByteArray methods can be used on LongStrings by utilizing the
// LongString::toQByteArray method.
//
// Known Limitations:
//
// 1) The internal representation is 8bit ascii, which is fine for 7bit ascii
//    email messages.
//
// 2) The underlying data is treated as read only.
//
// 3) mmap may not be supported on non *nix platforms.
//
// Additionally LongString provides a case insensitive indexOf method, this
// is useful as email fields/tokens (From, Date, Subject etc) are case
// insensitive.

class LongStringPrivate
{
public:
    LongStringPrivate(const QString &fileName, bool autoRemove );
    ~LongStringPrivate();
    QByteArray toQByteArray() const;
    void ref();
    void deref();
    QString fileName() const;

private:
    int mRefCount;
    size_t mSize;
    char* mBuffer;
    int mPageMod;
    QString mFileName;
    bool mAutoRemove;
};

LongStringPrivate::LongStringPrivate(const QString &fileName, bool autoRemove)
    :mRefCount(1), mSize( 0 ), mBuffer( 0 ), mPageMod( 0 )
{
  mFileName = fileName;
  mAutoRemove = autoRemove;
  QFileInfo fi( fileName );
  static int pagesize = -1;
  if (pagesize < 0)
      pagesize = getpagesize();

  if (fi.exists() && fi.isFile() && fi.isWritable() && fi.size() > 0)
  {
    FILE* stream = fopen(QFile::encodeName( fileName ), "r+" );
    int tell = ftell( stream );
    mPageMod = tell % pagesize;
    mSize = fi.size();
    mBuffer = (char *)mmap(0, fi.size() + mPageMod, PROT_READ, MAP_SHARED, fileno(stream), tell - mPageMod) + mPageMod;
    ++mPageMod;
    if (mBuffer == MAP_FAILED) {
        mBuffer = 0;
        mSize = 0;
        mPageMod = 0;
    }
  }
}

LongStringPrivate::~LongStringPrivate()
{
    if (mPageMod) {
        --mPageMod;
        munmap(mBuffer - mPageMod, mSize + mPageMod);

        if (mAutoRemove) {
            QFile tempFile( mFileName );
            tempFile.remove();
        }
    }
}

QByteArray LongStringPrivate::toQByteArray() const
{
    if (!mBuffer)
        return QByteArray();

    QByteArray ba( QByteArray::fromRawData( mBuffer, mSize ) );
    return ba;
}

QString LongStringPrivate::fileName() const
{
    return mFileName;
}

void LongStringPrivate::ref()
{
    ++mRefCount;
}

void LongStringPrivate::deref()
{
    --mRefCount;
    if (!mRefCount)
        delete this;
}

LongString::LongString()
  :d( 0 )
{
}

LongString::LongString(const LongString &s)
{
    mBa = s.mBa;
    mPrimaryBa = s.mPrimaryBa;
    d = s.d;
    if (d)
        d->ref();
}

LongString::LongString(const QByteArray &ba)
  :d( 0 )
{
    mBa = ba;
    mPrimaryBa = ba;
}

LongString::LongString(const QByteArray &ba, const QByteArray &pBa, LongStringPrivate *otherD)
{
    mBa = ba;
    mPrimaryBa = pBa;
    d = otherD;
    if (d)
        d->ref();
}

LongString::LongString(const QString &fileName, bool autoRemove )
  :d( 0 )
{
  d = new LongStringPrivate( fileName, autoRemove );
  mBa = d->toQByteArray();
  mPrimaryBa = mBa;
}

LongString::~LongString()
{
    if (d)
        d->deref();
    // Reference to mPrimaryBa is released
}

LongString &LongString::operator=(const LongString &other)
{
    if (this == &other)
        return *this;
    mBa = other.mBa;
    mPrimaryBa = other.mPrimaryBa;
    if (d)
        d->deref();
    d = other.d;
    if (d)
        d->ref();
    return *this;
}

int LongString::length() const
{
    return mBa.length();
}

bool LongString::isEmpty() const
{
    return mBa.isEmpty();
}

#define REHASH(a) \
    if (ol_minus_1 < sizeof(uint) * CHAR_BIT) \
        hashHaystack -= (a) << ol_minus_1; \
    hashHaystack <<= 1

int LongString::indexOf(const QByteArray &ba, int from) const
{
    // Based on QByteArray::indexOf, except use strncasecmp for
    // case-insensitive string comparison
    const int l = mBa.length();
    while ((from < 0) && (l > 0))
        from += l;

    const int ol = ba.size();
    if (from > l || ol + from > l) {
        return -1;
    }
    if (ol == 0) {
        return from;
    }
    if (ol == 1) {
        return mBa.indexOf(*ba.data(), from);
    }
/*
    Haven't reimplemented this code as it's doubtful that
    this indexof method is a bottleneck.

    if (l > 500 && ol > 5)
        return QByteArrayMatcher(ba).indexIn(*this, from);
*/
    const char *needle = ba.data();
    const char *haystack = mBa.data() + from;
    const char *end = mBa.data() + (l - ol);
    const uint ol_minus_1 = ol - 1;
    uint hashNeedle = 0, hashHaystack = 0;
    int idx;
    for (idx = 0; idx < ol; ++idx) {
        hashNeedle = ((hashNeedle<<1) + needle[idx]);
        hashHaystack = ((hashHaystack<<1) + haystack[idx]);
    }
    hashHaystack -= *(haystack + ol_minus_1);

    while (haystack <= end) {
        hashHaystack += *(haystack + ol_minus_1);
        if (hashHaystack == hashNeedle  && *needle == *haystack
             && strncasecmp(needle, haystack, ol) == 0)
        {
            return haystack - mBa.data();
        }
        REHASH(*haystack);
        ++haystack;
    }
    return -1;
}

const LongString LongString::mid(int i, int len) const
{
    if (len < 0)
        len = length() - i;

    QByteArray ba( QByteArray::fromRawData( mBa.constData() + i, len ) );
    return LongString( ba, mPrimaryBa, d );
}

const LongString LongString::left(int len) const
{
    if (len < 0)
        len = mBa.length(); // QString compatibility

    QByteArray ba( QByteArray::fromRawData( mBa.constData(), len ) );
    return LongString( ba, mPrimaryBa, d );
}

const LongString LongString::right(int len) const
{
    if (len < 0)
        len = mBa.length(); // QString compatibility

    QByteArray ba( QByteArray::fromRawData( mBa.constData() + mBa.length() - len, len ) );
    return LongString( ba, mPrimaryBa, d );
}

QString LongString::toQString() const
{
    return mBa;
}

const QByteArray LongString::toQByteArray() const
{
    return mBa;
}

QString LongString::fileName() const
{
    if (d)
        return d->fileName();

    return QString::null;
}

