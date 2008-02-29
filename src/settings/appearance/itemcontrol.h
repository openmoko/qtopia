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

#include <QThread>
#include <QListWidgetItem>
#include <QWindowDecorationInterface>
#include <QContent>

#include "samplewindow.h"

class QListWidget;
class QLabel;
class QPushButton;
class QPluginManager;

class PluginItem : public QListWidgetItem
{
public:
    PluginItem( const QString &t, QListWidget *lb );
    void setFilename( const QString fn );
    const QString &filename() const;
    void setThemeFilename( const QString fn );
    const QString &themeFilename() const;
    void setDecorationFilename( const QString fn );
    const QString &decorationFilename() const;

private:
    QString file;
    QString themeFile;
    QString decorationFile;
};

class DefaultWindowDecoration : public QWindowDecorationInterface
{
public:
    DefaultWindowDecoration();
    QString name() const;
    QPixmap icon() const;
};

class AppearanceItemControl : public QThread
{
    Q_OBJECT

public:
    AppearanceItemControl(QWidget *view);
    ~AppearanceItemControl();

    enum ApplyItemType { DefaultItem, ActiveItem, CurrentItem };

    virtual void run();

    virtual void populate();
    virtual void apply(ApplyItemType item = CurrentItem);

    void setSampleWindow(SampleWindow*);

protected slots:
    virtual void currentItemChanged(QListWidgetItem *);

protected:
    QWidget *mView;
    QListWidget *mList;
    QListWidgetItem *mDefaultItem;
    QListWidgetItem *mActiveItem;
    QListWidgetItem *mLastAppliedItem;
    bool mApplied;
#ifndef QTOPIA_PHONE
    SampleWindow *mSample;
#endif
};

class ColorAppearanceItemControl : public AppearanceItemControl
{
    Q_OBJECT

public:
    ColorAppearanceItemControl(QWidget *view);
    ~ColorAppearanceItemControl();

protected:
    virtual void populate();
    virtual void apply(ApplyItemType item);

private:
    QPalette readColorScheme(int id);

protected slots:
    virtual void currentItemChanged(QListWidgetItem *);

private:
    QStringList colorListIDs;
};

#ifdef QTOPIA_PHONE
class ThemeAppearanceItemControl : public AppearanceItemControl
{
    Q_OBJECT

public:
    ThemeAppearanceItemControl(QWidget *view);
    ~ThemeAppearanceItemControl();

protected:
    virtual void populate();
    virtual void apply(ApplyItemType item);
};

class LabelAppearanceItemControl : public AppearanceItemControl
{
    Q_OBJECT

public:
    LabelAppearanceItemControl(QWidget *view);
    ~LabelAppearanceItemControl();

protected:
    virtual void populate();
    virtual void apply(ApplyItemType item);
};

class BackgroundAppearanceItemControl : public AppearanceItemControl
{
    Q_OBJECT

public:
    BackgroundAppearanceItemControl(QWidget *view);
    ~BackgroundAppearanceItemControl();

    void updatePreview();

protected:
    virtual void populate();
    virtual void apply(ApplyItemType item);

    void selectBackground(bool src_wallpaper);

protected slots:
    void selectImage();
    void selectWallpaper();
    void clearBackground();

private:
    bool displayMode();
    void checkFileSystem(const QContent);
    void copyImage(QContent);

public:
    QLabel *mPreviewLabel;
    QPushButton *mMyPictures;
    QPushButton *mWallpaper;
    QPushButton *mClearButton;
    QString bgImgName;
    int bgDisplayMode;

private:
    QString activeImgName;
};

#else

class FontAppearanceItemControl : public AppearanceItemControl
{
    Q_OBJECT

public:
    FontAppearanceItemControl(QWidget *view);
    ~FontAppearanceItemControl();

protected:
    virtual void populate();
    virtual void apply(ApplyItemType item);

private slots:
    QFont fontSelected( const QString &name );
    void fontSizeSelected( const QString &sz );

private:
    int prefFontSize;
    int maxFontSize;
};

class StyleAppearanceItemControl : public AppearanceItemControl
{
    Q_OBJECT

public:
    StyleAppearanceItemControl(QWidget *view);
    ~StyleAppearanceItemControl();

protected:
    virtual void populate();
    virtual void apply(ApplyItemType item);

protected slots:
    virtual void currentItemChanged(QListWidgetItem *);

private:
    void setStyle( QWidget *w, QStyle *s );
    bool isWide();

private slots:
    void styleSelected( QListWidgetItem * );
};

class DecorationAppearanceItemControl : public AppearanceItemControl
{
    Q_OBJECT

public:
    DecorationAppearanceItemControl(QWidget *view);
    ~DecorationAppearanceItemControl();

protected:
    virtual void populate();
    virtual void apply(ApplyItemType item);

private slots:
    virtual void currentItemChanged(QListWidgetItem *);

private:
    QWindowDecorationInterface *wdiface;
    QPluginManager *wdLoader;
    bool wdIsPlugin;
};
#endif
