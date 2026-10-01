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
    )
    target_include_directories(${CMAKE_PROJECT_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/test/QmlUITests
            ${CMAKE_SOURCE_DIR}/test/UnitTestFramework
            ${CMAKE_SOURCE_DIR}/test/UnitTestFramework/BaseClasses
            ${CMAKE_CURRENT_SOURCE_DIR}/test
    )
endif()
