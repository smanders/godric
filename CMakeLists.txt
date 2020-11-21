cmake_minimum_required(VERSION 3.17)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(preproject)
project(godric VERSION 1.2.0.0)
set(externpro_REV 20.10.1)
find_package(externpro REQUIRED)
xpSetFlags()
xpFindPkg(PKGS boost wxwidgets wxx)
##########
set(CPACK_GENERATOR ZIP)
include(CPack)
##########
set(exe_srcs
  godric.cpp
  )
list(APPEND ${PROJECT_NAME}_srcs ${exe_srcs})
##########
set(resources_srcs
  resources/action_run.png
  resources/filter_add.png
  resources/filter_delete.png
  resources/folder_download.png
  resources/folder_upload.png
  )
list(APPEND ${PROJECT_NAME}_srcs ${resources_srcs})
xpCreateHeaderResource(hrcResources ${resources_srcs})
add_custom_target(${PROJECT_NAME}_hrc DEPENDS ${hrcResources})
##########
add_executable(${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_srcs})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_hrc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}) # hrcResources
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::filesystem wx::aui wxx::things)
install(TARGETS ${PROJECT_NAME} DESTINATION .)
##########
xpSourceDirIgnore(cmake)
xpSourceDirIgnore(test)
xpSourceListAppend("${${PROJECT_NAME}_srcs}")
