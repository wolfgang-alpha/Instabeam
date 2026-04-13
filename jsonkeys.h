#ifndef JSONKEYS_H
#define JSONKEYS_H

#include <QString>

namespace JsonKeys { // there is no .cpp-file of this .h-file because it only defines the json-keys used for saving and reloading the truss
// as long as the variable-name equals exactly the string it contains the compiler will create errors if one tries to define a variable that already exists, therefore this system is
// preventing hard to find bugs when the same string is used to describe multiple keys
    const QString version = "version";
    const QString graphicsView = "graphicsView";
    const QString minSceneHeight = "minSceneHeight";
    const QString minSceneWidth = "minSceneWidth";
    const QString graphicsScene = "graphicsScene";
    const QString items = "items";
    const QString x = "x";
    const QString y = "y";
    const QString zValue = "zValue";
    const QString parentItem = "parentItem";
    const QString thisItem = "thisItem";
    const QString elementType = "elementType";
    const QString bearingType = "bearingType";
    const QString angle = "angle";
    const QString reactionForceX = "reactionForceX";
    const QString reactionForceY = "reactionForceY";
    const QString id = "id";
    const QString value = "value";
    const QString text = "text";
    const QString nodeType = "nodeType";
    const QString rods = "rods";
    const QString label = "label";
    const QString node1 = "node1";
    const QString node2 = "node2";
    const QString force = "rodForce";
    const QString orientation = "orientation";
    const QString baselinePos = "baselinePos";
    const QString radius = "radius";
} // end namespace JsonKeys

#endif // JSONKEYS_H
