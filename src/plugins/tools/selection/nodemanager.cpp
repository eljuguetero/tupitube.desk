/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "nodemanager.h"
#include "tupgraphicobject.h"

NodeManager::NodeManager(QGraphicsItem *parentItem, QGraphicsScene *gScene, int zValue)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::NodeManager()]";
    #endif

    parent = parentItem;
    scene = gScene;
    pressed = false;

    rotationValue = parent->data(TupGraphicObject::Rotate).toReal();
    scaleX = parent->data(TupGraphicObject::ScaleX).toReal();
    if (scaleX == 0.0)
        scaleX = 1;
    scaleY = parent->data(TupGraphicObject::ScaleY).toReal();
    if (scaleY == 0.0)
        scaleY = 1;

    // This condition is only for SVG objects
    if (qgraphicsitem_cast<QGraphicsSvgItem *> (parentItem)) {
        if (static_cast<int> (scaleX) == 0) {
            scaleX = 1;
            parent->setData(TupGraphicObject::ScaleX, 1);
        }
        if (static_cast<int> (scaleY) == 0) {
            scaleY = 1;
            parent->setData(TupGraphicObject::ScaleY, 1);
        }
    } 

    QRectF rect = parentItem->sceneBoundingRect();
    Node *topLeft = new Node(Node::TopLeft, Node::Scale, rect.topLeft(), this, parentItem, zValue);
    Node *topRight = new Node(Node::TopRight, Node::Scale, rect.topRight(), this, parentItem, zValue);
    Node *bottomLeft = new Node(Node::BottomLeft, Node::Scale, rect.bottomLeft(), this, parentItem, zValue);
    Node *bottomRight = new Node(Node::BottomRight, Node::Scale, rect.bottomRight(), this, parentItem, zValue);
    Node *center = new Node(Node::Center, Node::Scale, rect.center(), this, parentItem, zValue);

    nodes.insert(Node::TopLeft, topLeft);
    nodes.insert(Node::TopRight, topRight);
    nodes.insert(Node::BottomLeft, bottomLeft);
    nodes.insert(Node::BottomRight, bottomRight);
    nodes.insert(Node::Center, center);

    proportional = false;
    
    beginToEdit();
}

NodeManager::~NodeManager()
{
}

void NodeManager::clear()
{
    foreach (Node *node, nodes) {
        if (node) {
            QGraphicsScene *scene = node->scene();
            if (scene)
                scene->removeItem(node);
        }
        delete node;
        node = 0;
    }
    nodes.clear();
}

void NodeManager::syncNodes(const QRectF &rect)
{
    if (nodes.isEmpty())
        return;
    
    QHash<Node::NodeType, Node *>::iterator it = nodes.begin();
    while (it != nodes.end()) {
        if ((*it)) {
            switch (it.key()) {
                case Node::TopLeft:
                {
                    if ((*it)->scenePos() != rect.topLeft())
                        (*it)->setPos(rect.topLeft());
                    break;
                }
                case Node::TopRight:
                {
                    if ((*it)->scenePos() != rect.topRight())
                        (*it)->setPos(rect.topRight());
                    break;
                }
                case Node::BottomRight:
                {
                    if ((*it)->scenePos() != rect.bottomRight())
                        (*it)->setPos(rect.bottomRight());
                    break;
                }
                case Node::BottomLeft:
                {
                    if ((*it)->scenePos() != rect.bottomLeft())
                        (*it)->setPos(rect.bottomLeft());
                    break;
                }
                case Node::Center:
                {
                    if ((*it)->scenePos() != rect.center())
                        (*it)->setPos(rect.center());
                    break;
                }
            }
        }
        ++it;
    }
}

void NodeManager::syncNodesFromParent()
{
    if (parent)
        syncNodes(parent->sceneBoundingRect());
}

QGraphicsItem *NodeManager::parentItem() const
{
    return parent;
}

bool NodeManager::isModified() const
{
    return !((parent->transform() == origTransform) && (parent->pos() == origPos));
}

void NodeManager::beginToEdit()
{
    origTransform = parent->transform();
    origPos = parent->pos();
}

void NodeManager::restoreItem()
{
    parent->setTransform(origTransform);
    parent->setPos(origPos);
}

void NodeManager::scale(qreal sx, qreal sy)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::scale()]";
        qWarning() << "Scale X: " << sx;
        qWarning() << "Scale Y: " << sy;
        qWarning() << "Rotation: " << rotationValue;
    #endif

    QTransform transform;
    QPointF point = parent->boundingRect().center();
    transform.translate(point.x(), point.y());
    transform.rotate(rotationValue);
    transform.scale(sx, sy);
    transform.translate(-point.x(), -point.y());

    parent->setTransform(transform);

    syncNodesFromParent();
    scaleX = sx;
    scaleY = sy;
    parent->setData(TupGraphicObject::ScaleX, scaleX);
    parent->setData(TupGraphicObject::ScaleY, scaleY);

    emit scaleUpdated(sx, sy);
}

void NodeManager::rotate(double angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[NodeManager::rotate()]";
        qWarning() << "New angle: " << angle;
    #endif

    QTransform transform;
    QPointF point = parent->boundingRect().center();
    transform.translate(point.x(), point.y());
    transform.rotate(angle);
    transform.scale(scaleX, scaleY);
    transform.translate(-point.x(), -point.y());

    parent->setTransformOriginPoint(point);
    parent->setTransform(transform);

    syncNodesFromParent();
    rotationValue = angle;
    parent->setData(TupGraphicObject::Rotate, rotationValue);

    emit rotationUpdated(static_cast<int>(angle));
}

void NodeManager::horizontalFlip()
{
    scale(scaleX*(-1), scaleY);
}

void NodeManager::verticalFlip()
{
    scale(scaleX, scaleY*(-1));
}

void NodeManager::crossedFlip()
{
    scale(scaleX*(-1), scaleY*(-1));
}

void NodeManager::show()
{
    foreach (Node *node, nodes) {
        if (!node->scene())
            scene->addItem(node);
    }
}

void NodeManager::setPressedStatus(bool pressedFlag)
{
    pressed = pressedFlag;
}

bool NodeManager::isPressed()
{
    return pressed;
}

void NodeManager::toggleAction()
{
    foreach (Node *node, nodes) {
        if (node->nodeAction() == Node::Scale) {
            node->setAction(Node::Rotate);
        } else if (node->nodeAction() == Node::Rotate) {
            node->setAction(Node::Scale);
        }
    }
}

void NodeManager::setActionNode(Node::NodeAction action)
{
    foreach (Node *node, nodes)
        node->setAction(action);
}

void NodeManager::resizeNodes(qreal factor)
{
    foreach (Node *node, nodes)
        node->resize(factor);
}

void NodeManager::setVisible(bool visible)
{
    foreach (Node *node, nodes)
        node->setVisible(visible);
}

double NodeManager::rotation()
{
    return rotationValue;
}

void NodeManager::setProportion(bool flag)
{
    proportional = flag;
}

bool NodeManager::proportionalScale()
{
    return proportional;
}
