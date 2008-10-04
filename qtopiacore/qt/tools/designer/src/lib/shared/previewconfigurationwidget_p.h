/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef PREVIEWCONFIGURATIONWIDGET_H
#define PREVIEWCONFIGURATIONWIDGET_H

#include "shared_global_p.h"

#include <QtGui/QGroupBox>
#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QSettings;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PreviewConfiguration;
class PreviewConfigurationWidgetStateData;

// ----------- PreviewConfigurationWidgetState: Additional state that goes
//             to QSettings besides the actual PreviewConfiguration (enabled flag and list of user deviceSkins).

class QDESIGNER_SHARED_EXPORT PreviewConfigurationWidgetState {
public:
    PreviewConfigurationWidgetState();
    PreviewConfigurationWidgetState(const QStringList &userDeviceSkins, bool enabled);

    PreviewConfigurationWidgetState(const PreviewConfigurationWidgetState&);
    PreviewConfigurationWidgetState& operator=(const PreviewConfigurationWidgetState&);
    ~PreviewConfigurationWidgetState();

    bool isEnabled() const;
    void setEnabled(bool e);

    QStringList userDeviceSkins() const;
    void setUserDeviceSkins(const QStringList &u);

    void clear();
    void toSettings(const QString &prefix, QSettings &settings) const;
    void fromSettings(const QString &prefix, const QSettings &settings);

    // Returns the PreviewConfiguration according to the enabled flag.
    PreviewConfiguration previewConfiguration(const PreviewConfiguration &serializedConfiguration) const;

private:
    QSharedDataPointer<PreviewConfigurationWidgetStateData> m_d;
};

// ----------- PreviewConfigurationWidget: Widget to edit the preview configuration.

class QDESIGNER_SHARED_EXPORT PreviewConfigurationWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit PreviewConfigurationWidget(QWidget *parent = 0);
    virtual ~PreviewConfigurationWidget();

    void setCore(QDesignerFormEditorInterface *core);

    PreviewConfigurationWidgetState previewConfigurationWidgetState() const;
    void setPreviewConfigurationWidgetState(const PreviewConfigurationWidgetState &pc);

    // Note: These accessors are for serialization only; to determine
    // the actual PreviewConfiguration, the enabled flag of PreviewConfigurationWidgetState has to be observed.
    PreviewConfiguration previewConfiguration() const;
    void setPreviewConfiguration(const PreviewConfiguration &pc);

private slots:
    void slotEditAppStyleSheet();
    void slotDeleteSkinEntry();
    void slotSkinChanged(int);

private:
    class PreviewConfigurationWidgetPrivate;
    PreviewConfigurationWidgetPrivate *m_impl;

    PreviewConfigurationWidget(const PreviewConfigurationWidget &other);
    PreviewConfigurationWidget &operator =(const PreviewConfigurationWidget &other);
};
}

QT_END_NAMESPACE

#endif // PREVIEWCONFIGURATIONWIDGET_H
