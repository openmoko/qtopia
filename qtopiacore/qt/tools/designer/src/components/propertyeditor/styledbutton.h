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
