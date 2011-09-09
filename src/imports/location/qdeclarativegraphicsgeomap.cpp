/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativegraphicsgeomap_p.h"
#include "qdeclarativegeomapmousearea_p.h"

#include "qdeclarativecoordinate_p.h"
#include "qdeclarativegeoserviceprovider_p.h"

#include <qgeoserviceprovider.h>
#include <qgeomappingmanager.h>

#include <QGraphicsSceneMouseEvent>
#include <QDeclarativeContext>
#include <QtDeclarative/qdeclarativeinfo.h>
#include <QModelIndex>

#include <QDebug>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass Map

    \brief The Map element displays a map.
    \inherits QSGItem

    \ingroup qml-location-maps

    The Map element can be used be used to display a map of the world.  The
    bulk of the functionality is provided by a mapping plugin described
    by the Plugin element associated with the Map.

    Various map objects can be added to the map.  These map objects are
    specified in terms of coordinates and metres.

    MapObjects can be directly added to the Map element and it will display them
    automatically. The various objects that can be added include:

    \list
    \o MapRectangle
    \o MapCircle
    \o MapText
    \o MapImage
    \o MapPolygon
    \o MapPolyline
    \o MapGroup
    \o MapObjectView
    \endlist

    Of the above list, MapObjectView is a special case and not a MapObject as such.
    Here is a small example to illustrate this:

    \snippet doc/src/snippets/declarative/declarative-map.qml Basic MapObjects and View on Map

    Mouse handling is done by adding MapMouseArea items as children of either
    MapObjects or the Map item itself.

    The Map element is part of the \bold{QtMobility.location 1.2} module.
*/

QDeclarativeGraphicsGeoMap::QDeclarativeGraphicsGeoMap(QSGItem *parent)
    : QSGItem(parent),
      plugin_(0),
//      mapData_(0),
      center_(0),
      initialCoordinate(0),
//      mapType_(NoMap),
//      connectivityMode_(NoConnectivity),
      componentCompleted_(false)
{
    initialCoordinate = new QGeoCoordinate(-27.0, 153.0);
    zoomLevel_ = 8;
    size_ = QSizeF(100.0, 100.0);
    //setAcceptsHoverEvents(true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MidButton | Qt::RightButton);
    setFlag(QSGItem::ItemHasContents, true);
    //setFlag(QGraphicsItem::ItemHasNoContents, false);
    //setFlag(QGraphicsItem::ItemAcceptsInputMethod);
}

QDeclarativeGraphicsGeoMap::~QDeclarativeGraphicsGeoMap()
{
    QList<QDeclarativeGeoMapObject*> objects = mapObjects_;
    // Remove all map objects (map does not manage their allocation)
    for (int i = 0; i < objects.count(); ++i) {
        removeMapObject(objects.at(i));
    }
    // Note: delete map object view before the map data,
    // so that it removes its map objects from the qgeomapdata,
    // otherwise qgeomapdata will delete data it doesn't own.
    if (!mapViews_.isEmpty())
        qDeleteAll(mapViews_);
    //if (mapData_)
    //    delete mapData_;
    if (initialCoordinate)
        delete initialCoordinate;
}

void QDeclarativeGraphicsGeoMap::componentComplete()
{
    qmlInfo(this) << tr("!WARNING! Current Map element is undergoing Qt5 (QML2) changes.");
    qmlInfo(this) << tr("!WARNING! In its current state, the Map is not usable as a visual element.");
    componentCompleted_ = true;
    QSGItem::componentComplete();
    populateMap();
}

void QDeclarativeGraphicsGeoMap::recursiveAddToObjectMap(QDeclarativeGeoMapObject *mapObject)
{
#if 0
    objectMap_.insert(mapObject->mapObject(), mapObject);

    QDeclarativeGeoMapGroupObject *groupObject =
        qobject_cast<QDeclarativeGeoMapGroupObject *>(mapObject);

    if (groupObject) {
        QDeclarativeListReference ref(groupObject, "objects");
        for (int i = 0; i < ref.count(); ++i) {
            QDeclarativeGeoMapObject *subObject =
                qobject_cast<QDeclarativeGeoMapObject *>(ref.at(i));

            if (subObject)
                recursiveAddToObjectMap(subObject);
        }
    }
#else
    qDebug() << Q_FUNC_INFO << "FIXME";
#endif
}

