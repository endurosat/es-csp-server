include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(PkgConfig REQUIRED)
pkg_get_variable(SYSTEMD_UNIT_DIR systemd systemdsystemunitdir)
# If no build type was specified, set it to Release.
if(NOT SYSTEMD_UNIT_DIR)
	set(SYSTEMD_UNIT_DIR ${CMAKE_INSTALL_PREFIX}/etc/systemd/system CACHE STRING
	"Default install path for services" FORCE)
endif(NOT SYSTEMD_UNIT_DIR)

# If no build type was specified, set it to Release.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
            "Choose the type of build, options are: None Debug Release."
            FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# If no installation prefix is given manually, install locally.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING
            "The install location"
            FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Projectwise paths
set(PROJECT_PREFIX ${PROJECT_NAME})
set(INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})
set(INSTALL_CONFIG_DIR  ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME})
set(INSTALL_SYSCONFIG_DIR  ${CMAKE_INSTALL_SYSCONFDIR})
set(INSTALL_LIBRARY_DIR ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_ARCHIVE_DIR ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
set(INSTALL_SRC_DIR src/${PROJECT_NAME})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(INSTALL_SYSTEMD_SERVICE ${CMAKE_INSTALL_PREFIX}/${SYSTEMD_UNIT_DIR})
else(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(INSTALL_SYSTEMD_SERVICE ${SYSTEMD_UNIT_DIR})
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
