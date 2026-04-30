/*
 * Instabeam - 2D structural analysis with FEM (curved beams, trusses, CST)
 * Copyright (C) 2018-2019 bernhard1536 (original TrussCalculator code)
 * Copyright (C) 2020-2026 Wolfgang Flachberger (Instabeam additions)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "utilities.h"

#include <QPainter>

bool Utilities::isequal(double a, double b)
{
    return std::isnan(a) || std::isnan(b) ? false : !std::islessgreater(a, b);
}

QRectF Utilities::max(const QRectF &rect1, const QRectF &rect2)
{
    double maxWidth = fmax(rect1.width(), rect2.width());
    double maxHeight = fmax(rect1.height(), rect2.height());
    return {- maxWidth / 2, - maxHeight / 2, maxWidth, maxHeight};
}

QString Utilities::convertAddressToString(const void * const address, int base)
{
    if (base < 2 || base > 36) {
        qDebug() << "error in Utilities::convertAddressToString() occured";
    }
    return QString::number(reinterpret_cast<std::uintptr_t>(address), base); // cast address (contains the address of the obj pointed to by address) to unsigned int and convert it
}

void Utilities::drawArrow(QPainter *painter, double arrowWidth, double arrowHeight)
{
    QPainterPath p(QPointF(0, 0));
    p.lineTo(arrowHeight, - arrowWidth / 2);
    p.lineTo(p.currentPosition().x(), arrowWidth / 2);
    p.lineTo(0, 0);
    painter->save();
    painter->fillPath(p, QBrush(painter->pen().color()));
    painter->drawPath(p);
    painter->restore();
}

double Utilities::arctan(double dy, double dx)
{
    double angle = atan2(dy, dx) * 180 / M_PI; // calculate arcus tangens of dy/dx and convert it to degrees
    return fmod(angle + 360, 360); // because the atan2-fct returns the value in the interval [-pi/2; +pi/2] add a full circle and then truncate the value back to be between 0° and 360°
}

double Utilities::setAlmostZeroToZero(double value)
{
    return value > -1e-12 && value < 1e-12 ? 0 : value;
}
