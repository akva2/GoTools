PROJECT(GoImplicitization)


# Include directories

INCLUDE_DIRECTORIES(
  ${GoImplicitization_SOURCE_DIR}/include
  ${GoToolsCore_SOURCE_DIR}/include
  ${GoTools_COMMON_INCLUDE_DIRS}
  )


# Linked in libraries

SET(DEPLIBS
  sisl
  newmat
  GoToolsCore
  )


# Make the GoImplicitization library

FILE(GLOB_RECURSE GoImplicitization_SRCS src/*.C include/*.h)
ADD_LIBRARY(GoImplicitization ${GoImplicitization_SRCS})
TARGET_LINK_LIBRARIES(GoImplicitization ${DEPLIBS})
SET_PROPERTY(TARGET GoImplicitization
  PROPERTY FOLDER "GoImplicitization/Libs")
SET_TARGET_PROPERTIES(GoImplicitization PROPERTIES SOVERSION ${GoTools_ABI_VERSION})



# Apps, examples, tests, ...?
IF(GoTools_COMPILE_APPS)
  FILE(GLOB GoImplicitization_APPS app/*.C)
  FOREACH(app ${GoImplicitization_APPS})
    GET_FILENAME_COMPONENT(appname ${app} NAME_WE)
    ADD_EXECUTABLE(${appname} ${app})
    TARGET_LINK_LIBRARIES(${appname} GoImplicitization ${DEPLIBS})
    SET_TARGET_PROPERTIES(${appname}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY app)
    SET_PROPERTY(TARGET ${appname}
      PROPERTY FOLDER "GoImplicitization/Apps")
  ENDFOREACH(app)

  FILE(GLOB_RECURSE GoImplicitization_EXAMPLES examples/*.C)
  FOREACH(app ${GoImplicitization_EXAMPLES})
    GET_FILENAME_COMPONENT(appname ${app} NAME_WE)
    ADD_EXECUTABLE(${appname} ${app})
    TARGET_LINK_LIBRARIES(${appname} GoImplicitization ${DEPLIBS})
    SET_TARGET_PROPERTIES(${appname}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY examples)
    SET_PROPERTY(TARGET ${appname}
      PROPERTY FOLDER "GoImplicitization/Examples")
  ENDFOREACH(app)
ENDIF(GoTools_COMPILE_APPS)

# Copy data
ADD_CUSTOM_COMMAND(
  TARGET GoImplicitization
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy_directory ${GoImplicitization_SOURCE_DIR}/data 
  ${GoImplicitization_BINARY_DIR}/data
  )


# 'install' target

IF(WIN32)
  # Windows
  # lib
  INSTALL(TARGETS GoImplicitization DESTINATION ${GoTools_INSTALL_PREFIX}/lib)
  # include
  INSTALL(DIRECTORY include/GoTools/implicitization
    DESTINATION ${GoTools_INSTALL_PREFIX}/include/GoTools
    FILES_MATCHING PATTERN "*.h"
    PATTERN ".svn" EXCLUDE
    )
ELSE(WIN32)
  # Linux
  # lib
  INSTALL(TARGETS GoImplicitization DESTINATION lib COMPONENT implicitization)
  # include
  INSTALL(DIRECTORY include/GoTools/implicitization
    DESTINATION include/GoTools
    COMPONENT implicitization-dev
    FILES_MATCHING PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN ".svn" EXCLUDE
    )
ENDIF(WIN32)

SET(CPACK_STRIP_FILES ${CPACK_STRIP_FILES} libGoImplicitization.so)
