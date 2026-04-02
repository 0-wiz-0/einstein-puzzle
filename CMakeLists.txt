cmake_minimum_required(VERSION 3.10)
project(einstein VERSION 2.2 LANGUAGES CXX)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(ZLIB REQUIRED)

# Sources shared between mkres and einstein
set(SHARE_SOURCES
    exceptions.cpp
    unicode.cpp
    streams.cpp
    table.cpp
    buffer.cpp
    convert.cpp
    lexal.cpp
)

# mkres tool sources
set(RES_SOURCES
    mkres.cpp
    compressor.cpp
    format.cpp
    msgwriter.cpp
    msgformatter.cpp
    args.cpp
)

# Main game sources
set(GAME_SOURCES
    puzgen.cpp
    main.cpp
    screen.cpp
    resources.cpp
    utils.cpp
    game.cpp
    widgets.cpp
    iconset.cpp
    puzzle.cpp
    rules.cpp
    verthints.cpp
    random.cpp
    horhints.cpp
    menu.cpp
    font.cpp
    storage.cpp
    tablestorage.cpp
    regstorage.cpp
    topscores.cpp
    opensave.cpp
    descr.cpp
    options.cpp
    messages.cpp
    formatter.cpp
    i18n.cpp
    tokenizer.cpp
    sound.cpp
    args.cpp
)

# mkres build tool
add_executable(mkres ${RES_SOURCES} ${SHARE_SOURCES})
target_link_libraries(mkres PRIVATE ZLIB::ZLIB)

# Generate einstein.res from resources
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/einstein.res
    COMMAND mkres --source resources.descr --output ${CMAKE_CURRENT_BINARY_DIR}/einstein.res
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res
    DEPENDS mkres
    COMMENT "Building einstein.res"
)
add_custom_target(einstein_res ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/einstein.res
)

# Main game executable
add_executable(einstein WIN32 ${GAME_SOURCES} ${SHARE_SOURCES})

add_dependencies(einstein einstein_res)

# Include directories for SDL2 (handle both modern targets and legacy variables)
if(TARGET SDL2::SDL2)
    target_link_libraries(einstein PRIVATE SDL2::SDL2 SDL2::SDL2main)
else()
    target_include_directories(einstein PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(einstein PRIVATE ${SDL2_LIBRARIES})
endif()

if(TARGET SDL2_mixer::SDL2_mixer)
    target_link_libraries(einstein PRIVATE SDL2_mixer::SDL2_mixer)
else()
    target_link_libraries(einstein PRIVATE SDL2_mixer)
endif()

if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(einstein PRIVATE SDL2_ttf::SDL2_ttf)
else()
    target_link_libraries(einstein PRIVATE SDL2_ttf)
endif()

target_link_libraries(einstein PRIVATE ZLIB::ZLIB)

# PREFIX definition for resource path (non-Windows)
if(NOT WIN32)
    if(NOT DEFINED CMAKE_INSTALL_PREFIX)
        set(CMAKE_INSTALL_PREFIX /usr/local)
    endif()
    target_compile_definitions(einstein PRIVATE PREFIX=L"${CMAKE_INSTALL_PREFIX}")
endif()

# Install
include(GNUInstallDirs)
install(TARGETS einstein RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/einstein.res
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/einstein/res
)
