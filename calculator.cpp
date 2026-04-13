#include "calculator.h"

#include "graphicsscene.h"
#include "elements/bearing.h"
#include "elements/node.h"
#include "elements/rod.h"
#include "elements/curvedbeam.h"
#include "elements/singleforce.h"
#include "utilities.h"
#include "widgets/mainwindow.h"

QString Calculator::calculate(GraphicsScene *scene)
{
    // prepare elements to use in this fct (set all ids to -1, because from the last call of calculate() there are different values set)
    cleanUp(scene);

    // number rods and nodes
    int dofCount = 0; // init with 0 because parseAndNumberElements needs it to be 0 initially
    int rodCount = 0;
    QList<Rod *> rods;
    QList<Node *> nodes;
    auto status = parseAndNumberElements(scene, dofCount, rodCount, rods, nodes);
    if (status != "") { // check if the fct called above would result in an error, if so abort calculation with a message provided by the fct
        return status;
    }

    // examine ESM
    QVector<Eigen::Matrix6d> k_es(rodCount);
    status = determineESM(rods, k_es);
    if (status != "") {
        return status;
    }

    // compose coincidence-table
    QVector<QVector<int>> coincidenceTable(rodCount);
    status = assembleCoincidenceTable(rods, coincidenceTable);
    if (status != "") {
        return status;
    }

    // assemble GSM
    Eigen::MatrixXd K = Eigen::MatrixXd::Zero(dofCount, dofCount); // initialize GSM with zeros
    status = assembleGSM(dofCount, rodCount, coincidenceTable, k_es, K);
    if (status != "") {
        return status;
    }

    // apply constraints
    Eigen::VectorXd F = Eigen::VectorXd::Zero(dofCount); // initialize GLV with zeros (because the values are unknown, boundary conditions get applied below)
    Eigen::VectorXb F_k = Eigen::VectorXb::Constant(dofCount, false); // initialize GLV-known-vector with false
    Eigen::VectorXd U = Eigen::VectorXd::Zero(dofCount); // initialize GVV with zeros
    Eigen::VectorXb U_k = Eigen::VectorXb::Constant(dofCount, false); // initialize GVV-known-vector with false
    status = applyConstraints(rods, F, F_k, U, U_k);
    if (status != "") {
        return status;
    }

    // solve K * U = F
    status = solveSystemOfEquations(dofCount, F_k, U_k, K, F, U);
    if (status != "") {
        return status;
    }

    // set the variables for the translations and reaction forces to the calculated values
    status = applyResults(scene, rods, F, U);
    if (status != "") {
        return status;
    }

    // everything went good, no error occured, therefore return empty string to clear the status-bar-message
    return status;
}

