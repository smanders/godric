cmake_minimum_required(VERSION 3.17)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(preproject)
project(godric VERSION 1.1.0.0)
set(externpro_REV 20.10.1)
find_package(externpro REQUIRED)
xpSetFlags()
xpFindPkg(PKGS boost wxwidgets)
set(CPACK_GENERATOR ZIP)
include(CPack)
xpCreateHeaderResource(hrcResources
  resources/action_run.png
  resources/filter_add.png
  resources/filter_delete.png
  resources/folder_download.png
  resources/folder_upload.png
  )
add_custom_target(${PROJECT_NAME}_hrc DEPENDS ${hrcResources})
add_executable(${PROJECT_NAME} WIN32 godric.cpp)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_hrc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}) # hrcResources
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::filesystem wx::core)
install(TARGETS ${PROJECT_NAME} DESTINATION .)