#if 0
void QDeclarativeGraphicsGeoMap::recursiveRemoveFromObjectMap(QGeoMapObject *mapObject)
{
    objectMap_.remove(mapObject);

    QGeoMapGroupObject *groupObject = qobject_cast<QGeoMapGroupObject *>(mapObject);
    if (groupObject) {
        foreach (QGeoMapObject *subObject, groupObject->childObjects())
            recursiveRemoveFromObjectMap(subObject);
    }
}
#endif

void QDeclarativeGraphicsGeoMap::populateMap()
{
//    if (!mapData_ || !componentCompleted_)
    if (!componentCompleted_)
        return;
    QObjectList kids = children();
    for (int i = 0; i < kids.size(); ++i) {
        // dispatch items appropriately
        QDeclarativeGeoMapObjectView* mapView = qobject_cast<QDeclarativeGeoMapObjectView*>(kids.at(i));
        if (mapView) {
            mapViews_.append(mapView);
            setupMapView(mapView);
            continue;
        }
        QDeclarativeGeoMapObject *mapObject = qobject_cast<QDeclarativeGeoMapObject*>(kids.at(i));
        if (mapObject) {
            mapObjects_.append(mapObject);
            //recursiveAddToObjectMap(mapObject);
            //mapData_->addMapObject(mapObject->mapObject());
            mapObject->setMap(this);
            emit objectsChanged();
            continue;
        }
        QDeclarativeGeoMapMouseArea *mouseArea
        = qobject_cast<QDeclarativeGeoMapMouseArea*>(kids.at(i));
        if (mouseArea) {
            mouseArea->setMap(this);
            mouseAreas_.append(mouseArea);
        }
    }
}

void QDeclarativeGraphicsGeoMap::setupMapView(QDeclarativeGeoMapObjectView *view)
{
    qFatal("fix the next line");
//    view->setMapData(this);
    view->repopulate();
}

void QDeclarativeGraphicsGeoMap::paint(QPainter *painter,
                                       const QStyleOptionGraphicsItem *option,
                                       QWidget * /*widget*/)
{
//    if (mapData_) {
//        mapData_->paint(painter, option);
//    }
}

void QDeclarativeGraphicsGeoMap::geometryChanged(const QRectF &newGeometry,
        const QRectF & /*oldGeometry*/)
{
    setSize(newGeometry.size());
}

/*!
    \qmlproperty Plugin Map::plugin

    This property holds the plugin which provides the mapping functionality.

    This is write-once property.  Once the map has a plugin associated with
    it any attempted modifications of the plugin will be ignored.
*/

void QDeclarativeGraphicsGeoMap::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (plugin_) {
        qmlInfo(this) << tr("Plugin is a write-once property, and cannot be set again.");
        return;
    }
    plugin_ = plugin;
    emit pluginChanged(plugin_);
    QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
    if (serviceProvider->error() != QGeoServiceProvider::NoError) {
        qWarning() << serviceProvider->errorString();
        return;
    }
    QGeoMappingManager *mappingManager = serviceProvider->mappingManager();
    if (!mappingManager || serviceProvider->error() != QGeoServiceProvider::NoError) {
        qWarning() << serviceProvider->errorString();
        return;
    }
    //mapData_ = mappingManager->createMapData();
    //mapData_->init();
    //mapData_->setParentItem(this);

//    // setters
//    mapData_->setWindowSize(size_);
//    mapData_->setZoomLevel(zoomLevel_);

//    if (center_)
//        mapData_->setCenter(center_->coordinate());
//    else
//        mapData_->setCenter(*initialCoordinate);