QString Calculator::parseAndNumberElements(GraphicsScene *scene, int &dofCount, int &rodCount, QList<Rod *> &rods, QList<Node *> &nodes)
{
    // this fct sets the calc-ids of all nodes, rods and dofs; it also applies transition-conditions by numbering corresponding dofs the same (boundary-conditions are applied later in another fct)
    int nodeCount = 0;
    for (auto element : scene->items()) {
        if (auto rod = dynamic_cast<Rod *>(element)) { // loop through every rod
            rod->setCalcId(rodCount++);
            rods.append(rod); // append rod to global list
            for (int n = 0; n < 2; n++) {
                Node *node = n == 0 ? rod->getNode1() : rod->getNode2();
                if (node->getCalcId() == -1) { // assign calc-id if the node has no id yet
                    node->setCalcId(nodeCount++);
                    nodes.append(node); // append node to global list if not already present
                }
                QVector<int> range = n == 0 ? QVector<int>{0, 1, 4} : QVector<int>{2, 3, 5}; // select the dofs depending on the current node (1 or 2)
                auto connectedRods = node->getRods();
                if (connectedRods.isEmpty()) {
                    return "There is a not connected node present, aborting calculation.";
                }
                if (connectedRods.length() > 1) {
                    for (int i : range) { // loop through the dofs the corresponding node is connected to
                        if ((i == 1 || i == 3) && node->getNodeType() == NodeType::GerberJoint) {
                            rod->setDOF(i, dofCount++); // if the node is a gerber-joint assign different dof-ids to the rotation-dofs of the connected rods
                        } else { // weld or translation-dof
                            int dofId = -1; // get dof-id of the connected rods if one is set already
                            for (auto diffRod : connectedRods) { // loop through all connected rods
                                if (n == 0) { // node is node1
                                    if (node == diffRod->getNode2()) { // map dof {0, 1, 4}_thisRod to {2, 3, 5}_otherRod if this_node1 is connected to other_node2
                                        if (i == 0 && diffRod->getDOF(2) != -1) {
                                            dofId = diffRod->getDOF(2); // set dofId to the same as the other rod if set
                                            break; // break search for already set ids if one is found
                                        } else if (i == 1 && diffRod->getDOF(3) != -1) {
                                            dofId = diffRod->getDOF(3);
                                            break;
                                        } else if (i == 4 && diffRod->getDOF(5) != -1) {
                                            dofId = diffRod->getDOF(5);
                                            break;
                                        }
                                    } else { // no mapping, dof {0, 1, 4}_thisRod equals {0, 1, 4}_otherRod
                                        if (diffRod->getDOF(i) != -1) {
                                            dofId = diffRod->getDOF(i);
                                            break;
                                        }
                                    }
                                } else { // node is node2
                                    if (node == diffRod->getNode1()) { // map dof {2, 3, 5}_thisRod to {0, 1, 4}_otherRod if this_node2 is connected to other_node1
                                        if (i == 2 && diffRod->getDOF(0) != -1) {
                                            dofId = diffRod->getDOF(0);
                                            break;
                                        } else if (i == 3 && diffRod->getDOF(1) != -1) {
                                            dofId = diffRod->getDOF(1);
                                            break;
                                        } else if (i == 5 && diffRod->getDOF(4) != -1) {
                                            dofId = diffRod->getDOF(4);
                                            break;
                                        }
                                    } else { // no mapping, dof {2, 3, 5}_thisRod equals {2, 3, 5}_otherRod
                                        if (diffRod->getDOF(i) != -1) {
                                            dofId = diffRod->getDOF(i);
                                            break;
                                        }
                                    }
                                }
                            }
                            if (dofId == -1) { // other rod's dof not set yet
                                rod->setDOF(i, dofCount++); // if the current dof is a x- or y-dof (or z-dof at a weld) assign a new id to the rod's dof if the other rod's dof is not set yet
                            } else { // other rod's dof already set
                                rod->setDOF(i, dofId); // if the current dof is a x- or y-dof (or z-dof at a weld) assign the same dof to the connected rods if present
                            }
                        }
                    }
                } else { // if there is only one rod to the node connected, just index the dofs at this node
                    for (int i : range) {
                        rod->setDOF(i, dofCount++);
                    }
                }
            }
        }
    }
    if (rodCount == 0 || nodeCount == 0) {
        return "there are no rods or nodes";
    }
    return "";
}

