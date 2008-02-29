/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QDIR_H
#define QDIR_H

#ifndef QT_H
#include "qglobal.h"
#include "qstrlist.h"
#include "qfileinfo.h"
#endif // QT_H


#ifndef QT_NO_DIR
typedef QPtrList<QFileInfo> QFileInfoList;
typedef QPtrListIterator<QFileInfo> QFileInfoListIterator;
class QStringList;
template <class T> class QDeepCopy;


class Q_EXPORT QDir
{
public:
    enum FilterSpec { Dirs	    = 0x001,
		      Files	    = 0x002,
		      Drives	    = 0x004,
		      NoSymLinks    = 0x008,
		      All	    = 0x007,
		      TypeMask	    = 0x00F,

		      Readable	    = 0x010,
		      Writable	    = 0x020,
		      Executable    = 0x040,
		      RWEMask	    = 0x070,

		      Modified	    = 0x080,
		      Hidden	    = 0x100,
		      System	    = 0x200,
		      AccessMask    = 0x3F0,

		      DefaultFilter = -1 };

    enum SortSpec   { Name	    = 0x00,
		      Time	    = 0x01,
		      Size	    = 0x02,
		      Unsorted	    = 0x03,
		      SortByMask    = 0x03,

		      DirsFirst	    = 0x04,
		      Reversed	    = 0x08,
		      IgnoreCase    = 0x10,
		      DefaultSort   = -1 };

    QDir();
    QDir( const QString &path, const QString &nameFilter = QString::null,
	  int sortSpec = Name | IgnoreCase, int filterSpec = All );
    QDir( const QDir & );

    virtual ~QDir();

    QDir       &operator=( const QDir & );
    QDir       &operator=( const QString &path );

    virtual void setPath( const QString &path );
    virtual QString path()		const;
    virtual QString absPath()	const;
    virtual QString canonicalPath()	const;

    virtual QString dirName() const;
    virtual QString filePath( const QString &fileName,
			      bool acceptAbsPath = TRUE ) const;
    virtual QString absFilePath( const QString &fileName,
				 bool acceptAbsPath = TRUE ) const;

    static QString convertSeparators( const QString &pathName );

    virtual bool cd( const QString &dirName, bool acceptAbsPath = TRUE );
    virtual bool cdUp();

    QString	nameFilter() const;
    virtual void setNameFilter( const QString &nameFilter );
    FilterSpec filter() const;
    virtual void setFilter( int filterSpec );
    SortSpec sorting() const;
    virtual void setSorting( int sortSpec );

    bool	matchAllDirs() const;
    virtual void setMatchAllDirs( bool );

    uint count() const;
    QString	operator[]( int ) const;

    virtual QStrList encodedEntryList( int filterSpec = DefaultFilter,
				       int sortSpec   = DefaultSort  ) const;
    virtual QStrList encodedEntryList( const QString &nameFilter,
				       int filterSpec = DefaultFilter,
				       int sortSpec   = DefaultSort   ) const;
    virtual QStringList entryList( int filterSpec = DefaultFilter,
				   int sortSpec   = DefaultSort  ) const;
    virtual QStringList entryList( const QString &nameFilter,
				   int filterSpec = DefaultFilter,
				   int sortSpec   = DefaultSort   ) const;

    virtual const QFileInfoList *entryInfoList( int filterSpec = DefaultFilter,
						int sortSpec = DefaultSort ) const;
    virtual const QFileInfoList *entryInfoList( const QString &nameFilter,
						int filterSpec = DefaultFilter,
						int sortSpec = DefaultSort ) const;

    static const QFileInfoList *drives();

    virtual bool mkdir( const QString &dirName,
			bool acceptAbsPath = TRUE ) const;
    virtual bool rmdir( const QString &dirName,
			bool acceptAbsPath = TRUE ) const;

    virtual bool isReadable() const;
    virtual bool exists()   const;
    virtual bool isRoot()   const;

    virtual bool isRelative() const;
    virtual void convertToAbs();

    virtual bool operator==( const QDir & ) const;
    virtual bool operator!=( const QDir & ) const;

    virtual bool remove( const QString &fileName,
			 bool acceptAbsPath = TRUE );
    virtual bool rename( const QString &name, const QString &newName,
			 bool acceptAbsPaths = TRUE  );
    virtual bool exists( const QString &name,
			 bool acceptAbsPath = TRUE );

    static char separator();

    static bool setCurrent( const QString &path );
    static QDir current();
    static QDir home();
    static QDir root();
    static QString currentDirPath();
    static QString homeDirPath();
    static QString rootDirPath();

    static bool match( const QStringList &filters, const QString &fileName );
    static bool match( const QString &filter, const QString &fileName );
    static QString cleanDirPath( const QString &dirPath );
    static bool isRelativePath( const QString &path );
    void refresh() const;

private:
#ifdef Q_OS_MAC
    typedef struct FSSpec FSSpec;
    static FSSpec *make_spec(const QString &);
#endif
    void init();
    virtual bool readDirEntries( const QString &nameFilter,
				 int FilterSpec, int SortSpec  );

    static void slashify( QString & );

    QString	dPath;
    QStringList   *fList;
    QFileInfoList *fiList;
    QString	nameFilt;
    FilterSpec	filtS;
    SortSpec	sortS;
    uint	dirty	: 1;
    uint	allDirs : 1;

    void detach();
    friend class QDeepCopy< QDir >;
};


inline QString QDir::path() const
{
    return dPath;
}

inline QString QDir::nameFilter() const
{
    return nameFilt;
}

inline QDir::FilterSpec QDir::filter() const
{
    return filtS;
}

inline QDir::SortSpec QDir::sorting() const
{
    return sortS;
}

inline bool QDir::matchAllDirs() const
{
    return allDirs;
}

inline bool QDir::operator!=( const QDir &d ) const
{
    return !(*this == d);
}


struct QDirSortItem {
    QString filename_cache;
    QFileInfo* item;
};

#endif // QT_NO_DIR
#endif // QDIR_H
