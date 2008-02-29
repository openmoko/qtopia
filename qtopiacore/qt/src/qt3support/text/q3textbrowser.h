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

#ifndef Q3TEXTBROWSER_H
#define Q3TEXTBROWSER_H

#include <QtGui/qpixmap.h>
#include <QtGui/qcolor.h>
#include <Qt3Support/q3textedit.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_TEXTBROWSER

class Q3TextBrowserData;

class Q_COMPAT_EXPORT Q3TextBrowser : public Q3TextEdit
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource)

    friend class Q3TextEdit;

public:
    Q3TextBrowser(QWidget* parent=0, const char* name=0);
    ~Q3TextBrowser();

    QString source() const;

public Q_SLOTS:
    virtual void setSource(const QString& name);
    virtual void backward();
    virtual void forward();
    virtual void home();
    virtual void reload();
    void setText(const QString &txt) { setText(txt, QString()); }
    virtual void setText(const QString &txt, const QString &context);

Q_SIGNALS:
    void backwardAvailable(bool);
    void forwardAvailable(bool);
    void sourceChanged(const QString&);
    void highlighted(const QString&);
    void linkClicked(const QString&);
    void anchorClicked(const QString&, const QString&);

protected:
    void keyPressEvent(QKeyEvent * e);

private:
    Q_DISABLE_COPY(Q3TextBrowser)

    void popupDetail(const QString& contents, const QPoint& pos);
    bool linksEnabled() const { return true; }
    void emitHighlighted(const QString &s);
    void emitLinkClicked(const QString &s);
    Q3TextBrowserData *d;
};

#endif // QT_NO_TEXTBROWSER

QT_END_HEADER

#endif // Q3TEXTBROWSER_H