//    mapData_->setMapType(QGraphicsGeoMap::MapType(mapType_));
//    mapData_->setConnectivityMode(QGraphicsGeoMap::ConnectivityMode(connectivityMode_));

    // Populate the map objects.
    populateMap();
    // setup signals
//    connect(mapData_,
//            SIGNAL(updateMapDisplay(QRectF)),
//            this,
//            SLOT(updateMapDisplay(QRectF)));

//    connect(mapData_,
//            SIGNAL(centerChanged(QGeoCoordinate)),
//            this,
//            SLOT(internalCenterChanged(QGeoCoordinate)));

//    connect(mapData_,
//            SIGNAL(mapTypeChanged(QGraphicsGeoMap::MapType)),
//            this,
//            SLOT(internalMapTypeChanged(QGraphicsGeoMap::MapType)));

//    connect(mapData_,
//            SIGNAL(connectivityModeChanged(QGraphicsGeoMap::ConnectivityMode)),
//            this,
//            SLOT(internalConnectivityModeChanged(QGraphicsGeoMap::ConnectivityMode)));

//    connect(mapData_,
//            SIGNAL(windowSizeChanged(QSizeF)),
//            this,
//            SIGNAL(sizeChanged(QSizeF)));

//    connect(mapData_,
//            SIGNAL(zoomLevelChanged(qreal)),
//            this,
//            SIGNAL(zoomLevelChanged(qreal)));
}

void QDeclarativeGraphicsGeoMap::updateMapDisplay(const QRectF &target)
{
    Q_UNUSED(target);
    //update(target);
}

QDeclarativeGeoServiceProvider* QDeclarativeGraphicsGeoMap::plugin() const
{
    return plugin_;
}

/*!
    \qmlproperty qreal Map::minimumZoomLevel

    This property holds the minimum valid zoom level for the map.
*/
qreal QDeclarativeGraphicsGeoMap::minimumZoomLevel() const
{
    if (plugin_) {
        QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
        if (serviceProvider) {
            QGeoMappingManager *mappingManager = serviceProvider->mappingManager();
            if (mappingManager)
                return mappingManager->minimumZoomLevel();
        }
    }

    return -1.0;
}

/*!
    \qmlproperty qreal Map::maximumZoomLevel

    This property holds the maximum valid zoom level for the map.
*/
qreal QDeclarativeGraphicsGeoMap::maximumZoomLevel() const
{
    if (plugin_) {
        QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
        if (serviceProvider) {
            QGeoMappingManager *mappingManager = serviceProvider->mappingManager();
            if (mappingManager)
                return mappingManager->maximumZoomLevel();
        }
    }

    return -1.0;
}

// TODO make these more QML like
//QList<MapType> QDeclarativeGraphicsGeoMap::supportedMapTypes() const;
//QList<ConnectivityMode> QDeclarativeGraphicsGeoMap::supportedConnectivityModes() const;

/*!
    \qmlproperty QSizeF Map::size

    This property holds the size of the map viewport.
*/
void QDeclarativeGraphicsGeoMap::setSize(const QSizeF &size)
{
//    if (mapData_) {
//        setWidth(size.width());
//        setHeight(size.height());
//        mapData_->setWindowSize(size);
//    } else {
        if (size_ == size)
            return;

        size_ = size;

        emit sizeChanged(size_);
//    }

}

QSizeF QDeclarativeGraphicsGeoMap::size() const
{
//    if (mapData_)
//        return mapData_->windowSize();
//    else
        return size_;
}

/*!
    \qmlproperty qreal Map::zoomLevel

    This property holds the zoom level for the map.

    Larger values for the zoom level provide more detail.

    The default value is 8.0.
*/
void QDeclarativeGraphicsGeoMap::setZoomLevel(qreal zoomLevel)
{
//    if (mapData_) {
//        mapData_->setZoomLevel(zoomLevel);
//    } else {
        if (zoomLevel_ == zoomLevel)
            return;

        zoomLevel_ = zoomLevel;

        emit zoomLevelChanged(zoomLevel_);
//    }
}

