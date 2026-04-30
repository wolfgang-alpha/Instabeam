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

#ifndef ROD_H
#define ROD_H

#include "trusselement.h"
#include "calculator.h"
#include "pen.h"

class Node;
class Label;

class Rod : public TrussElement
{
    Q_OBJECT

protected:
    class ColorMap
    {
    public:
        explicit ColorMap() : minValue(0), maxValue(1) {}

        void setMinValue(double value)
        {
            minValue = value;
        }

        void setMaxValue(double value)
        {
            maxValue = value;
        }

        Color getColor(double value)
        {
            Color color;
            color.setHsl(240 * (1 - (value - minValue) / (maxValue - minValue)), 255, 128); // red (hue = 0) means biggest pull-load, blue (hue = 240) biggest push-load
            return color;
        }

    private:
        double minValue; // smallest value of all rod-forces (including the sign, therefore biggest push-load)
        double maxValue; // biggest value of all rod-forces
    };

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW // this is required for c++14 or lower because of memory-alignment-issues (see http://eigen.tuxfamily.org/dox-devel/group__TopicUnalignedArrayAssert.html)

    explicit Rod(); // default ctor
    Rod(Node *elementNode1, Node *elementNode2);
    ~Rod() override;
    // disable copying/moving because of member-ptrs (implement later if necessary)
    Rod(const Rod &) = delete;
    Rod(Rod &&) = delete;
    Rod &operator =(const Rod &) = delete;
    Rod &operator =(Rod &&) = delete;

    Node *getNode1() const { return node1; }
    Node *getNode2() const { return node2; }
    Node *takeNode1();
    Node *takeNode2();
    void setNode2(Node *newNode2); // this fct overrides the old ptr to node2

    void forwardPrepareGeometryChange();

    void setColorMapValues(double minValue, double maxValue);

    Label *getLabel() const { return label; } // returns weak ptr
    void setLabel(Label *newLabel); // if the label exists already, it gets deleted and label is set to newLabel; the rod takes ownership of newLabel

    void moveLabelX(double distance);
    void moveLabelY(double distance);

    int getThickness() const { return pen.width(); }

    void linkNode1(Node *newNode1); // only sets node1 to newNode1 without modifying anything else (only call this fct from the json-file-loading-process)
    void linkNode2(Node *newNode2); // only sets node2 to newNode2 without modifying anything else (only call this fct from the json-file-loading-process)

    void setE(double e) { prepareGeometryChange(); E = e; }
    double getE() const { return E; } // 210000000000
    void setA(double a) { prepareGeometryChange(); A = a; }
    double getA() const { return A; } // 0.001106
    void setI(double i) { prepareGeometryChange(); I = i; }
    double getI() const { return I; } // 0.00000171
    double getEA() const { return getE() * getA(); } // [N]
    double getEI() const { return getE() * getI(); } // [Nm²]
    double getLength() const; // [m]
    double getAngle() const; // [rad]

    void setInnerForce(double newInnerForce) { innerForce = newInnerForce; } // sets the normal-force in the rod
    double getInnerForce() const { return innerForce; } // returns the normal-force in the rod

    void setCalcId(int internCalcId) { calcId = internCalcId; }
    int getCalcId() const { return calcId; }

    void setDOF(int localId, int globalId) { dof[localId] = globalId; }
    int getDOF(int localId) const { return dof[localId]; }

    void setU(int localId, double value) { prepareGeometryChange(); u[localId] = value; }
    double getU(int localId) const { return u[localId]; }

    double wGraph(double x) const; // returns w(x)
    double uGraph(double x) const; // returns u(x)
    double getPhi(int index, double x) const; // returns phi(dof, x), where phi_dof is the interpolation-fct to get an approximation for the deformation between the nodes

    void setElementTransformationMatrix(const Eigen::Matrix6d &matrix);
    Eigen::Matrix6d getElementTransformationMatrix() const { return T; }

    QPainterPath getDeformedRod() const; // [px]

    void setMaxDisplacement(double d); // [px]
    double getMaxDisplacement() const { return maxDisplacement; } // [px]

protected:
    Node *node1; // weak ptr
    Node *node2; // weak ptr
    Pen pen;
    static ColorMap colorMap;
    Id<Rod> id;
    Label *label; // this is parent
    int calcId;
    double innerForce;
    int dof[6]; // y1, m1, y2, m2, x1, x2 in global (x right, y up, m counterclockwise positive) coords (index 1: node1, index2: node2)
    double u[6]; // y1, m1, y2, m2, x1, x2 in global coords
    Eigen::Matrix6d T; // element-transformation-matrix
    static double maxDisplacement; // [px]
    double E; // young's modulus [N/m²]
    double A; // cross-section area [m²]
    double I; // area-moment of inertia [m^4]

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QPainterPath shape() const override;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *) override {} // override to do nothing

    // TrussElement interface
public:
    EasyChangeDialog  *createEasyChangeDialog() override;
    QString getElementSelectionDialogButtonText() const override;
    QString getId() const override;
    QJsonObject saveAsJson() const override;
    void loadFromJson(const QJsonObject &object) override;
};

#endif // ROD_H
