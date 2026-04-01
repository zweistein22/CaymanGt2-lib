cmake_minimum_required(VERSION 3.10)

# 1. Name your project
project(NextionFW2Device VERSION 1.0 LANGUAGES CXX)

# 2. Set the C++ standard (C++20 is great for 2026)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. Add your source file
# Replace 'main.cpp' with your actual filename if it's different
add_executable(NextionFW2Device nextionfw2device.cpp)

# 4. Optional: Add compiler-specific flags
if(MSVC)

    # This fixes the C2664 error by forcing Unicode support
    add_compile_definitions(UNICODE _UNICODE)
    
    # "Permissive" flags to make MSVC behave more like older compilers
    # /permissive- is usually ON by default in newer VS versions; 
    # we can explicitly set it to allow some non-conforming code.
    target_compile_options(NextionFW2Device PRIVATE /permissive)

    # Windows-specific flags: extra warnings, exception handling
    target_compile_options(NextionFW2Device PRIVATE /W4 /EHsc)
else()
    # Linux/GCC/Clang-specific flags
    target_compile_options(NextionFW2Device PRIVATE -Wall -Wextra)
endif()
