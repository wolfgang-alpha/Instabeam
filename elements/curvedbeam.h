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

#ifndef CURVEDBEAM_H
#define CURVEDBEAM_H

#include "rod.h"

class Node;

class CurvedBeam : public Rod
{
    Q_OBJECT

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    explicit CurvedBeam();
    CurvedBeam(Node *elementNode1, Node *elementNode2);
    ~CurvedBeam() override = default;
    CurvedBeam(const CurvedBeam &) = delete;
    CurvedBeam(CurvedBeam &&) = delete;
    CurvedBeam &operator =(const CurvedBeam &) = delete;
    CurvedBeam &operator =(CurvedBeam &&) = delete;

    void setR(double r) { prepareGeometryChange(); R = r; }
    double getR() const { return R; }
    double getEffectiveR() const; // [m], auto-clamped so |R| >= L/2, sign preserved

    double getOpeningAngle() const; // [rad], always positive
    double getArcLength() const; // [m]

    QPointF getArcCenter() const; // in scene coords [px]

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QPainterPath shape() const override;

    // TrussElement interface
public:
    EasyChangeDialog *createEasyChangeDialog() override;
    QString getElementSelectionDialogButtonText() const override;
    QJsonObject saveAsJson() const override;
    void loadFromJson(const QJsonObject &object) override;

    QPainterPath getDeformedArc() const; // deformed arc path for painting [scene coords]
    double getMaxArcDisplacement() const; // max |w| or |u| along the arc [m]

private:
    double R; // radius of curvature [m], sign determines direction: positive = center to left of chord (y-up convention)

    QPainterPath getArcPath() const; // undeformed arc path for painting
};

#endif // CURVEDBEAM_H
