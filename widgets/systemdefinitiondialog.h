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

#ifndef SYSTEMDEFINITIONDIALOG_H
#define SYSTEMDEFINITIONDIALOG_H

#include <QDialog>

class GraphicsScene;
class QTableView;
class QStandardItemModel;
class Node;
class Rod;
class Bearing;
class SingleForce;
class QGraphicsItem;

namespace Ui {
class SystemDefinitionDialog;
} // end namespace Ui

class SystemDefinitionDialog : public QDialog
{
    Q_OBJECT

public:
    SystemDefinitionDialog(GraphicsScene *graphicsScene, QWidget *parent);
    virtual ~SystemDefinitionDialog() override;

private:
    Ui::SystemDefinitionDialog *ui; // deleted in dtor

    void populateModel(QStandardItemModel *model, QList<Node *> list);
    void populateModel(QStandardItemModel *model, QList<Rod *> list);
    void populateModel(QStandardItemModel *model, QList<Bearing *> list);
    void populateModel(QStandardItemModel *model, QList<SingleForce *> list);

    template<typename T>
    void setupTable(QTableView *view, const QList<QGraphicsItem *> &items, const QStringList &horizontalHeaderLabels);

    // QWidget interface
private slots:
    void on_tabWidget_currentChanged(int index);
    void on_okButton_clicked();
};

#endif // SYSTEMDEFINITIONDIALOG_H
