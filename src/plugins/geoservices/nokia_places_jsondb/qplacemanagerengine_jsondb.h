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

#ifndef QPLACEMANAGERENGINE_NOKIA_JSONDB_H
#define QPLACEMANAGERENGINE_NOKIA_JSONDB_H

#include <qplacemanagerengine.h>
#include <qgeoserviceprovider.h>

#include "jsondbhandler.h"

QT_USE_NAMESPACE

class QPlaceManagerEngineJsonDb : public QPlaceManagerEngine
{
    Q_OBJECT
public:

    QPlaceManagerEngineJsonDb(const QMap<QString, QVariant> &parameters, QGeoServiceProvider::Error *error,
                                                                        QString *errorString);
    ~QPlaceManagerEngineJsonDb();

    QPlaceDetailsReply *getPlaceDetails(const QString &placeId);

    QPlaceContentReply *getContent(const QPlace &place, const QPlaceContentRequest &request);

    QPlaceSearchReply *search(const QPlaceSearchRequest &request);

    QPlaceSearchReply *recommendations(const QPlace &place, const QPlaceSearchRequest &request);
    QPlaceTextPredictionReply *textPredictions(const QPlaceSearchRequest &request);

    QPlaceIdReply *savePlace(const QPlace &place);
    QPlaceIdReply *removePlace(const QPlace &place);

    QPlaceIdReply *saveCategory(const QPlaceCategory &category, const QString &parentId);
    QPlaceIdReply *removeCategory(const QString &categoryId);

    QPlaceReply *initializeCategories();
    QString parentCategoryId(const QString &categoryId) const {return QString();}
    QStringList childrenCategoryIds(const QString &categoryId) const {return QStringList();}
    QPlaceCategory category(const QString &categoryId) const {return QPlaceCategory();}

    QList<QPlaceCategory> childCategories(const QString &parentId) const;

    QLocale locale() const;
    void setLocale(const QLocale &locale);

    QUrl constructIconUrl(const QPlaceIcon &icon, const QSize &size, QPlaceIcon::IconFlags flags);
public slots:
    void processJsonDbResponse(int id, const QVariant &data);
    void processJsonDbError(int id, int code, const QString &data);

private:
    void recursiveRemoveHelper(const QString &categoryId, const QString &parentId);
    mutable JsonDbHandler m_jsonDbHandler;
    mutable QMap<int, QPlaceReply *> m_idReplyMap;
    friend class JsonDbHandler;
    friend class SaveReply;
    friend class MediaReply;
    friend class RemoveReply;
    friend class TextPredictionReply;
    friend class ReviewReply;
    friend class Reply;
    friend class SearchReply;
};

#endif
