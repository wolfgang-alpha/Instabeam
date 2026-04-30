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

#ifndef BEARING_H
#define BEARING_H

#include "elements/trusselement.h"
#include "calculator.h"
#include "pen.h"

#include <QDebug>

class Node;

enum class BearingType {
    LocatingBearing = 0,
    FloatingBearing = 1,
    FixedClamping = 2
};

class Bearing : public TrussElement
{
    Q_OBJECT

public:
    explicit Bearing(Node *parentNode = nullptr);
    ~Bearing() override;
    // disable copy-/move-operations because of member-ptrs, implement later if needed
    Bearing(const Bearing &) = delete;
    Bearing(Bearing &&) = delete;
    Bearing &operator =(const Bearing &) = delete;
    Bearing &operator =(Bearing &&) = delete;

    void setAngle(double newAngle);
    double getAngle() const { return angle; }

    void setBearingType(BearingType newBearingType);
    BearingType getBearingType() const { return bearingType; }

    QList<double> getReactionForces() const;
    double getReactionForceX() const;
    double getReactionForceY() const;
    double getReactionMomentZ() const;

private:
    BearingType bearingType;
    Pen pen;
    int bearingWidthTotal; // [px]
    int bearingHeight; // [px] (this is the height for floating-bearings, subtract floatingBearingVerticalSpacing to get the height for locating-bearings
    int floatingBearingVerticalSpacing; // [px]
    double angle; // [°] (0° is left, angle increases counter-clockwise)
    double fx;
    double fy;
    Id<Bearing> id;
    int fixedClampingWidth; // [px]
    int fixedClampingHeight; // [px]

    // TrussElement interface
public:
    EasyChangeDialog *createEasyChangeDialog() override;
    QString getElementSelectionDialogButtonText() const override;
    QString getId() const override;
    QJsonObject saveAsJson() const override;
    void loadFromJson(const QJsonObject &object) override;

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QPainterPath shape() const override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *) override {} // the bearing should not be moveable by the mouse
};

#endif // BEARING_H
