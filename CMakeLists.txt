cmake_minimum_required(VERSION 4.3)
project(godric)
xpSetFlags()
xpFindPkg(PKGS boost wxx)
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
  )
list(APPEND ${PROJECT_NAME}_srcs ${resources_srcs})
xpCreateHeaderResource(hrcResources ${resources_srcs})
add_custom_target(${PROJECT_NAME}_hrc DEPENDS ${hrcResources})
##########
add_executable(${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_srcs})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_hrc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}) # hrcResources
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::filesystem wx::aui wxx::things)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
##########
set(targetsFile ${PROJECT_NAME}-targets)
if(COMMAND xpExternPackage)
  xpExternPackage(TARGETS_FILE ${targetsFile}
    EXE ${PROJECT_NAME} FIND_XPRO_CMAKE
    BASE v1.0 XPDIFF "intro"
    WEB "https://github.com/smanders/godric"
    DESC "tool for sorting files into structured directories based on filename patterns"
    )
elseif(NOT DEFINED CMAKE_INSTALL_CMAKEDIR)
  set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATADIR}/cmake)
endif()
install(TARGETS ${PROJECT_NAME} EXPORT ${targetsFile}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
install(EXPORT ${targetsFile} DESTINATION ${CMAKE_INSTALL_CMAKEDIR} NAMESPACE ${PROJECT_NAME}::)
##########
xpSourceDirIgnore(test)
xpSourceListAppend("${${PROJECT_NAME}_srcs}")
