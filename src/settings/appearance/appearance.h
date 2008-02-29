/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include <QDialog>
#include <QMap>
#include <QSoftMenuBar>
#include <QLabel>

class QWaitWidget;
class QComboBox;
class QCheckBox;
class QGroupBox;
class QPixmap;
class ThemedView;
class ThemeTextItem;
class ThemeImageItem;

class ThemePreview : public QObject
{
  Q_OBJECT

  public:
    ThemePreview(const QString &name, int width, int height);
    ~ThemePreview();
    void setXmlFilename(const QString &filename);
    void setButton(bool icon, ThemeTextItem *textItem, ThemeImageItem *imageItem, const QString &text, const QString &image);
    void reload();
    void setColor(const QString &color);
    void setIconLabel(bool enable);
    void requestPreview();

  signals:
    void previewReady(const QPixmap &);

  private slots:
    void doPreview();

  private:
    QString     m_name;
    ThemedView  *themedView;
    bool        m_button0Icon;
    bool        m_button1Icon;
    bool        m_button2Icon;
    int         m_width;
    int         m_height;
    QString     m_filename;
    QString     m_color;
    QPalette    m_palette;
    QPixmap     m_preview;
};

class ThemeInfo
{
public:
    ThemeInfo( const QString &tn);
    const QString & themeName();
    void setStyleName( const QString sn );
    const QString &styleName() const;
    void setThemeFileName( const QString fn );
    const QString &themeFileName() const;
    void setDecorationFileName( const QString fn );
    const QString &decorationFileName() const;
    void setServerWidget(const QString &);
    QString serverWidget() const;
    void setColorSchemes(const QStringList &);
    QStringList colorSchemes() const;
    void setBackgroundImages(const QStringList&);
    QStringList backgroundImages() const;

private:
    QString name;
    QString style;
    QString themeFile;
    QString decorationFile;
    QString srvWidget;
    QStringList colSchemes;
    QStringList bgImages;
    QString extendedFocusHighlight;
    QString formStyle;
    QString popupShadows;
    QString hideMenuIcons;
    QString fullWidthMenu;

friend class AppearanceSettings;
};

class AppearanceSettings : public QDialog
{
    Q_OBJECT

public:
    AppearanceSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AppearanceSettings();

protected:
    void accept();

protected slots:
    void populate();
    void themeSelected( int index );
    void colorSelected( int index );
    void backgroundSelected( const QString &text );
    void labelToggled( bool toggled );
    void receive( const QString& msg, const QByteArray& data );
    void pushSettingStatus();
    void setTitlePixmap(const QPixmap &);
    void setContextPixmap(const QPixmap &);
    void openHomescreenSettings();

private:
    void setupUi();
    void readThemeInfo();
    void readColorScheme();
    void applyStyle();
    void applyTheme();
    void applyColorScheme();
    void applyBackgroundImage();
    void applyLabel();
    QString status();
    void setStatus( const QString details );
    void pullSettingStatus();

private:
    QComboBox       *themeCombo;
    QComboBox       *colorCombo;
    QComboBox       *bgCombo;
    QCheckBox       *labelCkBox;
    QGroupBox       *groupBox;
    QLabel          titleLabel;
    QLabel          contextLabel;
    QLabel          backgroundLabel;
    ThemePreview    *titlePreview;
    ThemePreview    *contextPreview;

    QList<ThemeInfo> themes;
    QStringList colorListIDs;
    QString mServerWidgets;
    QString mActiveTheme, mActiveColor, mActiveBackground;
    int mActiveThemeIndex;
    QSoftMenuBar::LabelType mActiveLabelType;

    bool isStatusView, isFromActiveProfile;
};

#endif // APPEARANCESETTINGS_H
