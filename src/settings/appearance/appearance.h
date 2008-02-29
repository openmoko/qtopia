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
#ifndef APPEARANCESETTINGS_H
#define APPEARANCESETTINGS_H


#ifdef QTOPIA_PHONE
#include "ui_appearancesettingsbasephone.h"
#else
#include "ui_appearancesettingsbase.h"
#endif
#include <QDialog>
#include <QStringList>
#include <QSoftMenuBar>

class QWindowDecorationInterface;
class QPluginManager;
class SampleWindow;
class QListWidgetItem;
class QHBoxLayout;
class QWaitWidget;

class AppearanceSettings : public QDialog, Ui::AppearanceSettingsBase
{
    Q_OBJECT

public:
    AppearanceSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AppearanceSettings();

protected:
    void accept();
    void reject();
    void done(int r);
    void resizeEvent( QResizeEvent * );
    void showEvent( QShowEvent * );
#ifdef QTOPIA_PHONE
    bool eventFilter(QObject *o, QEvent *e);
#endif
    QPalette readColorScheme(int id);

protected slots:
    void applyStyle();
    void colorSelected( QListWidgetItem * );
    void styleSelected( QListWidgetItem * );
    void decorationSelected( QListWidgetItem * );
    QFont fontSelected( const QString &name );
    void fontSizeSelected( const QString &sz );
    void fixSampleGeometry();
    void selectImage();
    void selectWallpaper();
    void updateBackground();
    void clearBackground();
#ifdef QTOPIA_PHONE
    void pushSettingStatus();
    void receive( const QString& msg, const QByteArray& data );
    void tabChanged( int curIndex );
    void updateContextLabels();
#endif

private:
    void populateColorList();
#ifndef QTOPIA_PHONE
    void populateStyleList();
    void populateDecorationList();
#endif
    void populateFontList(const QString& cur, int cursz);
    void populate(const QString&, int);
#ifdef QTOPIA_PHONE
    void selectBackground(bool src_wallpaper);
    void populateThemeList( QString current );
    void populateLabelTypeList(const QSoftMenuBar::LabelType type);
    QString status();
    void setStatus( const QString details );
    void pullSettingStatus();
#endif
    void setStyle( QWidget *w, QStyle *s );
    bool isWide();
    bool displayMode();

private:
    QWindowDecorationInterface *wdiface;
    QPluginManager *wdLoader;
    bool wdIsPlugin;
    SampleWindow *sample;
    int prefFontSize;
    int maxFontSize;
    QStringList colorListIDs;
    QHBoxLayout *hBoxLayout;
    bool isClosing, rtl;
    QListWidgetItem *defaultColor;
    QString initColor;
#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
    QString bgImgName;
    int bgDisplayMode;
    QString initStatus;
    QAction *actionShowText;
    QString activeDetails;
    bool isThemeLoaded, isShowPreview, isStatusView, isFromActiveProfile;
    QListWidgetItem *defaultTheme;
    QWaitWidget *waitWidget;
#endif
    int currColor;
    int currTheme;
    bool bgChanged;
};


#endif // APPEARANCESETTINGS_H

