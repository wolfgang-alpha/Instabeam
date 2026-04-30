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

#include "curvedbeam.h"

#include "node.h"
#include "widgets/easychange/curvedbeamdialog.h"
#include "utilities.h"
#include "graphicsscene.h"
#include "widgets/mainwindow.h"
#include "elements/label.h"
#include "jsonkeys.h"

#include <QPainter>
#include <QGraphicsView>
#include <QJsonObject>

CurvedBeam::CurvedBeam() :
    Rod(),
    R(1)
{
}

CurvedBeam::CurvedBeam(Node *elementNode1, Node *elementNode2) :
    Rod(elementNode1, elementNode2),
    R(1)
{
}

double CurvedBeam::getEffectiveR() const
{
    double L = getLength(); // chord length in meters
    double R_abs = fabs(R);
    double R_min = L / 2.0 * 1.01; // small margin above semicircle to avoid singularity
    if (R_abs < R_min) {
        return (R >= 0 ? 1.0 : -1.0) * R_min;
    }
    return R;
}

double CurvedBeam::getOpeningAngle() const
{
    double L = getLength(); // chord length in meters
    double R_eff = fabs(getEffectiveR());
    if (R_eff == 0) {
        return 0;
    }
    return 2.0 * asin(L / (2.0 * R_eff));
}

double CurvedBeam::getArcLength() const
{
    return fabs(getEffectiveR()) * getOpeningAngle();
}

QPointF CurvedBeam::getArcCenter() const
{
    if (node1 == nullptr || node2 == nullptr) {
        return {0, 0};
    }
    double scaleValue = static_cast<GraphicsScene *>(scene())->getScaleValue();
    double R_eff = getEffectiveR();
    double R_px = fabs(R_eff) * scaleValue;

    double dx = node2->x() - node1->x();
    double dy = node2->y() - node1->y();
    double L_px = sqrt(dx * dx + dy * dy);

    if (L_px == 0) {
        return {(node1->x() + node2->x()) / 2, (node1->y() + node2->y()) / 2};
    }

    // midpoint of chord
    QPointF M((node1->x() + node2->x()) / 2.0, (node1->y() + node2->y()) / 2.0);

    // perpendicular direction: (dy, -dx) points "left" in y-up convention when walking from node1 to node2
    QPointF perp;
    if (R_eff > 0) {
        perp = QPointF(dy, -dx) / L_px;
    } else {
        perp = QPointF(-dy, dx) / L_px;
    }

    // distance from midpoint to center
    double half_chord = L_px / 2.0;
    double d = sqrt(R_px * R_px - half_chord * half_chord);

    return M + d * perp;
}

QPainterPath CurvedBeam::getArcPath() const
{
    if (node1 == nullptr || node2 == nullptr) {
        return {};
    }
    double scaleValue = static_cast<GraphicsScene *>(scene())->getScaleValue();
    double R_eff = getEffectiveR();
    double R_px = fabs(R_eff) * scaleValue;

    QPointF C = getArcCenter();

    // angles from center to each node (in screen coords)
    double beta0 = atan2(node1->y() - C.y(), node1->x() - C.x());
    double beta1 = atan2(node2->y() - C.y(), node2->x() - C.x());

    // compute sweep angle (R_eff > 0 → clockwise in screen, R_eff < 0 → counterclockwise)
    double sweep = beta1 - beta0;
    if (R_eff > 0) {
        // want clockwise (negative sweep in math convention)
        while (sweep > 0) sweep -= 2 * M_PI;
        while (sweep < -2 * M_PI) sweep += 2 * M_PI;
    } else {
        // want counterclockwise (positive sweep)
        while (sweep < 0) sweep += 2 * M_PI;
        while (sweep > 2 * M_PI) sweep -= 2 * M_PI;
    }

    // draw arc point by point
    int numPoints = 50;
    QPainterPath path;
    for (int i = 0; i <= numPoints; i++) {
        double t = static_cast<double>(i) / numPoints;
        double beta = beta0 + t * sweep;
        QPointF point(C.x() + R_px * cos(beta), C.y() + R_px * sin(beta));
        if (i == 0) {
            path.moveTo(point);
        } else {
            path.lineTo(point);
        }
    }
    return path;
}

