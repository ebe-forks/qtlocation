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
import "utils.js" as Utils

TestCase {
    id: testCase

    name: "CategoryModel"

    CategoryModel {
        id: testModel
    }

    Plugin {
        id: testPlugin
        name: "qmlgeo.test.plugin"
        allowExperimental: true
        parameters: [
            PluginParameter {
                name: "initializePlaceData"
                value: true
            }
        ]
    }

    function test_setAndGet_data() {
        return [
            { tag: "plugin", property: "plugin", signal: "pluginChanged", value: testPlugin },
            { tag: "hierarchical", property: "hierarchical", signal: "hierarchicalChanged", value: false, reset: true },
        ];
    }

    function test_setAndGet(data) {
        Utils.testObjectProperties(testCase, testModel, data);
    }

    VisualDataModel {
        id: categoryModel

        model: CategoryModel { }
        delegate: Item { }
    }

    function test_hierarchicalModel() {
        var modelSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        modelSpy.target = categoryModel.model;
        modelSpy.signalName = "statusChanged";

        compare(categoryModel.model.status, CategoryModel.Ready);
        compare(categoryModel.count, 0);


        // set the plugin
        categoryModel.model.plugin = testPlugin;

        tryCompare(categoryModel.model, "status", CategoryModel.Updating);
        compare(modelSpy.count, 1);

        tryCompare(categoryModel.model, "status", CategoryModel.Ready);
        compare(modelSpy.count, 2);

        var expectedNames = [ "Accommodation", "Park" ];

        compare(categoryModel.count, expectedNames.length);

        for (var i = 0; i < expectedNames.length; ++i) {
            var category = categoryModel.model.data(categoryModel.modelIndex(i),
                                                    CategoryModel.CategoryRole);
            compare(category.name, expectedNames[i]);
        }


        // check that "Accommodation" has children
        categoryModel.rootIndex = categoryModel.modelIndex(0);

        expectedNames = [ "Camping", "Hotel", "Motel" ];

        compare(categoryModel.count, expectedNames.length);

        for (i = 0; i < expectedNames.length; ++i) {
            category = categoryModel.model.data(categoryModel.modelIndex(i),
                                                    CategoryModel.CategoryRole);
            compare(category.name, expectedNames[i]);

            var parentCategory = categoryModel.model.data(categoryModel.modelIndex(i),
                                                         CategoryModel.ParentCategoryRole);
            compare(parentCategory.name, "Accommodation");
        }

        categoryModel.rootIndex = categoryModel.parentModelIndex();

        compare(categoryModel.count, 2);


        // check that "Park" has no children
        categoryModel.rootIndex = categoryModel.modelIndex(1);

        compare(categoryModel.count, 0);

        categoryModel.rootIndex = categoryModel.parentModelIndex();


        // clean up
        categoryModel.model.plugin = null;


        // check that model is empty
        compare(categoryModel.count, 0);
    }

    function test_flatModel() {
        var modelSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        modelSpy.target = categoryModel.model;
        modelSpy.signalName = "statusChanged";

        compare(categoryModel.model.status, CategoryModel.Ready);
        compare(categoryModel.count, 0);


        // set the plugin
        categoryModel.model.hierarchical = false;
        categoryModel.model.plugin = testPlugin;

        tryCompare(categoryModel.model, "status", CategoryModel.Updating);
        compare(modelSpy.count, 1);

        tryCompare(categoryModel.model, "status", CategoryModel.Ready);
        compare(modelSpy.count, 2);

        var expectedNames = [ "Accommodation", "Camping", "Hotel", "Motel", "Park" ];

        compare(categoryModel.count, expectedNames.length);

        for (var i = 0; i < expectedNames.length; ++i) {
            var category = categoryModel.model.data(categoryModel.modelIndex(i),
                                                    CategoryModel.CategoryRole);
            var name = categoryModel.model.data(categoryModel.modelIndex(i), 0);    // DisplayRole

            compare(name, expectedNames[i]);
            compare(category.name, expectedNames[i]);
        }


        // check that no category has children
        for (i = 0; i < categoryModel.count; ++i) {
            categoryModel.rootIndex = categoryModel.modelIndex(i);

            compare(categoryModel.count, 0);

            categoryModel.rootIndex = categoryModel.parentModelIndex();
        }


        // clean up
        categoryModel.model.hierarchical = true;
        categoryModel.model.plugin = null;


        // check that model is empty
        compare(categoryModel.count, 0);
    }
}
