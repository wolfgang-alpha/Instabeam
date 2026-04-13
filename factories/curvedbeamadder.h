#ifndef CURVEDBEAMADDER_H
#define CURVEDBEAMADDER_H

#include <QGraphicsSceneMouseEvent>

class CurvedBeam;
class Node;
class GraphicsScene;

class CurvedBeamAdder final
{
public:
    explicit CurvedBeamAdder(GraphicsScene *graphicsScene);
    ~CurvedBeamAdder();
    CurvedBeamAdder(const CurvedBeamAdder &) = delete;
    CurvedBeamAdder(CurvedBeamAdder &&) = delete;
    CurvedBeamAdder &operator =(const CurvedBeamAdder &) = delete;
    CurvedBeamAdder &operator =(CurvedBeamAdder &&) = delete;

    void mouseClicked(QGraphicsSceneMouseEvent *event, Node *element);
    void positionChanged(QPointF newPosition);

    bool node1IsNull() const { return node1 == nullptr; }

    void createCurvedBeam();

    bool curvedBeamExistsAlready(Node *node) const;

private:
    GraphicsScene *scene; // weak ptr
    CurvedBeam *curvedBeam; // weak ptr as long as it is added to the scene
    Node *node1; // weak ptr
    Node *virtualNode; // weak ptr as long as it is added to the scene
};

#endif // CURVEDBEAMADDER_H
