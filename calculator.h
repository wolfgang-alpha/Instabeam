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

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "libs/Eigen/Eigen/Eigen"

#include <QList>

class GraphicsScene;
class Node;
class Rod;
class CurvedBeam;
class Bearing;
class TrussElement;

namespace Calculator
{
    QString calculate(GraphicsScene *scene);

    QString parseAndNumberElements(GraphicsScene *scene, int &dofCount, int &rodCount, QList<Rod *> &rods, QList<Node *> &nodes);

    QString determineESM(const QList<Rod *> &rods, QVector<Eigen::Matrix6d> &k_es);

    QString assembleCoincidenceTable(const QList<Rod *> &rods, QVector<QVector<int>> &coincidenceTable);

    QString assembleGSM(int dofCount, int rodCount, const QVector<QVector<int>> &coincidenceTable, const QVector<Eigen::Matrix6d> &k_es, Eigen::MatrixXd &K);

    QString applyConstraints(const QList<Rod *> &rods, Eigen::VectorXd &F, Eigen::VectorXb &F_k, Eigen::VectorXd &U, Eigen::VectorXb &U_k);
    
    QString solveSystemOfEquations(int dofCount, const Eigen::VectorXb &F_k, const Eigen::VectorXb &U_k, const Eigen::MatrixXd &K, Eigen::VectorXd &F,
                                   Eigen::VectorXd &U);

    QString applyResults(GraphicsScene *scene, const QList<Rod *> &rods, const Eigen::VectorXd &F, const Eigen::VectorXd &U);

    void cleanUp(GraphicsScene *scene);
}

#endif // CALCULATOR_H
