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

#ifndef RODADDER_H
#define RODADDER_H

#include <QGraphicsSceneMouseEvent>

class Rod;
class Node;
class GraphicsScene;

class RodAdder final
{
public:
    explicit RodAdder(GraphicsScene *graphicsScene);
    ~RodAdder();
    // prevent copying or moving this class (because how would a copy look like and since it is not necessary to move it around there is no need to implement the move-operation)
    RodAdder(const RodAdder &) = delete;
    RodAdder(RodAdder &&) = delete;
    RodAdder &operator =(const RodAdder &) = delete;
    RodAdder &operator =(RodAdder &&) = delete;

    void mouseClicked(QGraphicsSceneMouseEvent *event, Node *element);
    void positionChanged(QPointF newPosition);

    bool node1IsNull() const { return node1 == nullptr; }

    void createRod();

    bool rodExistsAlready(Node *node) const; // returns true if there is already a rod between node1 and the given node

private:
    GraphicsScene *scene; // weak ptr
    Rod *rod; // weak ptr as long as it is added to the scene
    Node *node1; // weak ptr
    Node *virtualNode; // weak ptr as long as it is added to the scene
};

#endif // RODADDER_H
