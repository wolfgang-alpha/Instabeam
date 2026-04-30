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

#ifndef LABELADDER_H
#define LABELADDER_H

#include <QGraphicsSceneMouseEvent>

class Label;
class GraphicsScene;

class LabelAdder final
{
public:
    LabelAdder(GraphicsScene *graphicsScene); // see NodeAdder for comments
    ~LabelAdder();
    // prevent copying or moving this class (because how would a copy look like and since it is not necessary to move it around there is no need to implement the move-operation)
    LabelAdder(const LabelAdder &) = delete;
    LabelAdder(LabelAdder &&) = delete;
    LabelAdder &operator =(const LabelAdder &) = delete;
    LabelAdder &operator =(LabelAdder &&) = delete;

    void mouseClicked(QGraphicsSceneMouseEvent *event);

    void positionChanged(const QPointF &newPosition);

    void addLabelPermanently();

private:
    GraphicsScene *scene;
    Label *label;
};

#endif // LABELADDER_H
