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

#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>

#include <memory>

class TrussElement;
class Node;
class NodeAdder;
class RodAdder;
class CurvedBeamAdder;
class BearingAdder;
class ForceAdder;
class DimensionAdder;
class LabelAdder;
class MainWindow;

class GraphicsScene final : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GraphicsScene(MainWindow *parent = nullptr);
    GraphicsScene(const QJsonObject &object, MainWindow *parent = nullptr); // construct scene with properties given by the json-object
    ~GraphicsScene() override; // not designed to be inherited
    // disable copying/moving the scene since copying is prevented by the base class and moving is not needed therefore it is not necessary to implement it
    GraphicsScene(const GraphicsScene &) = delete;
    GraphicsScene(GraphicsScene &&) = delete;
    GraphicsScene &operator =(const GraphicsScene &) = delete;
    GraphicsScene &operator =(GraphicsScene &&) = delete;

    QList<TrussElement *> getElements(const QPointF &point) const; // point is in scene-coords, returns list of weak ptrs
    TrussElement *getElementAt(const QPointF &point) const; // point is in scene-coords, returns weak ptr
    Node *getNodeAt(const QPointF &point) const; // point is in scene-coords

    void removeElement(TrussElement *element);
    void removeElement(const QList<TrussElement *> &elementList);
    void removeElementAt(const QPointF &point); // point is in scene-coords
    void clearScene();

    void validateNode(Node *node);

    void addNode(bool checked);
    void addRod(bool checked);
    void addCurvedBeam(bool checked);
    void addBearing(bool checked);
    void addForce(bool checked);
    void addDimension(bool checked);
    void addLabel(bool checked);

    bool nodeAdderActive() { return nodeAdder != nullptr; } // returns true if in node-adding-mode
    bool rodAdderActive() { return rodAdder != nullptr; }
    bool curvedBeamAdderActive() { return curvedBeamAdder != nullptr; }
    bool labelAdderActive() { return labelAdder != nullptr; }

    TrussElement *getEasyChangeDialogElement() const; // returns weak-ptr
    void closeEasyChangeDialog();

    NodeAdder *getNodeAdder() const { return nodeAdder.get(); } // returns weak ptr
    LabelAdder *getLabelAdder() const { return labelAdder.get(); } // returns weak ptr

    QJsonObject saveAsJson() const;

    void setScaleValue(double newScaleValue);
    double getScaleValue() const { return scaleValue; }

    void setMaxDisplacementDistance(double d);
    double getMaxDisplacementDistance() const { return maxDisplacementDistance; }

    void setDisplacementCalculationStep(double s) { displacementCalculationStep = s; }
    double getDisplacementCalculationStep() const { return displacementCalculationStep; }

private:
    template<typename T>
    void setupElementFromJson(const QJsonValue &jsonElement, QList<QPair<QString, TrussElement *>> &memoryMap);

    std::unique_ptr<NodeAdder> nodeAdder;
    std::unique_ptr<RodAdder> rodAdder;
    std::unique_ptr<CurvedBeamAdder> curvedBeamAdder;
    std::unique_ptr<BearingAdder> bearingAdder;
    std::unique_ptr<ForceAdder> forceAdder;
    std::unique_ptr<DimensionAdder> dimensionAdder;
    std::unique_ptr<LabelAdder> labelAdder;

    double scaleValue; // holds how many pixels one meter are [px/m]
    bool clickInEmptySceneSpace; // true if the user clicks in empty scene space
    double maxDisplacementDistance; // distance that the max displacement is drawn away from the unloaded rod
    double displacementCalculationStep; // indicates how fine the deformed system is drawn

    // QGraphicsScene interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // GRAPHICSSCENE_H