QString Calculator::determineESM(const QList<Rod *> &rods, QVector<Eigen::Matrix6d> &k_es)
{
    for (auto rod : rods) {
        int id = rod->getCalcId();
        Eigen::Matrix6d k_e_local; // element-stiffness-matrix in element-coords
        Eigen::Matrix6d T_e; // element-transformation-matrix

        if (auto curved = dynamic_cast<CurvedBeam *>(rod)) {
            // curved beam element stiffness matrix (Hermite Bogenelemente)
            double Ev = curved->getE();
            double Av = curved->getA();
            double Iv = curved->getI();
            double Rv = fabs(curved->getEffectiveR());
            double AE = Av * Ev;
            double EI = Ev * Iv;
            double a = curved->getOpeningAngle(); // opening angle [rad]
            if (a <= 0) {
                return "CurvedBeam has invalid geometry (radius too small for chord length).";
            }
            double a2 = a * a;
            double a3 = a2 * a;
            double a4 = a2 * a2;
            double a5 = a2 * a3;
            double a6 = a3 * a3;
            double R2 = Rv * Rv;
            double R3 = R2 * Rv;

            // Raw ESM from the Hermite curved beam derivation (Parkus S.247).
            // The notebook's energy formulation omits the 1/2 factor in the strain energy integral,
            // so all entries are 2x the correct stiffness values. We divide by 2 below.
            // Additionally, the notebook's DOF 1 and 3 (rotations) are scaled by R compared to
            // the standard convention (U_notebook = R * theta). We apply the DOF scaling
            // D = diag(1, R, 1, R, 1, 1) via: k_final(i,j) = k_raw(i,j) * D[i] * D[j] / 2.
            k_e_local(0,0) = 96*AE*a/(35*Rv) - 144*EI/(R3*a3) + (-12*AE*R2*a4 + 288*EI)/(3*R3*a3) + (2*AE*R2*a4 + 72*EI)/(R3*a3);
            k_e_local(0,1) = -46*AE*a2/(105*Rv) - 48*EI/(R3*a2) + (-AE*R2*a5 + 84*EI*a)/(R3*a3) + (4*AE*R2*a5 - 144*EI*a)/(3*R3*a3);
            k_e_local(0,2) = -61*AE*a/(35*Rv) + 72*EI/(R3*a3) + (6*AE*R2*a4 - 288*EI)/(3*R3*a3);
            k_e_local(0,3) = -127*AE*a2/(210*Rv) + 36*EI/(R3*a2) + (2*AE*R2*a5 - 144*EI*a)/(3*R3*a3);
            k_e_local(0,4) = AE/Rv + 12*EI/(R3*a2) + (-2*AE*R2*a3 - 12*EI*a)/(R3*a3);
            k_e_local(0,5) = -AE/Rv - 12*EI/(R3*a2) + (2*AE*R2*a3 + 12*EI*a)/(R3*a3);

            k_e_local(1,1) = -68*AE*a3/(105*Rv) - 16*EI/(R3*a) + (2*AE*R2*a6 + 72*EI*a2)/(3*R3*a3);
            k_e_local(1,2) = -13*AE*a2/(210*Rv) + 12*EI/(R3*a2);
            k_e_local(1,3) = -AE*a3/(70*Rv) + 4*EI/(R3*a);
            k_e_local(1,4) = -5*AE*a/(6*Rv) + 8*EI/(R3*a) + (AE*R2*a4 - 6*EI*a2)/(R3*a3);
            k_e_local(1,5) = 5*AE*a/(6*Rv) - 8*EI/(R3*a) + (-AE*R2*a4 + 6*EI*a2)/(R3*a3);

            k_e_local(2,2) = 26*AE*a/(35*Rv) + 24*EI/(R3*a3);
            k_e_local(2,3) = 11*AE*a2/(105*Rv) + 12*EI/(R3*a2);
            k_e_local(2,4) = -AE/Rv;
            k_e_local(2,5) = AE/Rv;

            k_e_local(3,3) = 2*AE*a3/(105*Rv) + 8*EI/(R3*a);
            k_e_local(3,4) = -AE*a/(6*Rv) - 2*EI/(R3*a);
            k_e_local(3,5) = AE*a/(6*Rv) + 2*EI/(R3*a);

            k_e_local(4,4) = (2*AE*R2*a2 + 2*EI*a2)/(R3*a3);
            k_e_local(4,5) = (-2*AE*R2*a2 - 2*EI*a2)/(R3*a3);

            k_e_local(5,5) = (2*AE*R2*a2 + 2*EI*a2)/(R3*a3);

            // fill symmetric entries
            k_e_local(1,0) = k_e_local(0,1);
            k_e_local(2,0) = k_e_local(0,2);
            k_e_local(2,1) = k_e_local(1,2);
            k_e_local(3,0) = k_e_local(0,3);
            k_e_local(3,1) = k_e_local(1,3);
            k_e_local(3,2) = k_e_local(2,3);
            k_e_local(4,0) = k_e_local(0,4);
            k_e_local(4,1) = k_e_local(1,4);
            k_e_local(4,2) = k_e_local(2,4);
            k_e_local(4,3) = k_e_local(3,4);
            k_e_local(5,0) = k_e_local(0,5);
            k_e_local(5,1) = k_e_local(1,5);
            k_e_local(5,2) = k_e_local(2,5);
            k_e_local(5,3) = k_e_local(3,5);
            k_e_local(5,4) = k_e_local(4,5);

            // apply DOF scaling (D = diag(1, R, 1, R, 1, 1)) and energy correction (/2)
            double D[6] = {1.0, Rv, 1.0, Rv, 1.0, 1.0};
            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 6; j++) {
                    k_e_local(i, j) *= D[i] * D[j] / 2.0;
                }
            }

            // curved beam transformation matrix: different tangent angles at each node
            double gamma = curved->getAngle(); // chord angle [rad]
            double half_alpha = a / 2.0;
            double theta0, theta1;
            if (curved->getEffectiveR() > 0) {
                theta0 = gamma + half_alpha;
                theta1 = gamma - half_alpha;
            } else {
                theta0 = gamma - half_alpha;
                theta1 = gamma + half_alpha;
            }
            double c0 = cos(theta0), s0 = sin(theta0);
            double c1 = cos(theta1), s1 = sin(theta1);
            T_e <<  c0,  0,   0,  0,  s0,   0,
                     0,  1,   0,  0,   0,   0,
                     0,  0,  c1,  0,   0,  s1,
                     0,  0,   0,  1,   0,   0,
                   -s0,  0,   0,  0,  c0,   0,
                     0,  0, -s1,  0,   0,  c1;
        } else {
            // straight beam element (Timoshenko)
            double EI = rod->getEI();
            double l = rod->getLength();
            double l2 = pow(l, 2);
            double l3 = pow(l, 3);
            double EA = rod->getEA();
            k_e_local <<   12 * EI / l3, - 6 * EI / l2, - 12 * EI / l3, - 6 * EI / l2,        0,        0,
                          - 6 * EI / l2,   4 * EI / l ,    6 * EI / l2,   2 * EI / l ,        0,        0,
                         - 12 * EI / l3,   6 * EI / l2,   12 * EI / l3,   6 * EI / l2,        0,        0,
                          - 6 * EI / l2,   2 * EI / l ,    6 * EI / l2,   4 * EI / l ,        0,        0,
                                      0,             0,              0,             0,   EA / l, - EA / l,
                                      0,             0,              0,             0, - EA / l,   EA / l;
            double alpha = rod->getAngle();
            double c = cos(alpha);
            double s = sin(alpha);
            T_e <<  c,  0,  0,  0,  s,  0,
                    0,  1,  0,  0,  0,  0,
                    0,  0,  c,  0,  0,  s,
                    0,  0,  0,  1,  0,  0,
                   -s,  0,  0,  0,  c,  0,
                    0,  0, -s,  0,  0,  c;
        }
        Eigen::Matrix6d k_e = T_e.transpose() * k_e_local * T_e; // ESM in global coords
        rod->setElementTransformationMatrix(T_e);
        k_es.replace(id, k_e);
    }
    return ""; // everything went ok, indicate this to the caller-fct by returning an empty string
}

