# NEXUS GCS custom-build configuration.
# Keep this file small. Prefer QGC extension points over upstream edits.

set(QGC_APP_NAME "NEXUS GCS" CACHE STRING "Application name" FORCE)
set(QGC_APP_DESCRIPTION "Offline-first UAV Ground Control Station" CACHE STRING "Application description" FORCE)
set(QGC_ORG_NAME "NEXUS GCS" CACHE STRING "Organization name" FORCE)
set(QGC_ORG_DOMAIN "nexus-gcs.local" CACHE STRING "Organization domain" FORCE)
set(QGC_PACKAGE_NAME "com.abitss.nexusgcs" CACHE STRING "Package identifier" FORCE)
set(QGC_ANDROID_PACKAGE_NAME "com.abitss.nexusgcs" CACHE STRING "Android package identifier" FORCE)

# V0.1 keeps the stock PX4 and ArduPilot firmware factories available.
# PX4 is the first qualification target; ArduPilot remains available for later validation.
