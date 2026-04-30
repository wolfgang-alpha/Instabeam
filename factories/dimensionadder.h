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

#ifndef DIMENSIONADDER_H
#define DIMENSIONADDER_H

class Node;
class Dimension;
class GraphicsScene;
class QGraphicsSceneMouseEvent;
class QPointF;

class DimensionAdder final
{
public:
    DimensionAdder(GraphicsScene *graphicsScene);
    ~DimensionAdder();
    // prevent copying/moving this class
    DimensionAdder(const DimensionAdder &) = delete;
    DimensionAdder(DimensionAdder &&) = delete;
    DimensionAdder &operator =(const DimensionAdder &) = delete;
    DimensionAdder &operator =(DimensionAdder &&) = delete;

    void mouseClicked(QGraphicsSceneMouseEvent *event, Node *node);

    void positionChanged(const QPointF &newPosition);

private:
    void createDimension();

    GraphicsScene *scene; // weak ptr
    Dimension *dimension; // owner is this as long as it is not added to the scene
    Node *node1; // weak ptr
    Node *virtualNode; // owner as long as it is not added to the scene
};

#endif // DIMENSIONADDER_H