QString Calculator::assembleCoincidenceTable(const QList<Rod *> &rods, QVector<QVector<int>> &coincidenceTable)
{
    // assemble the coincidence-table
    // e. g.:
    // | element | DOF local  | 1 | 2 | 3 | 4 | 5 | 6 |
    // |   (1)   | DOF global | 2 | 8 | 4 | 5 | 1 | 3 |
    // |   (2)   | DOF global | 4 | 5 | 7 | 9 | 3 | 6 |
    // note: actually the indexing of the elements and DOFs starts at 0, not at 1 like in the example given above
    for (auto rod : rods) {
        // dof-local:                       0,              1,              2,              3,              4,              5;
        QVector<int> dofGlobal{rod->getDOF(0), rod->getDOF(1), rod->getDOF(2), rod->getDOF(3), rod->getDOF(4), rod->getDOF(5)};
        coincidenceTable.replace(rod->getCalcId(), dofGlobal);
    }
    return "";
}

QString Calculator::assembleGSM(int dofCount, int rodCount, const QVector<QVector<int>> &coincidenceTable, const QVector<Eigen::Matrix6d> &k_es,
                                Eigen::MatrixXd &K)
{
    for (int e = 0; e < rodCount; e++) {
        Eigen::MatrixXd K_tilde_e = Eigen::MatrixXd::Zero(dofCount, dofCount);
        auto k_e = k_es.at(e);
        for (int row = 0; row < k_e.rows(); row++) { // apply the coincidence-table to set the correct values for the element-GSM
            for (int col = 0; col < k_e.cols(); col++) {
                K_tilde_e(coincidenceTable.at(e).at(row), coincidenceTable.at(e).at(col)) = k_e(row, col);
            }
        }
        K += K_tilde_e; // the GSM is the sum of all element-GSMs
    }
    return "";
}

