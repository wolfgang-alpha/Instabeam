#ifndef TRUSSELEMENT_H
#define TRUSSELEMENT_H

#include "color.h"
#include "utilities.h"

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

class Color;
class EasyChangeDialog;

enum class ElementType {
    Node = 0,
    Rod = 1,
    Bearing = 2,
    SingleForce = 3,
    Label = 4,
    Dimension = 5,
    CurvedBeam = 6
};

enum ElementZValue { // no enum class to reduce the amount of type-casting
    Dimension_ = 0,
    Bearing_ = 50,
    Rod_ = 100,
    Node_ = 150,
    SingleForce_ = 200,
    Label_ = 300,
    // the following two z-values indicate that the element is hovering/selected; the values are added to the default-z-values
    Hovered = 1000, // has to be greater than the absolute difference between the lowest default-z-value and the highest default-z-value
    Selected = 10000 // has to be greater than the absolute difference between the lowest default-z-value and the highest default-z-value + the Hovered-z-value
};

template<typename T>
class Id
{
public:
    Id() : id(generateId()) {} // set id to a new value
    ~Id() { usedIds.removeOne(id); } // free id in dtor

    QString toString() const { return QString::number(id); } // return id as string

    void resetId(const QString &newIdString) // replaces the current id with the new id passed to this fct as string (newId mustn't be used already (except by this id-object)!)
    {
        if (id == newIdString.toUInt()) {
            return; // the current id is also the new id, therefore do nothing
        }
        usedIds.removeOne(id); // unregister current id
        if (usedIds.contains(newIdString.toUInt())) {
            qFatal("error 1 in Id(QString): id is used already");
        } else {
            id = newIdString.toUInt(); // assign new id
            Utilities::insertSorted(id, usedIds); // register new id to prevent double-assignment of the same id
        }
    }

private:
    unsigned long generateId()
    {
        unsigned long newId = 1;
        while (usedIds.contains(newId)) { // find first free id (usedIds does not contain it)
            newId++;
        }
        usedIds.append(newId); // mark the new id as used
        return newId;
    }

    unsigned long id;
    static QVector<unsigned long> usedIds;
};

template<typename T>
QVector<unsigned long> Id<T>::usedIds;

class TrussElement : public QGraphicsObject
{
    Q_OBJECT

public:
    TrussElement(double xPosition, double yPosition, TrussElement *parentTrussElement = nullptr);
    ~TrussElement() override;
    // disable copying/moving (implement later if necessary)
    TrussElement(const TrussElement &) = delete;
    TrussElement(TrussElement &&) = delete;
    TrussElement &operator =(const TrussElement &) = delete;
    TrussElement &operator =(TrussElement &&) = delete;

    virtual void setXPos(double newXPos); // sets the x-position of the item to newXPos WITHOUT updating the easychange-dialog
    virtual void setYPos(double newYPos); // sets the x-position of the item to newXPos WITHOUT updating the easychange-dialog
    virtual void setPosition(QPointF newPosition); // sets the position of the item to newPosition WITHOUT updating the easychange-dialog
    virtual void setPositionAndUpdateDialog(QPointF newPosition); // sets the position of the item to newPosition WITH UPDATING the easychange-dialog

    void setIsMoving(bool on);

    void setIsInSelection(bool on);
    bool getIsInSelection() const { return isInSelection; }

    void setIsUnderHoverAction(bool on);
    bool getIsUnderHoverAction() const { return isUnderHoverAction; }

    virtual EasyChangeDialog *createEasyChangeDialog() = 0; // not const because otherwise the this-ptr would also be const
    void showEasyChangeDialog();
    void closeEasyChangeDialog();
    void updateEasyChangeDialog();
    bool easyChangeDialogIsShown() const;

    Color getHoverPenColor() const { return hoverPenColor; }
    Color getSelectionPenColor() const { return selectionPenColor; }

    virtual QString getElementSelectionDialogButtonText() const = 0;

    virtual QString getId() const = 0;

    virtual QJsonObject saveAsJson() const;
    virtual void loadFromJson(const QJsonObject &object);

protected:
    Color hoverPenColor;
    Color selectionPenColor;
    bool isMoving;
    bool isInSelection;
    bool isUnderHoverAction;
    static EasyChangeDialog *easyChangeDialog;

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override = 0;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override = 0;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

#endif // TRUSSELEMENT_H
