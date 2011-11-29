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

#include "qplacemanagerengine_jsondb.h"

#include <QtCore/QDebug>

#include <QtAddOnJsonDb/jsondb-client.h>
#include <QtAddOnJsonDb/jsondb-error.h>

#include "detailsreply.h"
#include "reply.h"
#include "idreply.h"
#include "searchreply.h"
#include "unsupportedreplies.h"

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QPlaceReply::Error)
Q_DECLARE_METATYPE(QPlaceReply *)

QPlaceManagerEngineJsonDb::QPlaceManagerEngineJsonDb(const QMap<QString, QVariant> &parameters,
                                                     QGeoServiceProvider::Error *error,
                                                     QString *errorString)
:   QPlaceManagerEngine(parameters)
{
    qRegisterMetaType<QPlaceReply::Error>();
    qRegisterMetaType<QPlaceReply *>();
    qRegisterMetaType<QPlaceCategory>();

    m_db = new JsonDbClient(this);
    if (!m_db->isConnected()) {
        *error = QGeoServiceProvider::ConnectionError;
        *errorString = QLatin1String("JsonDb is not connected");
    } else {
        connect(m_db, SIGNAL(response(int,QVariant)),
                this, SLOT(processJsonDbResponse(int,QVariant)));
        connect(m_db, SIGNAL(error(int,int,QString)),
                this, SLOT(processJsonDbError(int,int,QString)));
        connect(m_db, SIGNAL(notified(QString,QVariant,QString)),
                    this, SLOT(processJsonDbNotification(QString,QVariant,QString)));

        //Setup notifications
        QVariantMap notificationMap;
        QVariantList actions;
        notificationMap.insert(JsonConverter::Type, JsonConverter::NotificationType);
        notificationMap.insert(QLatin1String("query"),
                               QString::fromLatin1("[?%1 = \"%2\" | %1 = \"%3\" ]")
                                .arg(JsonConverter::Type).arg(JsonConverter::PlaceType).arg(JsonConverter::CategoryType));
        actions.append(JsonConverter::Create);
        actions.append(JsonConverter::Update);
        actions.append(JsonConverter::Remove);
        notificationMap.insert(JsonConverter::Actions, actions);

        connect(m_db, SIGNAL(response(int,QVariant)),
                this, SLOT(setupNotificationsResponse(int,QVariant)));
        connect(m_db, SIGNAL(error(int,int,QString)),
                this, SLOT(setupNotificationsError(int,int,QString)));

        m_notificationReqId = m_db->create(notificationMap);

        *error = QGeoServiceProvider::NoError;
        errorString->clear();
    }
}

QPlaceManagerEngineJsonDb::~QPlaceManagerEngineJsonDb()
{
    delete m_db;
}

QPlaceDetailsReply *QPlaceManagerEngineJsonDb::getPlaceDetails(const QString &placeId)
{
    DetailsReply *detailsReply = new DetailsReply(this);
    detailsReply->setPlaceId(placeId);
    detailsReply->start();
    return detailsReply;
}

QPlaceContentReply *QPlaceManagerEngineJsonDb::getPlaceContent(const QString &placeId, const QPlaceContentRequest &request)
{
    Q_UNUSED(placeId)
    Q_UNUSED(request)

    ContentReply *contentReply = new ContentReply(this);
    contentReply->triggerDone(QPlaceReply::UnsupportedError, QString::fromLatin1("Fetching content is unsupported"));
    return contentReply;
}

QPlaceSearchReply *QPlaceManagerEngineJsonDb::search(const QPlaceSearchRequest &request)
{
    SearchReply *searchReply = new SearchReply(this);
    searchReply->setRequest(request);
    searchReply->start();
    return searchReply;
}

QPlaceSearchReply *QPlaceManagerEngineJsonDb::recommendations(const QString &placeId, const QPlaceSearchRequest &request)
{
    Q_UNUSED(placeId);
    SearchReply *searchReply = new SearchReply(this);
    searchReply->setRequest(request);
    searchReply->triggerDone(QPlaceReply::UnsupportedError, QString::fromLatin1("Recommendations are unsupported"));
    return searchReply;
}