QString Calculator::applyConstraints(const QList<Rod *> &rods, Eigen::VectorXd &F, Eigen::VectorXb &F_k, Eigen::VectorXd &U, Eigen::VectorXb &U_k)
{
    // boundary-conditions are applied in this fct, transition-conditions get applied in the numbering of the dofs in parseAndNumberElements()
    for (auto rod : rods) { // if a force is applied to a node, then two dofs are constrained
        for (int n = 0; n < 2; n++) { // n == 0 means node refers to node1, n == 1 means node refers to node2
            Node *node = n == 0 ? rod->getNode1() : rod->getNode2(); // get the current node
            QVector<int> range = n == 0 ? QVector<int>{0, 1, 4} : QVector<int>{2, 3, 5}; // select the corresponding local dofs depending on the current node
            if (auto bearing = node->getBearing()) { // if a bearing is at a node, the nodes movement is restricted depending on the kind of the bearing
                if (bearing->getBearingType() == BearingType::LocatingBearing) {
                    U(rod->getDOF(range.at(0))) = 0; // y-direction immoveable
                    U_k(rod->getDOF(range.at(0))) = true;
                    U(rod->getDOF(range.at(2))) = 0; // x-direction immoveable
                    U_k(rod->getDOF(range.at(2))) = true;
                    F(rod->getDOF(range.at(1))) = 0; // z-direction moment-free
                    F_k(rod->getDOF(range.at(1))) = true;
                } else if (bearing->getBearingType() == BearingType::FloatingBearing) {
                    U(rod->getDOF(range.at(0))) = 0; // y-direction immoveable
                    U_k(rod->getDOF(range.at(0))) = true;
                    F(rod->getDOF(range.at(2))) = 0; // x-direction force-free
                    F_k(rod->getDOF(range.at(2))) = true;
                    F(rod->getDOF(range.at(1))) = 0; // z-direction moment-free
                    F_k(rod->getDOF(range.at(1))) = true;
                } else { // fixed clamping
                    U(rod->getDOF(range.at(0))) = 0; // y-direction immoveable
                    U_k(rod->getDOF(range.at(0))) = true;
                    U(rod->getDOF(range.at(2))) = 0; // x-direction immoveable
                    U_k(rod->getDOF(range.at(2))) = true;
                    U(rod->getDOF(range.at(1))) = 0; // z-direction not rotable
                    U_k(rod->getDOF(range.at(1))) = true;
                }
            } else { // if there is no bearing on a node account for applied forces (because SUM(F_node_i) = F_node_applied (or 0 if no force is applied))
                Eigen::Vector2d force = node->getResultingAppliedForce(); // returns the applied force
                F(rod->getDOF(range.at(2))) = force(0); // x, positive axis direction is to the right (in calculator and in system definition)
                F_k(rod->getDOF(range.at(2))) = true; // mark that the value for F_node_i is known now
                F(rod->getDOF(range.at(0))) = - force(1); // y, positive axis direction in calculator is downwards, in system definition it is upwards, therefore -
                F_k(rod->getDOF(range.at(0))) = true;
                F(rod->getDOF(range.at(1))) = node->getResultingAppliedMoment(); // z, positive moment turns counterclockwise (same in calculator and in system definition)
                F_k(rod->getDOF(range.at(1))) = true;
            }
        }
    }
    return "";
}

