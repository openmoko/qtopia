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

/*
TRANSLATOR qdesigner_internal::StyledButton
*/

#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/QColorDialog>
#include <QtGui/QFileDialog>

#include "styledbutton.h"

using namespace qdesigner_internal;

StyledButton::StyledButton (QWidget *parent, ButtonType type)
    : QPushButton(parent), btype(type)
{
    connect(this, SIGNAL(clicked()), this, SLOT(onEditor()));
    mBrush = QBrush(Qt::darkGray);
}

const QBrush &StyledButton::brush()
{
    return mBrush;
}

void StyledButton::setBrush(const QBrush &b)
{
    mBrush = b;

    if (btype == PixmapButton)
        mBrush.setColor(Qt::darkGray);

    update();
}

void StyledButton::setButtonType(ButtonType type)
{
    btype = type;
    update();
}

void StyledButton::paintEvent (QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QStyleOptionButton opt;
    opt.init(this);
    QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);

    QPainter paint(this);

    if (btype == ColorButton)
        paint.setBrush(QBrush(mBrush.color()));
    else
        paint.setBrush(mBrush);

    paint.drawRect(contentRect.left()+2, contentRect.top()+2, contentRect.width()-5, contentRect.height()-5);
}

QString StyledButton::buildImageFormatList() const
{
    QString filter;

#if 0 // ### port me
    QString all = tr("All Pixmaps (");
    for (int i=0; i<QImageIO::outputFormats().count(); ++i) {
        QString outputFormat = QImageIO::outputFormats().at(i);
        QString outputExtension;

        if (outputFormat != "JPEG")
            outputExtension = outputFormat.toLower();
        else
            outputExtension = "jpg;*.jpeg";

        filter += tr("%1-Pixmaps (%2)\n").arg(outputFormat).arg("*." + outputExtension);
        all += "*." + outputExtension + ";";
    }
    filter.prepend(all + tr(")\n"));
#endif

    filter += tr("All Files (*.*)");

    return filter;
}

bool StyledButton::openPixmap()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(), buildImageFormatList());

    if (!fileName.isEmpty()) {
        pixFile = fileName;
        return true;
    }

    return false;
}

QString StyledButton::pixmapFileName() const
{
    return pixFile;
}

void StyledButton::onEditor()
{
    if (btype == ColorButton) {
        QColor c = QColorDialog::getColor(mBrush.color(), this);
        if (c.isValid()) {
            mBrush.setColor(c);
            emit changed();
        }
    }
    else if(openPixmap()) {
        emit changed();
    }
}
