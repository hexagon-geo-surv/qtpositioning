/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtLocation module of the Qt Toolkit.
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
#include "mapsphere_p.h"

#include "tilecache.h"
#include "tilespec.h"
#include "tile.h"
#include "map.h"
#include "map_p.h"

#include "qgeomappingmanager.h"

#include <QOpenGLFramebufferObject>

#include <Qt3D/qglscenenode.h>
#include <Qt3D/qglframebufferobjectsurface.h>
#include <Qt3D/qglcamera.h>
#include <Qt3D/qglpainter.h>

#include <QVector>
#include <QTimer>

MapSphere::MapSphere(Map* map, MapPrivate *mapPrivate, TileCache *tileCache)
    : QObject(0),
      tileCache_(tileCache),
      map_(map),
      mapPrivate_(mapPrivate),
      manager_(0)
{

    sphereNode_ = new QGLSceneNode(this);

    connect(this,
            SIGNAL(tileUpdated()),
            map,
            SIGNAL(updateRequired()));
}

MapSphere::~MapSphere()
{
    QList<QGLSceneNode*> nodes = built_.values();
    for (int i = 0; i < nodes.size(); ++i) {
        QGLSceneNode *node = nodes.at(i);
        //node->material()->texture()->release();
        node->geometry().clear();
        sphereNode_->removeNode(node);
        delete node;
    }
}

void MapSphere::setMappingManager(QGeoMappingManager *manager)
{
    manager_ = manager;
}

QGLSceneNode* MapSphere::sphereSceneNode() const
{
    return sphereNode_;
}

// Function to perform housekeeping that require access to GL context
// (access to GL context varies depending on if we are running as
// c++ app or QML app).
//
// Function is guaranteed to be only called when executing in
// rendering thread with valid GL context. Furthermore it is
// safe to update any geometry/structures - mutex is locked.
void MapSphere::GLContextAvailable()
{
    // need something like this in the cache for
    // releasing textures and freeing nodes that
    // have been evicted from the GPU
    tileCache_->GLContextAvailable(sphereNode_);
//    qDeleteAll(obsoleteNodes_);
//    obsoleteNodes_.clear();
}

void MapSphere::update(const QList<TileSpec> &tiles)
{
    QHash<TileSpec, QGLSceneNode*> stillBuilt;

    QVector<TileSpec> req(tiles.size());
    QVector<TileSpec> draw(tiles.size());

    QSet<TileSpec> cancelTiles = requested_ - tiles.toSet();

    int reqSize = 0;
    int drawSize = 0;
    QList<TileSpec>::const_iterator i = tiles.constBegin();
    QList<TileSpec>::const_iterator tilesEnd = tiles.constEnd();
    while (i != tilesEnd) {
        /*
          If the tile is already built or has been requested then we
          shouldn't use it (where "use" means drawing a cached tile or
          requested an uncached tile).
        */
        if (built_.contains(*i)) {
            stillBuilt.insert(*i, built_.value(*i));
        } else if (!requested_.contains(*i)){
            /*
              Otherwise we add it to the correct list
            */
            if (tileCache_->contains(*i)) {
                draw[drawSize] = *i;
                ++drawSize;
            } else {
                req[reqSize] = *i;
                ++reqSize;
                requested_.insert(*i);
            }
        }

        ++i;
    }

    req.resize(reqSize);

    updateMutex.lock();

    QHash<TileSpec, QGLSceneNode*>::const_iterator j = built_.constBegin();
    QHash<TileSpec, QGLSceneNode*>::const_iterator end = built_.constEnd();
    while (j != end) {
        TileSpec spec = j.key();
        if (!stillBuilt.contains(spec)) {
            sphereNode_->removeNode(j.value());
        }
        ++j;
    }
    built_.swap(stillBuilt);

    updateMutex.unlock();

    for (int i = 0; i < drawSize; ++i)
            displayTile(draw.at(i));

    if (req.isEmpty()) {
        emit tileUpdated();
    } else {
        if (manager_) {
            manager_->updateTileRequests(map_, req.toList().toSet(), cancelTiles);
        }
    }
}

void MapSphere::tileFetched(const TileSpec &spec)
{
    if (!requested_.contains(spec))
        return;

    displayTile(spec);

    requested_.remove(spec);

    emit tileUpdated();
}

void MapSphere::displayTile(const TileSpec &spec)
{
    if (built_.contains(spec))
        return;

    updateMutex.lock();
    Tile tile = tileCache_->get(spec);
    QGLSceneNode *node = tile.sceneNode();
    if (!node) {
        node = mapPrivate_->createTileNode(tile);
        tile.setSceneNode(node);
        tile.bind();
        tileCache_->update(spec, tile);
    }
    if (node) {
        sphereNode_->addNode(node);
        built_.insert(spec, node);
    }
    updateMutex.unlock();
}

void MapSphere::paintGL(QGLPainter *painter)
{
    if (!updateMutex.tryLock()) {
        qWarning() << "----- map will miss a frame, no mutex acquired!------";
        return;
    }

    GLContextAvailable();

    sphereNode_->draw(painter);

    updateMutex.unlock();
}
