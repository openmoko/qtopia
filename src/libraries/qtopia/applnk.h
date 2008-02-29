/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __APPLNK_H__
#define __APPLNK_H__

#include <qtopia/qpeglobal.h>
#include <qobject.h>
#include <qiconset.h>
#include <qlist.h>
#include <qdict.h>
#include <qstringlist.h>

class AppLnkSetPrivate;
class AppLnkPrivate;

class QTOPIA_EXPORT AppLnk
{
public:
    AppLnk();
    AppLnk( const QString &file );
    AppLnk( const AppLnk &copy ); // copy constructor
    virtual ~AppLnk();

    bool isValid() const { return !mLinkFile.isNull(); }

    static void setSmallIconSize(int);
    static void setBigIconSize(int);
    static int smallIconSize();
    static int bigIconSize();

    QString name() const { return mName; }
    const QPixmap& pixmap() const;
    const QPixmap& bigPixmap() const;
    QString icon() const;   // libqtopia
    virtual QString exec() const { return mExec; }
    QString type() const;
    QString rotation() const { return mRotation; }
    QString comment() const { return mComment; }
    QString file() const;
    QString linkFile() const;
    QStringList mimeTypes() const { return mMimeTypes; }
    QStringList mimeTypeIcons() const { return mMimeTypeIcons; }
    const QArray<int> &categories() const;
    int id() const { return mId; }

    bool fileKnown() const;	// libqtopia
    bool linkFileKnown() const;	// libqtopia

    void execute() const;
    void execute(const QStringList& args) const;
    void removeFiles();
    void removeLinkFile();

    void setName( const QString& docname );
#ifdef Q_WS_QWS
    void setExec( const QString& exec );
#endif
    void setFile( const QString& filename );
    void setLinkFile( const QString& filename );
    void setComment( const QString& comment );
    void setType( const QString& mimetype );
    void setIcon( const QString& iconname );
    void setCategories( const QArray<int> &v );
    bool writeLink() const;

    void setProperty(const QString& key, const QString& value);
    QString property(const QString& key) const;

    bool setLocation( const QString& docPath ); // libqtopia

    bool isDocLnk() const; // libqtopia
    
#ifdef QTOPIA_INTERNAL_PRELOADACCESS
// MOC_SKIP_BEGIN
    bool isPreloaded() const;
    void setPreloaded(bool yesNo);
// MOC_SKIP_END
#endif

    AppLnk &operator=(const AppLnk &other); // libqtopia

protected:
    QString mName;
    QPixmap mPixmap;
    QPixmap mBigPixmap;
    QString mExec;
    QString mType;
    QString mRotation;
    QString mComment;
    QString mFile;
    QString mLinkFile;
    QString mIconFile;
    QStringList mMimeTypes;
    QStringList mMimeTypeIcons;
    int mId;
    static int lastId;
    AppLnkPrivate *d;
    friend class AppLnkSet;
#ifdef Q_WS_QWS
    virtual void invoke(const QStringList& args) const;
#endif
    bool ensureLinkExists() const;

private:
    void storeLink() const;
};

#define QTOPIA_DEFINED_APPLNK
#include <qtopia/qtopiawinexport.h>

class QTOPIA_EXPORT DocLnk : public AppLnk
{
public:
    DocLnk();
    DocLnk( const DocLnk &o ) : AppLnk(o) { }
    DocLnk( const QString &file );
    DocLnk( const QString &file, bool may_be_desktopfile );
    virtual ~DocLnk();


    DocLnk &operator=(const DocLnk &other); // libqtopia

    QString exec() const;

protected:
    void invoke(const QStringList& args) const;

private:
    void init(const QString &file);
};

#define QTOPIA_DEFINED_DOCLNK
#include <qtopia/qtopiawinexport.h>

class QTOPIA_EXPORT AppLnkSet
{
public:
    AppLnkSet();
    AppLnkSet( const QString &dir );
    ~AppLnkSet();

    const AppLnk *find( int id ) const;
    const AppLnk *findExec( const QString& execname ) const;

    QStringList types() const { return typs; }
    QString typeName( const QString& ) const;
    QPixmap typePixmap( const QString& ) const;
    QPixmap typeBigPixmap( const QString& ) const;

    void add(AppLnk*);
    bool remove(AppLnk*);
    void clear(); // libqtopia

    const QList<AppLnk> &children() const { return mApps; }
    void detachChildren();

protected:
    friend class AppLnk;
    QList<AppLnk> mApps;
    QString mFile;
    QStringList typs;
    AppLnkSetPrivate *d;

private:
    AppLnkSet( const AppLnkSet & ); // no copying!
    void findChildren(const QString &, const QString& t, const QString& lt, int depth = 0);
};

#define QTOPIA_DEFINED_APPLNKSET
#include <qtopia/qtopiawinexport.h>

class QTOPIA_EXPORT DocLnkSet : public AppLnkSet
{
public:
    DocLnkSet();
    DocLnkSet( const QString &dir, const QString &mimefilter=QString::null );

    const QList<DocLnk> &children() const { return (const QList<DocLnk> &)mApps; }

    void appendFrom( DocLnkSet& other );

private:
    DocLnkSet( const DocLnkSet & ); // no copying!
    void findChildren(const QString &dr, const QValueList<QRegExp> &mimeFilters, QDict<void> &reference, int depth=0);
};

#define QTOPIA_DEFINED_DOCLNKSET
#include <qtopia/qtopiawinexport.h>

#endif // __APPLNK_H__