QPlaceSearchSuggestionReply *QPlaceManagerEngineJsonDb::searchSuggestions(const QPlaceSearchRequest &request)
{
    Q_UNUSED(request)

    SearchSuggestionReply *searchSuggestionReply = new SearchSuggestionReply(this);
    searchSuggestionReply->triggerDone(QPlaceReply::UnsupportedError, QString::fromLatin1("Search suggestions are unsupported"));
    return searchSuggestionReply;
}

QPlaceIdReply *QPlaceManagerEngineJsonDb::savePlace(const QPlace &place)
{
    SavePlaceReply *saveReply = new SavePlaceReply(this);
    saveReply->setPlace(place);
    saveReply->start();
    return saveReply;
}

QPlaceIdReply *QPlaceManagerEngineJsonDb::saveCategory(const QPlaceCategory &category, const QString &parentId)
{
    SaveCategoryReply *reply = new SaveCategoryReply(this);
    reply->setCategory(category);
    reply->setParentId(parentId);
    reply->start();
    return reply;
}

QPlaceIdReply *QPlaceManagerEngineJsonDb::removePlace(const QString &placeId)
{
    RemovePlaceReply *removeReply = new RemovePlaceReply(this);
    removeReply->setId(placeId);
    removeReply->start();
    return removeReply;
}

QPlaceIdReply *QPlaceManagerEngineJsonDb::removeCategory(const QString &categoryId)
{
    RemoveCategoryReply *reply = new RemoveCategoryReply(this);
    reply->setId(categoryId);
    reply->start();
    return reply;
}

QPlaceReply *QPlaceManagerEngineJsonDb::initializeCategories()
{
    Reply *reply = new Reply(this);
    reply->triggerDone();
    return reply;
}

QString QPlaceManagerEngineJsonDb::parentCategoryId(const QString &categoryId) const
{
    int reqId = m_db->query(QString("[?%1=\"%2\"][?%3 = \"%4\"]").arg(JsonConverter::Type).arg(JsonConverter::CategoryType)
                                      .arg(JsonConverter::Uuid).arg(categoryId));

    QVariantMap responseMap;
    //TODO: don't spin the event loop, instead try and have all category
    //      data cached since it shouldn't be too large.
    if (!waitForRequest(reqId, &responseMap)) {
        return QString();
    }

    if (responseMap.value(JsonConverter::Length).toInt() <= 0)
        return QString();

    return responseMap.value(JsonConverter::Data).toList()
            .first().toMap().value(JsonConverter::CategoryParentId).toString();
}

QStringList QPlaceManagerEngineJsonDb::childrenCategoryIds(const QString &categoryId) const
{
    QStringList result;
    QList<QPlaceCategory> children = childCategories(categoryId);
    foreach (const QPlaceCategory &child, children)
        result.append(child.categoryId());

    return result;
}

QPlaceCategory QPlaceManagerEngineJsonDb::category(const QString &categoryId) const
{
    if (categoryId.isEmpty())
        return QPlaceCategory();

    int reqId =  m_db->query(queryCategoryString(categoryId));
    QVariantMap response;
    if (!waitForRequest(reqId, &response))
        return QPlaceCategory();

    if (response.value(JsonConverter::Length).toInt() <= 0) {
        return QPlaceCategory();
    }

    return JsonConverter::convertJsonMapToCategory(response.value(JsonConverter::Data).toList().first().toMap(), this);
}

QList<QPlaceCategory> QPlaceManagerEngineJsonDb::childCategories(const QString &parentId) const
{
    int reqId = m_db->query(QString("[?%1=\"%2\"][?%3 = \"%4\"]").arg(JsonConverter::Type).arg(JsonConverter::CategoryType)
                .arg(JsonConverter::CategoryParentId).arg(parentId));

    QVariantMap responseMap;
    if (!waitForRequest(reqId, &responseMap)) {
        return QList<QPlaceCategory>();
    }

    if (responseMap.value(JsonConverter::Length).toInt() <= 0)
        return QList<QPlaceCategory>();

    return JsonConverter::convertJsonResponseToCategories(responseMap, this);
}

QList<QLocale> QPlaceManagerEngineJsonDb::locales() const
{
    return QList<QLocale>();
}

void QPlaceManagerEngineJsonDb::setLocales(const QList<QLocale> &locales)
{
    Q_UNUSED(locales);
}

