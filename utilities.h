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

#ifndef UTILITIES_H
#define UTILITIES_H

#include "widgets/lineedit.h"

#include <cmath>
#include <QRectF>
#include <QList>
#include <QDebug>
#include <QPainterPath>

namespace Utilities
{
    bool isequal(double a, double b);

    QRectF max(const QRectF &rect1, const QRectF &rect2); // return the rect with the biggest area

    void drawArrow(QPainter *painter, double arrowWidth, double arrowHeight); // draw a filled arrow pointing to the left at the current painter-position (width and height are in [px])

    double arctan(double dy, double dx); // returns the arcus tangens of dy/dx in degrees (the return value is within the interval [0°; 360°])

    QString convertAddressToString(const void * const address, int base = 16); // returns the address of the obj to which the given ptr points to, converted with the given base

    double setAlmostZeroToZero(double value);

    template<typename T, typename U>
    QList<T> getAllElementsOfType(const QList<U> &list)
    {
        static_assert(std::is_pointer<T>::value && std::is_pointer<U>::value, "types T and U need to be pointers");
        QList<T> elementsOfTypeT;
        for (int i = 0; i < list.size(); i++) { // loop over all elements in the vector
            if (T element = dynamic_cast<T>(list.at(i))) { // if the element cannot be casted it is set to nullptr
                elementsOfTypeT.append(element); // append the element to the return-list
            }
        }
        return elementsOfTypeT;
    }

    template<typename T, typename U>
    QList<T> convertTo(const QList<U> &list)
    {
        static_assert(std::is_pointer<T>::value && std::is_pointer<U>::value, "types T and U have to be pointers and U has to be convertible to T");
        QList<T> elementsOfTypeT;
        for (U itemU : list) {
            T element = dynamic_cast<T>(itemU); // no try-catch-block around the cast because it only throws when references are getting casted which must never be the case
            if (element != nullptr) {
                elementsOfTypeT.append(element);
            } else {
                qDebug() << "error whilst converting U to T";
            }
        }
        return elementsOfTypeT;
    }

    template<typename T>
    void connectLineEditToDoneSignal(LineEdit *lineEdit, T* receivingElement, void (T::*slot)()) // this is that all lineEdits are connected to the same editing-done-signal
    {
        QObject::connect(lineEdit, &LineEdit::returnPressed, receivingElement, slot); // all line-edits apply their values only when return gets pressed
    }

    template<typename T>
    QList<T> sortListById(const QList<T> &list)
    {
        static_assert(std::is_pointer<T>::value, "type T has to be a pointer"); // getId() has to be implemented as well!
        QList<T> sorted;
        for (auto element : list) { // loop through every element in the given list
            bool inserted = false;
            for (int i = 0; i < sorted.size(); i++) { // find the first element in the sorted-list that has a greater id than the current element and insert the element before it
                if (element->getId().toInt() < sorted.at(i)->getId().toInt()) {
                    sorted.insert(i, element);
                    inserted = true;
                    break;
                }
            }
            if (!inserted) { // if there is no element with a greater id, insert the current element at the last position
                sorted.append(element);
            }
        }
        return sorted;
    }

    template<typename T>
    void insertSorted(T element, QVector<T> &vector) // v has to be sorted ascending
    {
        for (int i = 0; i < vector.size(); i++) {
            if (element < vector.at(i)) {
                vector.insert(i, element);
                break;
            }
        }
    }

} // end namespace Utilities

#endif // UTILITIES_H