qreal QDeclarativeGraphicsGeoMap::zoomLevel() const
{
//    if (mapData_) {
//        return mapData_->zoomLevel();
//    } else {
        return zoomLevel_;
//    }
}

/*!
    \qmlproperty Coordinate Map::center

    This property holds the coordinate which occupies the center of the
    mapping viewport.

    The default value is an arbitrary valid coordinate.
*/
void QDeclarativeGraphicsGeoMap::setCenter(QDeclarativeCoordinate *center)
{
    if (center_) {
        center_->disconnect(this);
    }
    center_ = center;
    if (center_) {
        connect(center_,
                SIGNAL(latitudeChanged(double)),
                this,
                SLOT(centerLatitudeChanged(double)));
        connect(center_,
                SIGNAL(longitudeChanged(double)),
                this,
                SLOT(centerLongitudeChanged(double)));
        connect(center_,
                SIGNAL(altitudeChanged(double)),
                this,
                SLOT(centerAltitudeChanged(double)));

//        if (mapData_) {
//            mapData_->setCenter(center_->coordinate());
//        }
    }
    emit declarativeCenterChanged(center_);
}

QDeclarativeCoordinate* QDeclarativeGraphicsGeoMap::center()
{
//    if (mapData_ && center_)
//        center_->setCoordinate(mapData_->center());
    return center_;
}

void QDeclarativeGraphicsGeoMap::centerLatitudeChanged(double /*latitude*/)
{
//    if (mapData_ && center_)
//        mapData_->setCenter(center_->coordinate());
}

void QDeclarativeGraphicsGeoMap::centerLongitudeChanged(double /*longitude*/)
{
//    if (mapData_ && center_)
//        mapData_->setCenter(center_->coordinate());
}

void QDeclarativeGraphicsGeoMap::centerAltitudeChanged(double /*altitude*/)
{
//    if (mapData_ && center_)
//        mapData_->setCenter(center_->coordinate());
}

/*!
    \qmlproperty enumeration Map::mapType

    This property holds the type of map to display.

    The type can be one of:
    \list
    \o Map.StreetMap
    \o Map.SatelliteMapDay
    \o Map.SatelliteMapNight
    \o Map.TerrainMap
    \endlist

    The default value is determined by the plugin.
*/
//void QDeclarativeGraphicsGeoMap::setMapType(QDeclarativeGraphicsGeoMap::MapType mapType)
//{
//    if (mapData_) {
//        mapData_->setMapType(QGraphicsGeoMap::MapType(mapType));
//    } else {
//        if (mapType_ == mapType)
//            return;

//        mapType_ = mapType;

//        emit mapTypeChanged(mapType_);
//    }
//}

//QDeclarativeGraphicsGeoMap::MapType QDeclarativeGraphicsGeoMap::mapType() const
//{
//    if (mapData_) {
//        return QDeclarativeGraphicsGeoMap::MapType(mapData_->mapType());
//    } else {
//        return mapType_;
//    }
//}

// TODO document this (list has objects managed by add/removeMapObject and
// those declared as part of the class.
QDeclarativeListProperty<QDeclarativeGeoMapObject> QDeclarativeGraphicsGeoMap::objects()
{
    return QDeclarativeListProperty<QDeclarativeGeoMapObject>(this,
                                                          0, // opaque data parameter
                                                          objects_append,
                                                          objects_count,
                                                          objects_at,
                                                          objects_clear);
}

// not supported
void QDeclarativeGraphicsGeoMap::objects_append(QDeclarativeListProperty<QDeclarativeGeoMapObject>* prop, QDeclarativeGeoMapObject* object)
{
    Q_UNUSED(prop);
    Q_UNUSED(object);
}

int QDeclarativeGraphicsGeoMap::objects_count(QDeclarativeListProperty<QDeclarativeGeoMapObject>* prop)
{
    return static_cast<QDeclarativeGraphicsGeoMap*>(prop->object)->mapObjects_.count();
}

