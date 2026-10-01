# NEXUS GCS QGroundControl custom build.

message(STATUS "NEXUS: enabling custom build")

# Overlay Android packaging files without modifying the pinned QGC source tree.
if(ANDROID)
    set(NEXUS_ANDROID_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    if(IS_DIRECTORY "${NEXUS_ANDROID_DIR}")
        set(DEFAULT_ANDROID_DIR "${CMAKE_SOURCE_DIR}/android")
        set(FINAL_ANDROID_DIR "${CMAKE_BINARY_DIR}/${QGC_CUSTOM_DIR}/android")

        file(REMOVE_RECURSE "${FINAL_ANDROID_DIR}")
        file(COPY "${DEFAULT_ANDROID_DIR}/" DESTINATION "${FINAL_ANDROID_DIR}")
        file(COPY "${NEXUS_ANDROID_DIR}/" DESTINATION "${FINAL_ANDROID_DIR}")

        set(QGC_ANDROID_PACKAGE_SOURCE_DIR "${FINAL_ANDROID_DIR}")
        set(QGC_ANDROID_PACKAGE_SOURCE_DIR "${FINAL_ANDROID_DIR}" PARENT_SCOPE)
        message(STATUS "NEXUS: Android package overlay staged at ${FINAL_ANDROID_DIR}")
    endif()
endif()

set(NEXUS_QML_OVERRIDES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FlyViewCustomLayer.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FlyViewToolStripActionList.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusStatusChip.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusMetric.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusNavItem.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusActionButton.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusOpsPanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PlanToolBarIndicators.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusHealthCard.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusHealthPanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAlertPanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPreflightCard.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPreflightPanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPayloadPanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusVehiclePanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusEngineerPanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusOfflinePanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAnalyzeGraph.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAnalyzePanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusReportsPanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusDeviceHealthPanel.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusSecurityPanel.qml"
)

set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FlyViewCustomLayer.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/FlyViewCustomLayer.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FlyViewToolStripActionList.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/FlyViewToolStripActionList.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusStatusChip.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusStatusChip.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusMetric.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusMetric.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusNavItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusNavItem.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusActionButton.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusActionButton.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusOpsPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusOpsPanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PlanToolBarIndicators.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/PlanView/PlanToolBarIndicators.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusHealthCard.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusHealthCard.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusHealthPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusHealthPanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAlertPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusAlertPanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPreflightCard.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusPreflightCard.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPreflightPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusPreflightPanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPayloadPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusPayloadPanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusVehiclePanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusVehiclePanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusEngineerPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusEngineerPanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusOfflinePanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusOfflinePanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAnalyzeGraph.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusAnalyzeGraph.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAnalyzePanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusAnalyzePanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusReportsPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusReportsPanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusDeviceHealthPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusDeviceHealthPanel.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NexusSecurityPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/NexusSecurityPanel.qml"
)

qt_add_resources(
    ${CMAKE_PROJECT_NAME}
    nexus_qml_overrides
    PREFIX "/Custom/qml"
    FILES ${NEXUS_QML_OVERRIDES}
)

set(CUSTOM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPlanVerifier.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPlanVerifier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusHealthModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusHealthModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAlertManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAlertManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPreflightModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPreflightModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPayloadModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusPayloadModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusVehicleModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusVehicleModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusEngineerModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusEngineerModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusOfflineModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusOfflineModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAnalyzeModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusAnalyzeModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusReportsModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusReportsModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusDeviceHealthModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusDeviceHealthModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusSecurityModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NexusSecurityModel.h
    PARENT_SCOPE
)

set(CUSTOM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    PARENT_SCOPE
)

set(CUSTOM_DEFINITIONS
    QGC_CUSTOM_BUILD
    CUSTOMHEADER="NexusPlugin.h"
    CUSTOMCLASS=NexusPlugin
    PARENT_SCOPE
)

set(CUSTOM_QT_COMPONENTS
    Core
    PARENT_SCOPE
)


# Task 0.3 validation tests compile only into desktop test builds.
# They exercise the custom Fly cockpit against QGC's real test harness and
# an externally launched PX4 SITL process without touching production paths.
if(QGC_BUILD_TESTING AND NOT ANDROID)
    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusFlightCockpitUITest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusFlightCockpitUITest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPX4SITLRuntimeTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPX4SITLRuntimeTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPlanEditingTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPlanEditingTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPX4PlanSITLTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPX4PlanSITLTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusHealthModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusHealthModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusAlertManagerTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusAlertManagerTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPreflightModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPreflightModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPayloadModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusPayloadModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusVehicleModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusVehicleModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusEngineerModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusEngineerModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusOfflineModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusOfflineModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusAnalyzeModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusAnalyzeModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusReportsModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusReportsModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusDeviceHealthModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusDeviceHealthModelTest.h
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusSecurityModelTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/test/NexusSecurityModelTest.h
    )
    target_include_directories(${CMAKE_PROJECT_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/test/QmlUITests
            ${CMAKE_SOURCE_DIR}/test/UnitTestFramework
            ${CMAKE_SOURCE_DIR}/test/UnitTestFramework/BaseClasses
            ${CMAKE_CURRENT_SOURCE_DIR}/test
    )
endif()
