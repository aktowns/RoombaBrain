cmake_minimum_required(VERSION 3.10)
project(RoombaBrain)

include(CMakeListsGenerated.txt)

add_custom_target(
  BUILD ALL 
  COMMAND make
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

FILE(GLOB_RECURSE SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/main/*.*")
add_executable(${PROJECT_NAME} ${SRC_LIST})