QString Calculator::solveSystemOfEquations(int dofCount, const Eigen::VectorXb &F_k, const Eigen::VectorXb &U_k, const Eigen::MatrixXd &K,
                                           Eigen::VectorXd &F, Eigen::VectorXd &U)
{
    int knownFs = 0; // count number of knowns
    int knownUs = 0;
    for (int j = 0; j < dofCount; j++) {
        if (F_k(j) == true) {
            knownFs++;
        }
        if (U_k(j) == true) {
            knownUs++;
        }
    }
    if (knownFs + knownUs != dofCount) {
        return "knownFs + knownUs has to equal the dofCount!";
    }
    int knownFIndex = 0; // sort knowns/unknowns
    int unknownFIndex = 0;
    Eigen::VectorXd F_a(knownFs); // known values of F (index a: F known, U unknown; index b: U known, F unknown)
    Eigen::MatrixXd K_tmp(dofCount, dofCount);
    for (int i = 0; i < dofCount; i++) { // if a row in F is manipulated, the same manipulation has to be applied to the same row in K
        if (F_k(i) == true) {
            F_a(knownFIndex) = F(i);
            K_tmp.row(knownFIndex) = K.row(i);
            knownFIndex++;
        } else {
            //F_b(unknownFIndex) = F(i); // line kept for clarity but it would have no effect because the values in F_b are unknowns and get overwritten later
            K_tmp.row(knownFs + unknownFIndex) = K.row(i);
            unknownFIndex++;
        }
    }
    int knownUIndex = 0;
    int unknownUIndex = 0;
    Eigen::MatrixXd K_tmp2(dofCount, dofCount);
    Eigen::VectorXd U_b = Eigen::VectorXd::Zero(knownUs); // known values of U
    for (int i = 0; i < dofCount; i++) { // if a row in U is manipulated, the same manipulation has to be applied to the col with the same number in K
        if (U_k(i) == true) {
            U_b(knownUIndex) = U(i);
            K_tmp2.col(dofCount - knownUs + knownUIndex) = K_tmp.col(i); // dofCount - knownUs means unknownUs, so the arg is unknownUs + knownUIndex
            knownUIndex++;
        } else {
            //U_a(unknownUIndex) = U(i); // same as above for F_b
            K_tmp2.col(unknownUIndex) = K_tmp.col(i);
            unknownUIndex++;
        }
    }
    Eigen::MatrixXd K_aa = K_tmp2.topLeftCorner(knownFs, dofCount - knownUs); // split K, K_aa contains the known Fs and unknown Us
    Eigen::MatrixXd K_ab = K_tmp2.topRightCorner(knownFs, knownUs); // K_ab: known Fs and known Us
    Eigen::MatrixXd K_ba = K_tmp2.bottomLeftCorner(dofCount - knownFs, dofCount - knownUs); // K_ba: unknown Fs and unknown Us
    Eigen::MatrixXd K_bb = K_tmp2.bottomRightCorner(dofCount - knownFs, knownUs); // K_bb: unknown Fs and known Us
    // the system has the form K * U = F, which is now split into known- and unknown-vectors:
    //      (K_aa, K_ab,  *  (U_a,  =  (F_a,
    //       K_ba, K_bb)      U_b)      F_b)
    Eigen::FullPivLU<Eigen::MatrixXd> lu_decomp_of_K_aa(K_aa); // use LU-decomposition to determine if K_aa is invertible (square and determinant != 0)
    Eigen::VectorXd U_a = Eigen::VectorXd::Zero(dofCount - knownUs); // init U_a with 0s to set the size
    // firstly, solve first row for unknown Us
    if (lu_decomp_of_K_aa.isInvertible()) {
        U_a = lu_decomp_of_K_aa.inverse() * (F_a - K_ab * U_b);
    } else { // if K_aa is not invertible, use the pseudo-inverse to compute U_a
        Eigen::CompleteOrthogonalDecomposition<Eigen::MatrixXd> cod_of_K_aa(K_aa);
        U_a = cod_of_K_aa.solve(F_a - K_ab * U_b);
        // todo: check if pseudoinverse * matrix almost equals the identity matrix
        //return "K_aa has to be invertible!"; // this should only be reached in the unlikely event that K_aa's determinant is 0 by chance
    }
    // then solve second row for unknown Fs, using the Us calculated above
    Eigen::VectorXd F_b = K_ba * U_a + K_bb * U_b;
    for (int i = 0, j = 0; i < U_k.size(); i++) { // put the calculated values for the unknowns back into the U vector at the right position
         if (U_k(i) == false) {
             U(i) = U_a(j); // the relative order of the unknowns in U and U_a is the same
             j++;
         }
    }
    for (int i = 0, j = 0; i < F_k.size(); i++) { // same for F
        if (F_k(i) == false) {
            F(i) = F_b(j);
            j++;
        }
    }
    return "";
}

