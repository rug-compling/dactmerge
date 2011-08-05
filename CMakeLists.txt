cmake_minimum_required(VERSION 2.6)
project(DactMerge)

set(CMAKE_CXX_FLAGS "-pthread")

if(APPLE)
  set(PROGNAME DactMerge)
  set(CMAKE_OSX_ARCHITECTURES "x86_64")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.6")
else(APPLE)
  set(PROGNAME dactmerge)
endif(APPLE)

list(APPEND CMAKE_MODULE_PATH "${DactMerge_SOURCE_DIR}/cmake")

include_directories(${DactMerge_SOURCE_DIR}/include)
include_directories(${DactMerge_BINARY_DIR})

find_package(AlpinoCorpus REQUIRED)
if(ALPINOCORPUS_FOUND)
        INCLUDE_DIRECTORIES(${ALPINOCORPUS_INCLUDE_DIR})
endif(ALPINOCORPUS_FOUND)

find_package(Qt4 COMPONENTS QtCore QtGui QtMain QtNetwork REQUIRED)
INCLUDE(${QT_USE_FILE})

set(merge_SRCS
  src/MainWindow.cpp
  src/main.cpp
)

set(merge_MOC_HDRS
  include/MainWindow.hh
 )

set(merge_UI
  ui/MainWindow.ui
)

qt4_wrap_cpp(merge_MOC_SRCS ${merge_MOC_HDRS})
qt4_wrap_ui(merge_UI_HDRS ${merge_UI})

add_executable(${PROGNAME} MACOSX_BUNDLE WIN32
  ${merge_SRCS}
  ${merge_HDRS}
  ${merge_MOC_SRCS}
  ${merge_UI_HDRS}
  )

target_link_libraries(${PROGNAME} ${QT_LIBRARIES})
target_link_libraries(${PROGNAME} ${ALPINOCORPUS_LIBRARIES})

install(TARGETS ${PROGNAME} 
  BUNDLE DESTINATION . COMPONENT Runtime
  RUNTIME DESTINATION bin COMPONENT Runtime) 

# OS X bundle creation
if(APPLE)
  set(APP ${CMAKE_INSTALL_PREFIX}/${PROGNAME}.app)

  get_filename_component(ALPINOCORPUS_LIBRARY_DIR
    ${ALPINOCORPUS_LIBRARIES} PATH)

  set(DIRS
    ${QT_LIBRARY_DIR}
    ${ALPINOCORPUS_LIBRARY_DIR})

  set(MACOSX_BUNDLE_GUI_IDENTIFIER "nl.rug.Dact")
  set(MACOSX_BUNDLE_ICON_FILE "dact-espresso.icns")
  set(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION} )

  set_target_properties(${PROGNAME}
    PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${DactMerge_SOURCE_DIR}/resources/Info.plist)

  INSTALL(CODE "
    file(WRITE \"${APP}/Contents/Resources/qt.conf\")
    file(MAKE_DIRECTORY \"${APP}/Contents/Resources/qt_menu.nib\")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
      \"${QT_LIBRARY_DIR}/QtGui.framework/Resources/qt_menu.nib\"
      \"${APP}/Contents/Resources/qt_menu.nib\")
    include(BundleUtilities)
    fixup_bundle(\"${APP}\" \"\" \"${DIRS}\")
  " COMPONENT Runtime)

  set(CPACK_BINARY_DRAGNDROP ON)
  include(CPack)

endif(APPLE)
