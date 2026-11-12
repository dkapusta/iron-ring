# `cmake --install` (docs/install.md): the player folder is self-contained and
# relocatable (the launcher finds the engine next to itself), so installing is
# copying it. On Linux a desktop entry and a hicolor icon make it a regular app.
#
#   Linux:   <prefix>/lib/IronRing/            the player folder
#            <prefix>/share/applications/<IR_APP_ID>.desktop
#            <prefix>/share/icons/hicolor/256x256/apps/<IR_APP_ID>.png
#   Windows: <prefix>/IronRing/                the player folder
#   macOS:   <prefix>/IronRing.app             the bundle (--prefix /Applications)

include(GNUInstallDirs)

if(APPLE)
    set(_default_dir ".") # the player folder holds only IronRing.app
elseif(WIN32)
    set(_default_dir "${IR_PRODUCT_NAME}")
else()
    set(_default_dir "${CMAKE_INSTALL_LIBDIR}/${IR_PRODUCT_NAME}")
endif()
set(IR_INSTALL_DIR "${_default_dir}" CACHE STRING "Install destination of the player folder (relative to the prefix)")

install(DIRECTORY "${IR_PLAYER_OUTPUT_DIR}/" DESTINATION "${IR_INSTALL_DIR}" USE_SOURCE_PERMISSIONS
        COMPONENT Player)

# The licence texts (Licenses.cmake) beside the player. Not on macOS: files added
# inside a signed IronRing.app would break its signature; the release zip carries them.
if(NOT APPLE)
    install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${IR_INSTALL_DIR}/Licenses"
            RENAME "IronRing.txt" COMPONENT Player)
    install(DIRECTORY "${IR_LICENSES_DIR}/" DESTINATION "${IR_INSTALL_DIR}/Licenses" COMPONENT Player)
    install(FILES "${PROJECT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md" DESTINATION "${IR_INSTALL_DIR}/Licenses"
            COMPONENT Player)
endif()

if(UNIX AND NOT APPLE)
    install(FILES "${PROJECT_SOURCE_DIR}/Assets/Resources/UI/icon.png"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps"
            RENAME "${IR_APP_ID}.png" COMPONENT Player)
    # Exec needs an absolute path, and the prefix is only final at install time
    # (`cmake --install --prefix ...`), so the entry is written then.
    install(CODE "
        set(IR_PRODUCT_DISPLAY_NAME [[${IR_PRODUCT_DISPLAY_NAME}]])
        set(IR_APP_ID [[${IR_APP_ID}]])
        set(_prefix \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}\")
        set(_dir \"\${CMAKE_INSTALL_PREFIX}/${IR_INSTALL_DIR}\")
        set(IR_DESKTOP_PATH \"\${_dir}\")
        set(IR_DESKTOP_EXEC \"\${_dir}/${IR_PRODUCT_NAME}${IR_PLAYER_EXE_SUFFIX}\")
        configure_file([[${PROJECT_SOURCE_DIR}/cmake/templates/app.desktop.in]]
                       \"\${_prefix}/${CMAKE_INSTALL_DATADIR}/applications/${IR_APP_ID}.desktop\" @ONLY)
        message(STATUS \"Installing: \${_prefix}/${CMAKE_INSTALL_DATADIR}/applications/${IR_APP_ID}.desktop\")
    " COMPONENT Player)
endif()