QString Calculator::applyResults(GraphicsScene *scene, const QList<Rod *> &rods, const Eigen::VectorXd &F, const Eigen::VectorXd &U)
{
    for (auto rod : rods) {
        for (int i = 0; i < 6; i++) {
            if (i == 0 || i == 2) {
                rod->setU(i, Utilities::setAlmostZeroToZero(- U(rod->getDOF(i)))); // - because the y-axis is upwards positive in display (but downwards in calculation)
            } else {
                rod->setU(i, Utilities::setAlmostZeroToZero(U(rod->getDOF(i))));
            }
        }
        for (int n = 0; n < 2; n++) { // apply displacements and forces to the nodes
            Node *node = n == 0 ? rod->getNode1() : rod->getNode2();
            QVector<int> range = n == 0 ? QVector<int>{0, 1, 4} : QVector<int>{2, 3, 5};
            node->setFx(Utilities::setAlmostZeroToZero(F(rod->getDOF(range.at(2)))));
            node->setFy(Utilities::setAlmostZeroToZero(- F(rod->getDOF(range.at(0)))));
            node->setMz(Utilities::setAlmostZeroToZero(F(rod->getDOF(range.at(1)))));
        }
    }
    double maxAbsN = 0; // max absolute normal force, needed for coloring of the rods
    double maxAbsU = 0; // max absolute displacement, needed for drawing the deformations of the rods
    double step = static_cast<GraphicsScene *>(rods.first()->scene())->getDisplacementCalculationStep(); // as percentage of the length of the rod
    for (auto rod : rods) {
        double angle = rod->getAngle(); // calculate the inner-normal-forces of the rods depending on the displacements of the connected nodes
        double u1_e_local = rod->getNode1()->getUx() * cos(angle) + rod->getNode1()->getUy() * sin(angle);
        double u2_e_local = rod->getNode2()->getUx() * cos(angle) + rod->getNode2()->getUy() * sin(angle);
        double deltaU = u2_e_local - u1_e_local; // u2 - u1 because then deltaU is positive in case of rod-elongation (which means the force is pulling)
        double f_e_local = Utilities::setAlmostZeroToZero(rod->getEA() * deltaU / rod->getLength()); // sigma=E*epsilon, sigma=F/a and epsilon=deltaU/l => F=E*A*deltaU/l
        rod->setInnerForce(f_e_local);
        if (fabs(f_e_local) > maxAbsN) { // if this inner normal force is less (greater) than the current min(max)-rod-force, make this the new minimum(maximum)-value
            maxAbsN = fabs(f_e_local);
        }
        if (static_cast<MainWindow *>(scene->parent())->getDrawDeformedSystem()) {
            if (auto curved = dynamic_cast<CurvedBeam *>(rod)) {
                double d = curved->getMaxArcDisplacement();
                if (d > maxAbsU) {
                    maxAbsU = d;
                }
            } else {
                for (int t = 0; t <= 100; t += step) {
                    if (fabs(rod->uGraph(t / 100.0 * rod->getLength())) > maxAbsU) {
                        maxAbsU = fabs(rod->uGraph(t / 100.0 * rod->getLength()));
                    }
                    if (fabs(rod->wGraph(t / 100.0 * rod->getLength())) > maxAbsU) {
                        maxAbsU = fabs(rod->wGraph(t / 100.0 * rod->getLength()));
                    }
                }
            }
        }
    }
    rods.first()->setColorMapValues(- maxAbsN, maxAbsN); // save values for the color-map
    rods.first()->setMaxDisplacement(maxAbsU); // save value for drawing the displacements
    return "";
}

void Calculator::cleanUp(GraphicsScene *scene) // reset all indizes back to the invalid starting values because calculate() needs all ids to be -1 at the beginning
{
    for (auto element : scene->items()) { // loop through all elements
        if (auto rod = dynamic_cast<Rod *>(element)) {
            rod->setCalcId(-1); // set rod-id back to -1
            for (int i = 0; i < 6; i++) { // set all dof-ids back to -1
                rod->setDOF(i, -1);
                rod->setU(i, 0); // set all displacements to 0 (otherwise the obsolete displacements would be drawn)
                rod->setElementTransformationMatrix(Eigen::Matrix6d::Identity(6, 6));
            }
        } else if (auto node = dynamic_cast<Node *>(element)) {
            node->setCalcId(-1); // set all node-ids back to -1
        }
    }
}
