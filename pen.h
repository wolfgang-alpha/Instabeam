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

#ifndef PEN_H
#define PEN_H

#include "color.h"

#include <QPen>

class Pen final : public QPen
{
public:
    explicit Pen(const Color &color = Color(), qreal width = 1, Qt::PenCapStyle cap = Qt::RoundCap);
    // QPen has no virtual destructor, therefore be careful when deleting ptrs to Pen via ptrs to QPen (it is safest to use no ptrs to Pen at all)!!!

    Color getColor() const { return c; }

    Pen differentColor(const Color &otherColor); // return a copy of this pen with a different color
    Pen differentWidth(int otherWidth); // return a copy of this pen with a different width

private:
    Color c;
};

#endif // PEN_H