QRectF CurvedBeam::boundingRect() const
{
    if (node1 != nullptr && node2 != nullptr) {
        QPainterPath arc = getArcPath();
        QRectF rect = arc.boundingRect();
        QRectF deformedRect = getDeformedArc().boundingRect();
        rect = rect.united(deformedRect);
        return rect.adjusted(-pen.width(), -pen.width(), pen.width(), pen.width());
    }
    return {0, 0, 0, 0};
}

void CurvedBeam::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();
    painter->setPen(pen);
    if (static_cast<MainWindow *>(scene()->parent())->getColorRods()) {
        painter->setPen(pen.differentColor(colorMap.getColor(getInnerForce())));
    }
    if (static_cast<MainWindow *>(scene()->parent())->getMarkZeroLoadingRods() && Utilities::isequal(getInnerForce(), 0)) {
        painter->setPen(pen.differentColor(Color(Qt::lightGray)));
    }
    if (isUnderHoverAction) {
        painter->setPen(pen.differentColor(hoverPenColor));
    }
    if (isInSelection) {
        painter->setPen(pen.differentColor(selectionPenColor));
    }
    if (node1 != nullptr && node2 != nullptr) {
        painter->drawPath(getArcPath());
        if (static_cast<MainWindow *>(scene()->parent())->getDrawDeformedSystem()) {
            painter->save();
            QPen dpen(Qt::gray);
            dpen.setWidth(3);
            painter->setPen(dpen);
            painter->drawPath(getDeformedArc());
            painter->restore();
        }
    }
    painter->restore();
}

QPainterPath CurvedBeam::getDeformedArc() const
{
    if (node1 == nullptr || node2 == nullptr) {
        return {};
    }

    double R_eff = getEffectiveR();
    double R_abs = fabs(R_eff);
    double alpha = getOpeningAngle();
    double scaleValue = static_cast<GraphicsScene *>(scene())->getScaleValue();
    double R_px = R_abs * scaleValue;

    // Undo display sign-flips (indices 0,2) to recover solver-convention global DOFs,
    // then transform to physical local DOFs
    Eigen::Vector6d u_global;
    u_global << -u[0], u[1], -u[2], u[3], u[4], u[5];
    Eigen::Vector6d u_local = T.inverse() * u_global;

    // Scale rotation DOFs by R for arc-angle interpolation
    double u_nb[6] = {u_local[0], u_local[1] * R_abs, u_local[2], u_local[3] * R_abs, u_local[4], u_local[5]};

    // Tangent angles at nodes (same computation as in calculator ESM)
    double gamma = getAngle();
    double half_alpha = alpha / 2.0;
    double theta0, theta1;
    if (R_eff > 0) {
        theta0 = gamma + half_alpha;
        theta1 = gamma - half_alpha;
    } else {
        theta0 = gamma - half_alpha;
        theta1 = gamma + half_alpha;
    }

    // Arc geometry for undeformed positions
    QPointF C = getArcCenter();
    double beta0 = atan2(node1->y() - C.y(), node1->x() - C.x());
    double beta1 = atan2(node2->y() - C.y(), node2->x() - C.x());
    double sweep = beta1 - beta0;
    if (R_eff > 0) {
        while (sweep > 0) sweep -= 2 * M_PI;
        while (sweep < -2 * M_PI) sweep += 2 * M_PI;
    } else {
        while (sweep < 0) sweep += 2 * M_PI;
        while (sweep > 2 * M_PI) sweep -= 2 * M_PI;
    }

    double sv = maxDisplacement == 0 ? 1 : static_cast<GraphicsScene *>(scene())->getMaxDisplacementDistance() / maxDisplacement;

    int numPoints = 50;
    QPainterPath path;
    for (int i = 0; i <= numPoints; i++) {
        double t = static_cast<double>(i) / numPoints;
        double phi = t * alpha;
        double xi = t;

        // Shape functions (positive convention, arc-angle scaled)
        double h0 = 1 - 3 * xi * xi + 2 * xi * xi * xi;
        double h1 = phi * (1 - xi) * (1 - xi);
        double h2 = 3 * xi * xi - 2 * xi * xi * xi;
        double h3 = phi * (xi * xi - xi);
        double h4 = 1 - xi;
        double h5 = xi;

        // Local transverse and tangential displacements [m]
        double w_val = u_nb[0] * h0 + u_nb[1] * h1 + u_nb[2] * h2 + u_nb[3] * h3;
        double u_val = u_nb[4] * h4 + u_nb[5] * h5;

        // Tangent angle at this point (linearly interpolated along arc)
        double theta = theta0 + (theta1 - theta0) * t;
        double ct = cos(theta), st = sin(theta);

        // Map local (w, u) to screen displacement via tangent-angle rotation.
        // In calculator coords (x right, y down = screen coords):
        //   dx = -sin(θ)*w + cos(θ)*u
        //   dy =  cos(θ)*w + sin(θ)*u
        // This guarantees continuity at shared nodes.
        double dx = (-st * w_val + ct * u_val) * sv;
        double dy = ( ct * w_val + st * u_val) * sv;

        // Undeformed arc point in scene coords
        double beta = beta0 + t * sweep;
        double px = C.x() + R_px * cos(beta);
        double py = C.y() + R_px * sin(beta);

        QPointF point(px + dx, py + dy);
        if (i == 0) {
            path.moveTo(point);
        } else {
            path.lineTo(point);
        }
    }
    return path;
}