QUrl QPlaceManagerEngineJsonDb::constructIconUrl(const QPlaceIcon &icon, const QSize &size, QPlaceIcon::IconFlags flags)
{
    Q_UNUSED(size)
    Q_UNUSED(flags)

    return icon.fullUrl();
}

QPlaceManager::ManagerFeatures QPlaceManagerEngineJsonDb::supportedFeatures() const
{
    return QPlaceManager::SavePlaceFeature |
           QPlaceManager::RemovePlaceFeature |
           QPlaceManager::SaveCategoryFeature |
           QPlaceManager::RemoveCategoryFeature;
}

void QPlaceManagerEngineJsonDb::processJsonDbResponse(int id, const QVariant &data)
{
    if (m_helperMap.contains(id)) {
        m_helperMap.insert(id, data);
        m_eventLoop.exit();
    }
}

void QPlaceManagerEngineJsonDb::processJsonDbError(int id, int code, const QString &jsonDbErrorString)
{
    Q_UNUSED(code)
    Q_UNUSED(jsonDbErrorString)

    if (m_helperMap.contains(id)) {
        m_helperMap.insert(id, false);
        m_eventLoop.exit();
    }
}

void QPlaceManagerEngineJsonDb::setupNotificationsResponse(int id, const QVariant &data)
{
    if (id == m_notificationReqId) {
        m_notificationUuid = data.toMap().value(JsonConverter::Uuid).toString();
        disconnect(m_db, SIGNAL(response(int,QVariant)),
                this, SLOT(setupNotificationsResponse(int,QVariant)));
        disconnect(m_db, SIGNAL(error(int,int,QString)),
                this, SLOT(setupNotificationsResponse(int,QVariant)));
    }
}

void QPlaceManagerEngineJsonDb::setupNotificationsError(int id, int code, const QString &error)
{
    if (id == m_notificationReqId) {
        qWarning() << "Place notifications could not be initialized";
        disconnect(m_db, SIGNAL(response(int,QVariant)),
                this, SLOT(setupNotificationsResponse(int,QVariant)));
        disconnect(m_db, SIGNAL(error(int,int,QString)),
                this, SLOT(setupNotificationsResponse(int,QVariant)));
    }
}

bool QPlaceManagerEngineJsonDb::waitForRequest(int reqId, QVariantMap *variantMap) const
{
    m_helperMap.insert(reqId, QVariant());
    m_eventLoop.exec(QEventLoop::AllEvents);

    QVariant response = m_helperMap.value(reqId);
    if (response.type() == QVariant::Bool) {
        if (variantMap)
            *variantMap = QVariantMap();
        return false;
    } else {
        if (variantMap)
            *variantMap = response.toMap();
        return true;
    }
}

void QPlaceManagerEngineJsonDb::processJsonDbNotification(const QString &notifyUuid, const QVariant &object, const QString &action)
{
    if (notifyUuid != m_notificationUuid)
        return;

    if (object.toMap().value(JsonConverter::Type).toString() == JsonConverter::PlaceType) {
        if (action == JsonConverter::Create)
            emit placeAdded(object.toMap().value(JsonConverter::Uuid).toString());
        else if (action == JsonConverter::Update)
            emit placeUpdated(object.toMap().value(JsonConverter::Uuid).toString());
        else if (action == JsonConverter::Remove)
            emit placeRemoved(object.toMap().value(JsonConverter::Uuid).toString());

    } else if (object.toMap().value(JsonConverter::Type).toString() == JsonConverter::CategoryType) {
        QPlaceCategory category = JsonConverter::convertJsonMapToCategory(object.toMap(), this);
        QStringList ancestors = object.toMap().value(JsonConverter::Ancestors).toStringList();
        QString parentId;
        if (ancestors.count() >= 2) {
            ancestors.takeLast();//last element in ancestors is always the uuid of the catgory itself
            parentId = ancestors.takeLast(); //the 2nd last element must be the parent;
        }

        if (action == JsonConverter::Create)
            emit categoryAdded(category, parentId);
        else if (action == JsonConverter::Update)
            emit categoryUpdated(category, parentId);
        else if (action == JsonConverter::Remove)
            emit categoryRemoved(category.categoryId(), parentId);
    }
}