QDeclarativeGeoMapObject* QDeclarativeGraphicsGeoMap::objects_at(QDeclarativeListProperty<QDeclarativeGeoMapObject>* prop, int index)
{
    return static_cast<QDeclarativeGraphicsGeoMap*>(prop->object)->mapObjects_.at(index);
}

// not supported
void QDeclarativeGraphicsGeoMap::objects_clear(QDeclarativeListProperty<QDeclarativeGeoMapObject>* prop)
{
    Q_UNUSED(prop);
}

/*!
    \qmlproperty enumeration Map::connectivityMode

    This property holds the connectivity mode used to fetch the map data.

    The mode can be one of:
    \list
    \o Map.OfflineMode
    \o Map.OnlineMode
    \o Map.HybridMode
    \endlist

    The default value is determined by the plugin.
*/
//void QDeclarativeGraphicsGeoMap::setConnectivityMode(QDeclarativeGraphicsGeoMap::ConnectivityMode connectivityMode)
//{
//    if (mapData_) {
//        mapData_->setConnectivityMode(QGraphicsGeoMap::ConnectivityMode(connectivityMode));
//    } else {
//        if (connectivityMode_ == connectivityMode)
//            return;

//        connectivityMode_ = connectivityMode;

//        emit connectivityModeChanged(connectivityMode_);
//    }
//}

//QDeclarativeGraphicsGeoMap::ConnectivityMode QDeclarativeGraphicsGeoMap::connectivityMode() const
//{
//    if (mapData_)
//        return QDeclarativeGraphicsGeoMap::ConnectivityMode(mapData_->connectivityMode());
//    else
//        return connectivityMode_;
//}

/*!
    \qmlproperty list<QGeoMapObject> Map::objects
    \default

    This property holds the list of objects associated with this map.

    The various objects that can be added include:
    \list
    \o MapRectangle
    \o MapCircle
    \o MapText
    \o MapImage
    \o MapPolygon
    \o MapPolyline
    \o MapGroup
    \endlist
*/

/*!
    \qmlmethod Map::toCoordinate(QPointF screenPosition)

    Returns the coordinate which corresponds to the screen position
    \a screenPosition.

    Returns an invalid coordinate if \a screenPosition is not within
    the current viewport.

*/

QDeclarativeCoordinate* QDeclarativeGraphicsGeoMap::toCoordinate(QPointF screenPosition) const
{
    QGeoCoordinate coordinate;

//    if (mapData_)
//        coordinate = mapData_->screenPositionToCoordinate(screenPosition);

    return new QDeclarativeCoordinate(coordinate,
                                      const_cast<QDeclarativeGraphicsGeoMap *>(this));
}

/*!
    \qmlmethod Map::toScreenPosition(Coordinate coordinate)

    Returns the screen position which corresponds to the coordinate
    \a coordinate.

    Returns an invalid QPointF if \a coordinate is not within the
    current viewport.
*/
QPointF QDeclarativeGraphicsGeoMap::toScreenPosition(QDeclarativeCoordinate* coordinate) const
{
    QPointF point;

//    if (mapData_)
//        point = mapData_->coordinateToScreenPosition(coordinate->coordinate());

    return point;
}

void QDeclarativeGraphicsGeoMap::pan(int dx, int dy)
{
//    if (mapData_) {
//        mapData_->pan(dx, dy);
//        update();
//    } else {
        qmlInfo(this) << tr("Map plugin is not set, cannot pan.");
//    }
}

QDeclarativeGeoMapMouseEvent* QDeclarativeGraphicsGeoMap::createMapMouseEvent(QGraphicsSceneMouseEvent *event)
{
//    if (!event || !mapData_)
        return 0;

//    QDeclarativeGeoMapMouseEvent *mouseEvent = new QDeclarativeGeoMapMouseEvent(this);

//    mouseEvent->setButton(event->button());
//    mouseEvent->setModifiers(event->modifiers());

//    QGeoCoordinate coordinate = mapData_->screenPositionToCoordinate(event->pos());
//    mouseEvent->setCoordinate(new QDeclarativeCoordinate(coordinate, this));
//    mouseEvent->setX(event->pos().x());
//    mouseEvent->setY(event->pos().y());

//    return mouseEvent;
}