double CurvedBeam::getMaxArcDisplacement() const
{
    double R_abs = fabs(getEffectiveR());
    double alpha = getOpeningAngle();

    // Undo display sign-flips, then transform to physical local DOFs
    Eigen::Vector6d u_global;
    u_global << -u[0], u[1], -u[2], u[3], u[4], u[5];
    Eigen::Vector6d u_local = T.inverse() * u_global;
    double u_nb[6] = {u_local[0], u_local[1] * R_abs, u_local[2], u_local[3] * R_abs, u_local[4], u_local[5]};

    double maxD = 0;
    for (int i = 0; i <= 50; i++) {
        double t = static_cast<double>(i) / 50;
        double phi = t * alpha;
        double xi = t;

        double h0 = 1 - 3 * xi * xi + 2 * xi * xi * xi;
        double h1 = phi * (1 - xi) * (1 - xi);
        double h2 = 3 * xi * xi - 2 * xi * xi * xi;
        double h3 = phi * (xi * xi - xi);
        double h4 = 1 - xi;
        double h5 = xi;

        double w_val = fabs(u_nb[0] * h0 + u_nb[1] * h1 + u_nb[2] * h2 + u_nb[3] * h3);
        double u_val = fabs(u_nb[4] * h4 + u_nb[5] * h5);

        if (w_val > maxD) maxD = w_val;
        if (u_val > maxD) maxD = u_val;
    }
    return maxD;
}

QPainterPath CurvedBeam::shape() const
{
    QPainterPath arc = getArcPath();
    QPainterPathStroker stroker;
    stroker.setWidth(pen.width() + 4); // a bit wider than the visual line for easier clicking
    return stroker.createStroke(arc);
}

QJsonObject CurvedBeam::saveAsJson() const
{
    QJsonObject o(Rod::saveAsJson());
    o.insert(JsonKeys::elementType, static_cast<int>(ElementType::CurvedBeam)); // override Rod's element type
    o.insert(JsonKeys::radius, R);
    return o;
}

void CurvedBeam::loadFromJson(const QJsonObject &object)
{
    Rod::loadFromJson(object);
    R = object.value(JsonKeys::radius).toDouble(1.0);
}

EasyChangeDialog *CurvedBeam::createEasyChangeDialog()
{
    return new CurvedBeamDialog(scene()->views().first(), this);
}

QString CurvedBeam::getElementSelectionDialogButtonText() const
{
    return QString("Curved beam (ID: ") + getId() + QString(")");
}
