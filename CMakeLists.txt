cmake_minimum_required(VERSION 3.16)

# Project Name and Version
project(SmartSchedule VERSION 2.0.0 LANGUAGES CXX)

# Enforce C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling (clang-tidy, clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable AutoMOC, AutoUIC, AutoRCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network NetworkAuth Sql PrintSupport)
find_package(Qt6 QUIET COMPONENTS Multimedia)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906 /w14928)
endif()

# Enable Clang-Tidy if available
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# Define main executable
add_executable(SmartSchedule
    src/ui/companion/AIChibiWidget.cpp
    src/ui/companion/AIChibiWidget.h
    src/ui/companion/ChatBubbleWidget.cpp
    src/ui/companion/ChatBubbleWidget.h
    src/services/companion/AnimationController.cpp
    src/services/companion/AnimationController.h
    src/services/companion/AssistantService.cpp
    src/services/companion/AssistantService.h
    src/utils/GlobalHotkeyManager.cpp
    src/utils/GlobalHotkeyManager.h
    src/services/ResourceMonitor.cpp
    src/services/ResourceMonitor.h
    src/managers/StorageManager.cpp
    src/managers/StorageManager.h
    src/ui/SettingsWidget.cpp
    src/ui/SettingsWidget.h
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.ui
    src/calendar/CalendarManager.cpp
    src/calendar/CalendarManager.h
    src/services/oauth/GoogleOAuthManager.cpp
    src/services/oauth/GoogleOAuthManager.h
    src/ai/GeminiService.cpp
    src/ai/GeminiService.h
    src/ui/ScheduleWidget.cpp
    src/ui/ScheduleWidget.h
    resources.qrc
)

target_include_directories(SmartSchedule PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(SmartSchedule PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::PrintSupport
    Qt6::Sql
)

if(TARGET Qt6::Multimedia)
    target_link_libraries(SmartSchedule PRIVATE Qt6::Multimedia)
endif()

# Platform specific configurations
if(WIN32)
    set_target_properties(SmartSchedule PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(SmartSchedule PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Smart Schedule Qt6"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.aistudio.smartschedule"
    )
endif()

# Testing via GoogleTest
include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    
    # Suppress policy warning for FetchContent
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
        add_executable(SmartSchedule_Tests 
        tests/test_main.cpp 
        tests/test_resources.cpp 
        src/services/ResourceMonitor.cpp 
        src/managers/StorageManager.cpp
        src/calendar/CalendarManager.cpp
        src/services/oauth/GoogleOAuthManager.cpp
        src/ai/GeminiService.cpp
    )
    target_link_libraries(SmartSchedule_Tests PRIVATE gtest_main Qt6::Core Qt6::Sql Qt6::Network Qt6::NetworkAuth Qt6::Gui Qt6::Widgets spdlog::spdlog)
    
    include(GoogleTest)
    gtest_discover_tests(SmartSchedule_Tests)
endif()

# Structured Logging via spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.12.0
)
FetchContent_MakeAvailable(spdlog)
target_link_libraries(SmartSchedule PRIVATE spdlog::spdlog)

# --- INSTALLATION RULES ---
install(TARGETS SmartSchedule
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

# Modern Qt 6 Deployment (Packages Qt DLLs/Frameworks automatically)
if(QT_VERSION_MAJOR GREATER_EQUAL 6 AND NOT ANDROID)
    qt_generate_deploy_app_script(
        TARGET SmartSchedule
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()

# --- CPACK CONFIGURATION ---
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "SmartSchedule")
set(CPACK_PACKAGE_VENDOR "AIStudio")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Smart Schedule & Google Calendar Manager")
set(CPACK_PACKAGE_VERSION_MAJOR "${SmartSchedule_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SmartSchedule_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SmartSchedule_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

if(WIN32)
    # Generate .msi using WIX toolset
    set(CPACK_GENERATOR "WIX")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "SmartSchedule")
    set(CPACK_WIX_UPGRADE_GUID "12345678-1234-1234-1234-123456789012") # Needs a GUID for MSI
elseif(APPLE)
    # Generate .dmg for macOS
    set(CPACK_GENERATOR "DragNDrop")
elseif(UNIX)
    # Generate .deb for Linux/Ubuntu
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "AIStudio")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) # Auto resolve Linux dependencies
endif()

include(CPack)
target_include_directories(SmartSchedule_Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_sources(SmartSchedule_Tests PRIVATE tests/test_calendar.cpp)
