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

#ifndef NODEADDER_H
#define NODEADDER_H

#include <QGraphicsSceneMouseEvent>

class Node;
class GraphicsScene;

class NodeAdder final
{
public:
    explicit NodeAdder(GraphicsScene *graphicsScene);
    ~NodeAdder();
    // prevent copying or moving this class (because how would a copy look like and since it is not necessary to move it around there is no need to implement the move-operation)
    NodeAdder(const NodeAdder &) = delete;
    NodeAdder(Node &&) = delete;
    NodeAdder &operator =(const NodeAdder &) = delete;
    NodeAdder &operator =(NodeAdder &&) = delete;

    void positionChanged(const QPointF &newPosition);
    void mouseClicked(QGraphicsSceneMouseEvent *event);

    void addNodePermanently();

private:
    GraphicsScene *scene; // weak ptr
    Node *node; // owner when not added to the scene
};

#endif // NODEADDER_H
