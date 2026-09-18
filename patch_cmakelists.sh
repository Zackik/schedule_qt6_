#!/bin/bash
sed -i 's/find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network NetworkAuth Sql PrintSupport Qml Quick QuickWidgets)/find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network NetworkAuth Sql PrintSupport)/g' CMakeLists.txt
sed -i '/Qt6::Qml/d' CMakeLists.txt
sed -i '/Qt6::Quick/d' CMakeLists.txt
sed -i '/Qt6::QuickWidgets/d' CMakeLists.txt
