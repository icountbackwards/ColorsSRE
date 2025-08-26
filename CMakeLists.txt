cmake_minimum_required(VERSION 3.10)

project(ColorsSRE C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add executable with explicit sources
add_executable(${PROJECT_NAME}
    src/main.c
    src/math_utils.c
)

# Include project headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/pipeline
)