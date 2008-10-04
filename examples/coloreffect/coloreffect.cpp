/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/
#include "coloreffect.h"

#include <QString>
#include <QVariant>
#include <QMap>
#include <QImage>
#include <QColor>

void ColorEffect::applyEffect(const QString &effect, const QMap<QString, QVariant> &parameters, QImage *image)
{
    if (effect == QLatin1String("BlackAndWhite")) {
        blackAndWhite(image);
    } else if (effect == QLatin1String("Tint")) {
        tone(
            qvariant_cast<QColor>(parameters.value(QLatin1String("Color"))),
            parameters.value(QLatin1String("Weight")).toInt(),
            image);
    }
}

void ColorEffect::blackAndWhite(QImage *image)
{
    for (int y = 0; y < image->height(); ++y) {
        for (int x = 0; x < image->width(); ++x) {
            int intensity = qGray(image->pixel(x, y));

            image->setPixel(x, y, qRgb(intensity, intensity, intensity));
        }
    }
}

inline static int blend(int toneColor, int imageColor, int intensity, int weight)
{
    return (toneColor * intensity * (100 - weight) + imageColor * weight * 255) / 25500;
}

void ColorEffect::tone(const QColor &color, int weight, QImage *image)
{
    int red = color.red();
    int green = color.green();
    int blue = color.blue();

    for (int y = 0; y < image->height(); ++y) {
        for (int x = 0; x < image->width(); ++x) {
            const QRgb rgb = image->pixel(x, y);
            const int intensity = qGray(rgb);

            image->setPixel(x, y, qRgb(
                blend(red, qRed(rgb), intensity, weight),
                blend(green, qGreen(rgb), intensity, weight),
                blend(blue, qBlue(rgb), intensity, weight)));
        }
    }
}


QTOPIA_EXPORT_PLUGIN(ColorEffect);
