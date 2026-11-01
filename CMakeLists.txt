cmake_minimum_required(VERSION 3.16)

project(sensornode-ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

option(SENSORNODE_BUILD_APP "Build the application, needs Qt Charts" ON)
option(BUILD_TESTING "Build tests when tests/ is present" OFF)

find_package(Qt5 5.15 COMPONENTS Core Network Widgets REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)

add_compile_options(-Wall -Wextra -Wpedantic)

if(PRODUCTION_BUILD)
  add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

add_library(sensornode-ui-lib STATIC
  src/ipc/DaemonClient.cpp
  src/ipc/DaemonClient.h
  src/ipc/DaemonEndpoints.h

  src/model/HistoryModel.cpp
  src/model/HistoryModel.h
  src/model/OtaModel.cpp
  src/model/OtaModel.h
  src/model/OtaState.h
  src/model/Sample.h
  src/model/DisplayModel.cpp
  src/model/DisplayModel.h
  src/model/ChartScale.h
  src/model/DeviceInfoModel.cpp
  src/model/DeviceInfoModel.h
  src/model/ErrorLogModel.cpp
  src/model/ErrorLogModel.h
  src/model/Metric.h
  src/model/SettingsModel.cpp
  src/model/SettingsModel.h
  src/model/Locations.cpp
  src/model/Locations.h
  src/model/SystemStatusModel.cpp
  src/model/SystemStatusModel.h
  src/model/TelemetryModel.cpp
  src/model/TelemetryModel.h
)

target_include_directories(sensornode-ui-lib PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(sensornode-ui-lib PUBLIC
  Qt5::Core
  Qt5::Network
  PkgConfig::SQLITE3
)

add_library(sensornode-ui-widgets STATIC
  src/app/DemoData.h
  src/app/PageFrame.cpp
  src/app/PageFrame.h
  src/app/Stubs.cpp
  src/app/Stubs.h
  src/app/Theme.cpp
  src/app/Theme.h

  src/screens/DevicePage.cpp
  src/screens/DevicePage.h
  src/screens/DisplayPage.cpp
  src/screens/DisplayPage.h
  src/screens/ErrorLogPage.cpp
  src/screens/ErrorLogPage.h
  src/screens/FirmwarePage.cpp
  src/screens/FirmwarePage.h
  src/screens/HomeScreen.cpp
  src/screens/HomeScreen.h
  src/screens/LanguageLocationPage.cpp
  src/screens/LanguageLocationPage.h
  src/screens/LocationPage.cpp
  src/screens/LocationPage.h
  src/screens/SamplingPage.cpp
  src/screens/SamplingPage.h
  src/screens/SensorsPage.cpp
  src/screens/SensorsPage.h
  src/screens/ServicePage.cpp
  src/screens/ServicePage.h
  src/screens/SettingsScreen.cpp
  src/screens/SettingsScreen.h
  src/screens/UpdateFlowPage.cpp
  src/screens/UpdateFlowPage.h
  src/screens/UpdatePage.cpp
  src/screens/UpdatePage.h

  src/widgets/BottomNavBar.cpp
  src/widgets/BottomNavBar.h
  src/widgets/BottomSheet.cpp
  src/widgets/BottomSheet.h
  src/widgets/KeyValueList.cpp
  src/widgets/KeyValueList.h
  src/widgets/ListRow.cpp
  src/widgets/ListRow.h
  src/widgets/MetricTile.cpp
  src/widgets/MetricTile.h
  src/widgets/MiniSegment.cpp
  src/widgets/MiniSegment.h
  src/widgets/ProgressRing.cpp
  src/widgets/ProgressRing.h
  src/widgets/RaisedButton.cpp
  src/widgets/RaisedButton.h
  src/widgets/ScrollList.cpp
  src/widgets/ScrollList.h
  src/widgets/SegmentedControl.cpp
  src/widgets/SegmentedControl.h
  src/widgets/StepBar.cpp
  src/widgets/StepBar.h
  src/widgets/StatusHeader.cpp
  src/widgets/StatusHeader.h
  src/widgets/SubPage.cpp
  src/widgets/SubPage.h
  src/widgets/TouchSlider.cpp
  src/widgets/TouchSlider.h

  resources/assets.qrc
  resources/resources.qrc
)

target_include_directories(sensornode-ui-widgets PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(sensornode-ui-widgets PUBLIC sensornode-ui-lib Qt5::Widgets)

if(SENSORNODE_BUILD_APP)
  find_package(Qt5 5.15 COMPONENTS Charts REQUIRED)

  add_executable(sensornode-ui
    src/main.cpp

    src/app/AppWindow.cpp
    src/app/AppWindow.h

    src/screens/ChartScreen.cpp
    src/screens/ChartScreen.h
    src/screens/UpdatePopup.cpp
    src/screens/UpdatePopup.h
    src/screens/UpdatePopup.ui
    src/screens/UpdateProgressScreen.cpp
    src/screens/UpdateProgressScreen.h
    src/screens/UpdateProgressScreen.ui
  )

  target_link_libraries(sensornode-ui PRIVATE
    sensornode-ui-lib
    sensornode-ui-widgets
    Qt5::Charts
  )

  install(TARGETS sensornode-ui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_TESTING AND EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
  enable_testing()
  add_subdirectory(tests)
endif()
