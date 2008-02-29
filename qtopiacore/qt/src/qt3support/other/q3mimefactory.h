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

#ifndef Q3MIMEFACTORY_H
#define Q3MIMEFACTORY_H

#include <QtGui/qwindowdefs.h>
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_MIMEFACTORY

class QStringList;
class QMimeSource;
class Q3MimeSourceFactoryData;

class Q_COMPAT_EXPORT Q3MimeSourceFactory {
public:
    Q3MimeSourceFactory();
    virtual ~Q3MimeSourceFactory();

    static Q3MimeSourceFactory* defaultFactory();
    static void setDefaultFactory(Q3MimeSourceFactory*);
    static Q3MimeSourceFactory* takeDefaultFactory();
    static void addFactory(Q3MimeSourceFactory *f);
    static void removeFactory(Q3MimeSourceFactory *f);

    virtual const QMimeSource* data(const QString& abs_name) const;
    virtual QString makeAbsolute(const QString& abs_or_rel_name, const QString& context) const;
    const QMimeSource* data(const QString& abs_or_rel_name, const QString& context) const;

    virtual void setText(const QString& abs_name, const QString& text);
    virtual void setImage(const QString& abs_name, const QImage& im);
    virtual void setPixmap(const QString& abs_name, const QPixmap& pm);
    virtual void setData(const QString& abs_name, QMimeSource* data);
    virtual void setFilePath(const QStringList&);
    inline  void setFilePath(const QString &path) { setFilePath(QStringList(path)); }
    virtual QStringList filePath() const;
    void addFilePath(const QString&);
    virtual void setExtensionType(const QString& ext, const char* mimetype);

private:
    QMimeSource *dataInternal(const QString& abs_name, const QMap<QString, QString> &extensions) const;
    Q3MimeSourceFactoryData* d;
};

Q_COMPAT_EXPORT QPixmap qPixmapFromMimeSource(const QString &abs_name);

Q_COMPAT_EXPORT QImage qImageFromMimeSource(const QString &abs_name);

#endif // QT_NO_MIMEFACTORY

QT_END_HEADER

#endif // Q3MIMEFACTORY_H
