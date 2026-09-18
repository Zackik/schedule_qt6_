import re

with open('CMakeLists.txt', 'r') as f:
    text = f.read()

# Fix find_package Qt6
text = re.sub(r'find_package\(Qt6 6.2 REQUIRED COMPONENTS.*?(\r?\n|\r)', 'find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network NetworkAuth Sql PrintSupport Qml Quick QuickWidgets)\\1', text)

# Fix tests executable
tests_target = """    add_executable(SmartSchedule_Tests 
        tests/test_main.cpp 
        tests/test_resources.cpp 
        src/services/ResourceMonitor.cpp 
        src/managers/StorageManager.cpp
    )
    target_link_libraries(SmartSchedule_Tests PRIVATE gtest_main Qt6::Core)"""

text = re.sub(r'add_executable\(SmartSchedule_Tests tests/test_main\.cpp\)\s*target_link_libraries\(SmartSchedule_Tests PRIVATE gtest_main\)', tests_target, text)

with open('CMakeLists.txt', 'w') as f:
    f.write(text)
