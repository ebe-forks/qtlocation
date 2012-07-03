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

import QtTest 1.0
import QtLocation 5.0
import "utils.js" as Utils

TestCase {
    id: testCase

    name: "ContactDetail"

    ContactDetail { id: emptyContactDetail }

    function test_empty() {
        compare(emptyContactDetail.label, "");
        compare(emptyContactDetail.value, "");
    }

    ContactDetail {
        id: qmlContactDetail

        label: "Phone"
        value: "12345"
    }

    function test_qmlConstructedContactDetail() {
        compare(qmlContactDetail.label, "Phone");
        compare(qmlContactDetail.value, "12345");
    }

    ContactDetail {
        id: testContactDetail
    }

    function test_setAndGet_data() {
        return [
            { tag: "label", property: "label", signal: "labelChanged", value: "Phone", reset: "" },
            { tag: "value", property: "value", signal: "valueChanged", value: "12345", reset: "" },
        ];
    }

    function test_setAndGet(data) {
        Utils.testObjectProperties(testCase, testContactDetail, data);
    }
}