QDeclarativeGeoMapMouseEvent* QDeclarativeGraphicsGeoMap::createMapMouseEvent(QGraphicsSceneHoverEvent *event)
{
//    if (!event || !mapData_)
        return 0;

//    QDeclarativeGeoMapMouseEvent *mouseEvent = new QDeclarativeGeoMapMouseEvent(this);

//    mouseEvent->setModifiers(event->modifiers());

//    QGeoCoordinate coordinate = mapData_->screenPositionToCoordinate(event->pos());
//    mouseEvent->setCoordinate(new QDeclarativeCoordinate(coordinate, this));
//    mouseEvent->setX(event->pos().x());
//    mouseEvent->setY(event->pos().y());

//    return mouseEvent;
}

void QDeclarativeGraphicsGeoMap::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
//    if (!mapData_) {
//        qmlInfo(this) << tr("Map plugin is not set, mouse event cannot be processed.");
//        return;
//    }

//    // First deliver the mouse event to possible map objects (they will accept it if
//    // they have their own mouse area). If no accepting objects, check if the map
//    // object itself has any mouse areas. This way the map objects have higher priority
//    // in mouse event handling.
//    QList<QGeoMapObject*> objects = mapData_->mapObjectsAtScreenPosition(event->pos());

//    QDeclarativeGeoMapMouseEvent *mouseEvent = createMapMouseEvent(event);

//    for (int i = objects.size() - 1; i >= 0; --i) {
//        QDeclarativeGeoMapObject* mapObject = objectMap_.value(objects.at(i), 0);
//        if (mapObject) {
//            mapObject->pressEvent(mouseEvent);
//            if (mouseEvent->accepted()) {
//                event->setAccepted(true);
//                delete mouseEvent;
//                return;
//            }
//        }
//    }

//    for (int i = 0; i < mouseAreas_.size(); ++i) {
//        mouseAreas_.at(i)->pressEvent(mouseEvent);
//        if (mouseEvent->accepted()) {
//            event->setAccepted(true);
//            delete mouseEvent;
//            return;
//        }
//    }

//    delete mouseEvent;

    //QSGItem::mousePressEvent(event);
    QSGItem::mousePressEvent(event);
}

void QDeclarativeGraphicsGeoMap::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QDeclarativeGeoMapMouseEvent *mouseEvent = createMapMouseEvent(event);

    if (activeMouseArea_)
        activeMouseArea_->releaseEvent(mouseEvent);
    activeMouseArea_ = 0;

    bool accepted = (mouseEvent->accepted());

    delete mouseEvent;

    if (accepted)
        return;

    QSGItem::mouseReleaseEvent(event);
}

void QDeclarativeGraphicsGeoMap::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
//    if (!mapData_) {
//        qmlInfo(this) << tr("Map plugin is not set, mouse event cannot be processed.");
//        return;
//    }

//    activeMouseArea_ = 0;

//    QList<QGeoMapObject*> objects = mapData_->mapObjectsAtScreenPosition(event->pos());

//    QDeclarativeGeoMapMouseEvent *mouseEvent = createMapMouseEvent(event);

//    for (int i = objects.size() - 1; i >= 0; --i) {
//        QDeclarativeGeoMapObject* mapObject = objectMap_.value(objects.at(i), 0);
//        if (mapObject) {
//            mapObject->doubleClickEvent(mouseEvent);
//            if (mouseEvent->accepted()) {
//                event->setAccepted(true);
//                delete mouseEvent;
//                return;
//            }
//        }
//    }

//    for (int i = 0; i < mouseAreas_.size(); ++i) {
//        mouseAreas_.at(i)->doubleClickEvent(mouseEvent);
//        if (mouseEvent->accepted()) {
//            event->setAccepted(true);
//            delete mouseEvent;
//            return;
//        }
//    }

