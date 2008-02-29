/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef STYLEDBUTTON_H
#define STYLEDBUTTON_H

#include <QtGui/QPushButton>

namespace qdesigner_internal {

class StyledButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
    Q_ENUMS(ButtonType)

public:
    enum ButtonType {ColorButton, PixmapButton};

    StyledButton (QWidget *parent = 0, ButtonType type = ColorButton);
    virtual ~StyledButton () {}

    void setButtonType(ButtonType type);
    const QBrush &brush();
    void setBrush(const QBrush &b);

    QString pixmapFileName() const;

signals:
    void changed();

public slots:
    virtual void onEditor();

protected:
    void paintEvent (QPaintEvent *event);

private:
    QString buildImageFormatList() const;
    bool openPixmap();

    ButtonType btype;
    QString pixFile;
    QBrush mBrush;
};

}  // namespace qdesigner_internal

#endif // STYLEDBUTTON_H
