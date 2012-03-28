/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0
import QtLocation 5.0

Item {
    id: root

    property Fixture fixture: Qt.createQmlObject('Fixture {}', root)



    BoundingBox {
        id: germanyBerlinSiegessaeule
        center: Coordinate {
            latitude: 52.5143093
            longitude: 13.3497963
        }
        width: 200
        height: 200
    }

    BoundingBox {
        id: germanyBerlinErnstReuterPlatz
        center: Coordinate {
            latitude: 52.512608
            longitude: 13.321848
        }
        width: 200
        height: 200
    }

    Coordinate {
        id: germanyBerlinDorotheenstr100
        latitude: 52.51792
        longitude: 13.377862
    }

    Coordinate {
        id: germanyBerlinBismarkstr19
        latitude: 52.512246
        longitude: 13.313481
    }


    TestCase {
        name: "Excluded areas"

        function test_router_computes_route_around_one_excluded_area() {
            skip("Currenty not supported")
            var testData = { }
            testData.waypoints = [ germanyBerlinDorotheenstr100, germanyBerlinBismarkstr19 ]
            testData.excludedAreas = [germanyBerlinSiegessaeule]
            testData.noGrep = ["Sieges", "Stern"]
            testData.grep = ["des 17. Juni", "Ernst-Reuter"]
            fixture.runTest(this, testData)
        }

        function test_router_computes_route_around_several_excluded_area() {
            skip("Currenty not supported")
            var testData = { }
            testData.waypoints = [ germanyBerlinDorotheenstr100, germanyBerlinBismarkstr19 ]
            testData.excludedAreas = [germanyBerlinSiegessaeule, germanyBerlinErnstReuterPlatz]
            testData.noGrep = ["Sieges", "Stern", "des 17. Juni", "Ernst-Reuter"]
            fixture.runTest(this, testData)
        }
    }
}