//    bool accepted = (mouseEvent->accepted());

//    delete mouseEvent;

//    if (accepted)
//        return;

    QSGItem::mouseDoubleClickEvent(event);
}

void QDeclarativeGraphicsGeoMap::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QDeclarativeGeoMapMouseEvent *mouseEvent = createMapMouseEvent(event);

    if (activeMouseArea_)
        activeMouseArea_->moveEvent(mouseEvent);

    bool accepted = (mouseEvent->accepted());

    delete mouseEvent;

    if (accepted)
        return;

    QSGItem::mouseMoveEvent(event);
}

void QDeclarativeGraphicsGeoMap::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
//    qWarning() << "hover enter";
}

void QDeclarativeGraphicsGeoMap::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
//    if (!mapData_)
//        return;

//    QList<QGeoMapObject*> objectsThen = mapData_->mapObjectsAtScreenPosition(event->lastPos());
//    QList<QGeoMapObject*> objectsNow = mapData_->mapObjectsAtScreenPosition(event->pos());

//    QSet<QGeoMapObject*> enter = objectsNow.toSet();
//    enter -= objectsThen.toSet();

//    for (int i = 0; i < objectsNow.size(); ++i) {
//        if (!enter.contains(objectsNow.at(i)))
//            continue;

//        QDeclarativeGeoMapObject* mapObject = objectMap_.value(objectsNow.at(i), 0);
//        if (mapObject)
//            mapObject->enterEvent();
//    }

//    QSet<QGeoMapObject*> exit = objectsThen.toSet();
//    exit -= objectsNow.toSet();

//    for (int i = 0; i < objectsThen.size(); ++i) {
//        if (!exit.contains(objectsThen.at(i)))
//            continue;

//        QDeclarativeGeoMapObject* mapObject = objectMap_.value(objectsThen.at(i), 0);
//        if (mapObject)
//            mapObject->exitEvent();
//    }

//    QSet<QGeoMapObject*> move = objectsNow.toSet();
//    move += objectsThen.toSet();

//    QList<QGeoMapObject*> objects = mapData_->mapObjectsInViewport();

//    QDeclarativeGeoMapMouseEvent *mouseEvent = createMapMouseEvent(event);

//    for (int i = 0; i < objects.size(); ++i) {
//        if (!move.contains(objects.at(i)))
//            continue;

//        QDeclarativeGeoMapObject* mapObject = objectMap_.value(objects.at(i), 0);
//        if (mapObject)
//            mapObject->moveEvent(mouseEvent);
//    }

//    /*
//    QList<QGeoMapObject*> objects = mapData_->mapObjectsAtScreenPosition(event->pos());

//    for (int i = objects.size() - 1; i >= 0; --i) {
//        QDeclarativeGeoMapObject* mapObject = objectMap_.value(objects.at(i), 0);
//        if (mapObject)
//            mapObject->moveEvent(mouseEvent);
//    }
//    */

//    for (int i = 0; i < mouseAreas_.size(); ++i)
//        mouseAreas_.at(i)->moveEvent(mouseEvent);

//    delete mouseEvent;
}

void QDeclarativeGraphicsGeoMap::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
//    qWarning() << "hover leave";
}

void QDeclarativeGraphicsGeoMap::internalCenterChanged(const QGeoCoordinate &coordinate)
{
    emit declarativeCenterChanged(new QDeclarativeCoordinate(coordinate, this));
}

//void QDeclarativeGraphicsGeoMap::internalMapTypeChanged(QGraphicsGeoMap::MapType mapType)
//{
//    emit mapTypeChanged(QDeclarativeGraphicsGeoMap::MapType(mapType));
//}

//void QDeclarativeGraphicsGeoMap::internalConnectivityModeChanged(QGraphicsGeoMap::ConnectivityMode connectivityMode)
//{
//    emit connectivityModeChanged(QDeclarativeGraphicsGeoMap::ConnectivityMode(connectivityMode));
//}

