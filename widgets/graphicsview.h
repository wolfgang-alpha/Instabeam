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

#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

class GraphicsScene;
class ElementSelectionDialog;
class MainWindow;

enum class MouseBoundary {
    Left = 0,
    Right = 1,
    Top = 2,
    Bottom = 3
};

class GraphicsView final : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsView(QWidget *parent = nullptr);
    ~GraphicsView() override;
    // since it is not needed to copy/move the view it is not necessary to implement the corresponding fcts
    GraphicsView(const GraphicsView &) = delete;
    GraphicsView(GraphicsView &&) = delete;
    GraphicsView &operator =(const GraphicsView &) = delete;
    GraphicsView &operator =(GraphicsView &&) = delete;

    GraphicsScene *getGraphicsScene() const; // returns weak ptr

    void resizeSceneRect();

    void toggleNodeAdder(bool checked);
    void toggleRodAdder(bool checked);
    void toggleCurvedBeamAdder(bool checked);
    void toggleBearingAdder(bool checked);
    void toggleForceAdder(bool checked);
    void toggleDimensionAdder(bool checked);
    void toggleLabelAdder(bool checked);

    void forwardKeyPressEvent(QKeyEvent *event) { keyPressEvent(event); } // make it possible that an other widget passes key-events to the graphcis-view
    void forwardKeyReleaseEvent(QKeyEvent *event) { keyReleaseEvent(event); }

    void closeEasyChangeDialog();

    double getMinSceneWidth() { return minSceneWidth; }
    void setMinSceneWidth(double newMinSceneWidth);
    double getMinSceneHeight() { return minSceneHeight; }
    void setMinSceneHeight(double newMinSceneHeight);

    QJsonObject saveAsJson() const;
    void loadFromJson(const QJsonObject &object, MainWindow *parentForScene);

private:
    bool ctrlPressed;
    ElementSelectionDialog *elementSelectionDialog; // this class owns the ptr
    double minSceneWidth; // [px]
    double minSceneHeight; // [px]
    bool mouseClickedInBackground; // true if the user clicks in an empty scene space

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // GRAPHICSVIEW_H
