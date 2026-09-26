# NEXUS GCS QGroundControl custom build.

message(STATUS "NEXUS: enabling custom build")

set(NEXUS_QML_OVERRIDES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FlyViewCustomLayer.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FlyViewToolStripActionList.qml"
)

set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FlyViewCustomLayer.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/FlyViewCustomLayer.qml"
)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FlyViewToolStripActionList.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QGroundControl/FlyView/FlyViewToolStripActionList.qml"
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