/*!
    \qmlmethod Map::addMapObject(MapObject)

    Adds the given MapOject to the Map. If the object already
    is on the Map, it will not be added again.

    As an example, consider you have a MapCircle presenting your current position:

    \snippet doc/src/snippets/declarative/testpolymapobjects.qml Basic map position marker definition
    You can add it to Map (alterntively it can be defined as a child element of the Map):

    \snippet doc/src/snippets/declarative/testpolymapobjects.qml Basic add MapObject
    Note: MapObjectViews can not be added with this method.
*/

void QDeclarativeGraphicsGeoMap::addMapObject(QDeclarativeGeoMapObject *object)
{
//    if (!mapData_)
//        qmlInfo(this) << tr("Map plugin is not set, map object cannot be added.");
//    if (!mapData_ || !object || objectMap_.contains(object->mapObject()))
//        return;
//    mapObjects_.append(object);
//    recursiveAddToObjectMap(object);
//    mapData_->addMapObject(object->mapObject());
//    object->setMap(this);
//    emit objectsChanged();
}

/*!
    \qmlmethod Map::removeMapObject(MapObject)

    Removes the given MapObject from the Map. If the MapObject does not
    exist, function does nothing.

    As an example, consider you have a MapCircle presenting your current position:
    \snippet doc/src/snippets/declarative/testpolymapobjects.qml Basic map position marker definition

    You can remove it from the Map element:
    \snippet doc/src/snippets/declarative/testpolymapobjects.qml Basic remove MapObject


*/

void QDeclarativeGraphicsGeoMap::removeMapObject(QDeclarativeGeoMapObject *object)
{
//    if (!mapData_)
//        qmlInfo(this) << tr("Map plugin is not set, map object cannot be removed.");
//    if (!mapData_ || !object || !objectMap_.contains(object->mapObject()))
//        return;
//    objectMap_.remove(object->mapObject());
//    mapObjects_.removeOne(object);
//    mapData_->removeMapObject(object->mapObject());
//    if (!mapData_)
//        qmlInfo(this) << tr("Map plugin is not set, map object cannot be removed.");
//    if (!mapData_ || !object || !objectMap_.contains(object->mapObject()))
//        return;
//    recursiveRemoveFromObjectMap(object->mapObject());
//    mapObjects_.removeOne(object);
//    mapData_->removeMapObject(object->mapObject());
//    object->setMap(0);
//    emit objectsChanged();
}

// TODO document. Removes all map items that are added with addMapObject,
// leaves objects from model intact.
void QDeclarativeGraphicsGeoMap::clearMapObjects()
{
    //if (!mapData_ || mapObjects_.isEmpty())
    //    return;
    //QList<QDeclarativeGeoMapObject*> mapObjects = mapObjects_;
    //for (int i = 0; i < mapObjects.count(); ++ i) {
    //    if (!objectMap_.contains(mapObjects.at(i)->mapObject()))
    //        continue;
    //    recursiveRemoveFromObjectMap(mapObjects.at(i)->mapObject());
    //    mapObjects_.removeOne(mapObjects.at(i));
    //    mapData_->removeMapObject(mapObjects.at(i)->mapObject());
    //    mapObjects.at(i)->setMap(0);
    //}
    //emit objectsChanged();
}

void QDeclarativeGraphicsGeoMap::setActiveMouseArea(QDeclarativeGeoMapMouseArea *area)
{
    activeMouseArea_ = area;
}

QDeclarativeGeoMapMouseArea* QDeclarativeGraphicsGeoMap::activeMouseArea() const
{
    return activeMouseArea_;
}

// This function is strictly for testing purposes
int QDeclarativeGraphicsGeoMap::testGetDeclarativeMapObjectCount()
{
//    return objectMap_.values().count();
    return 0;
}

#include "moc_qdeclarativegraphicsgeomap_p.cpp"

QT_END_NAMESPACE
