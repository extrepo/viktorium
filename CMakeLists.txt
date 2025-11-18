cmake_minimum_required(VERSION 3.8)
project(viktorium VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Определяем пути для install
if(WIN32)
    set(CMAKE_INSTALL_BINDIR "." CACHE PATH "Executable binaries")
    set(CMAKE_INSTALL_LIBDIR "." CACHE PATH "Libraries")
    set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE PATH "Header files")
    set(CMAKE_INSTALL_DOCDIR "doc" CACHE PATH "Documentation")
else()
    include(GNUInstallDirs)
endif()

option(APP_DEPLOYQT "Deploy QT in Windows" OFF)

# Все переменные начинающиеся с APP_ представляем как #define
get_cmake_property(_VARNAMES VARIABLES)
foreach(_VARNAME ${_VARNAMES})
    if(_VARNAME MATCHES "^APP_.*")
        add_definitions(-D${_VARNAME}="${${_VARNAME}}")
    endif()
endforeach()

# Установка директорий для сборки проекта
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/debug)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/debug)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/release)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/release)
endif()

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj -O2")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Подключение Qt-библиотек
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5Sql CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)

# Подгружаем необходимые библиотеки
add_subdirectory(lib/unilog)
add_subdirectory(lib/utils)

# Подключение хэдеров
file(GLOB_RECURSE INCLUDES "include/*.h")

# Подключение исходников
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "src/main.cpp")

# Cоздание исполняемого файла проекта
add_executable(${PROJECT_NAME} ${INCLUDES} ${SOURCES} "src/main.cpp" resources.qrc resources.rc)
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE true)
install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(${PROJECT_NAME} PUBLIC utils unilog Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Sql Qt5::Network)

# Шаблоны
install(FILES "install/welcome.html" DESTINATION "${CMAKE_INSTALL_BINDIR}/vikatemplates/")
file(COPY "install/welcome.html" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vikatemplates/")
install(FILES "install/template1.html" DESTINATION "${CMAKE_INSTALL_BINDIR}/vikatemplates/")
file(COPY "install/template1.html" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vikatemplates/")
install(FILES "install/finish.html" DESTINATION "${CMAKE_INSTALL_BINDIR}/vikatemplates/")
file(COPY "install/finish.html" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vikatemplates/")

# Тесты
add_executable(lmtest ${INCLUDES} ${SOURCES} "tests/lmtest.cpp" resources.qrc resources.rc)
target_include_directories(lmtest PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(lmtest PUBLIC utils unilog Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Sql Qt5::Network)

add_executable(dbtest ${INCLUDES} ${SOURCES} "tests/dbtest.cpp" resources.qrc resources.rc)
target_include_directories(dbtest PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(dbtest PUBLIC utils unilog Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Sql Qt5::Network)

add_executable(resulttest ${INCLUDES} ${SOURCES} "tests/resulttest.cpp" resources.qrc resources.rc)
target_include_directories(resulttest PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(resulttest PUBLIC utils unilog Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Sql Qt5::Network)

# Подготовка окружения для инсталлятора
if(WIN32 AND APP_DEPLOYQT)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${WINDEPLOYQT_EXECUTABLE}"
        --release
        --no-opengl-sw
        --dir "${CMAKE_BINARY_DIR}/windeployqt"
        "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt libraries..."
    )
    install(DIRECTORY "${CMAKE_BINARY_DIR}/windeployqt/"
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        FILES_MATCHING PATTERN "*.dll"
    )
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PROJECT_NAME}")
set(CPACK_NSIS_PACKAGE_NAME "Викториум")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};${CPACK_NSIS_PACKAGE_NAME}")
set(CPACK_CREATE_DESKTOP_SHORTCUTS ON)
set(CPACK_NSIS_UNICODE TRUE)
set(CPACK_NSIS_EXECUTABLE_PRE_ARGUMENTS "/INPUTCHARSET UTF8")
set(CPACK_NSIS_COMPRESSOR "ZLIB")
set(CPACK_PACKAGE_CONTACT "ckbi@eleron.org")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/install/description.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "СПО Викториум")
set(CPACK_PACKAGE_VENDOR "JSC FCS&HT «SNPO «Eleron»")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/install/license.txt")

# Иконки на рабочем столе
if(WIN32)
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\\\${PROJECT_NAME}.lnk' '$INSTDIR\\\\${PROJECT_NAME}.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$DESKTOP\\\\${PROJECT_NAME}.lnk'")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icons\\\\${PROJECT_NAME}.ico")
else()
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/install/eleron-${PROJECT_NAME}.desktop.in" "${CMAKE_BINARY_DIR}/eleron-${PROJECT_NAME}.desktop" @ONLY)
    install(FILES "${CMAKE_BINARY_DIR}/eleron-${PROJECT_NAME}.desktop" DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/icons/${PROJECT_NAME}.png" DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/install/postinst.in" "${CMAKE_BINARY_DIR}/postinst" @ONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/install/postrm.in" "${CMAKE_BINARY_DIR}/postrm" @ONLY)
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst;${CMAKE_BINARY_DIR}/postrm")
endif()

include(CPack)

# SBOM
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/lib/cmake-sbom")
include(sbom)
sbom_generate(
        OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sbom-${PROJECT_NAME}.spdx
        SUPPLIER Eleron
        SUPPLIER_URL https://eleron.ru
)
sbom_directory(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} FILETYPE BINARY)
sbom_finalize()
