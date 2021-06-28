
# Search for SimGrid
include_directories("${SimGrid_INCLUDE_DIR}" SYSTEM)

INCLUDE_DIRECTORIES(../Elasticity)
INCLUDE_DIRECTORIES(SYSTEM /home/clem/.hunter/_Base/d45d77d/4430a64/3b7ee27/Install/include)
LINK_DIRECTORIES(
  /home/clem/.hunter/_Base/d45d77d/4430a64/3b7ee27/Install/lib
  ~/Code/framagit.org/simgridOfficiel/simgrid//build3.27/lib)

if(USE_JAEGERTRACING)
  list(APPEND EXTRA_LIBS  jaegertracing opentracing yaml-cppd)

endif()

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../Elasticity/config.yml
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE( dataSourceTest DataSourceTest.cpp )
TARGET_LINK_LIBRARIES(dataSourceTest Elasticity simgrid ${EXTRA_LIBS})

ADD_EXECUTABLE( testDoc testDoc.cpp )
TARGET_LINK_LIBRARIES(testDoc Elasticity simgrid ${EXTRA_LIBS})

ADD_EXECUTABLE( bench1 bench1.cpp )
TARGET_LINK_LIBRARIES(bench1 Elasticity simgrid ${EXTRA_LIBS})

ADD_EXECUTABLE( bench2 bench2.cpp )
TARGET_LINK_LIBRARIES(bench2 Elasticity simgrid ${EXTRA_LIBS})

ADD_EXECUTABLE( testReqTypes testReqTypes.cpp )
TARGET_LINK_LIBRARIES(testReqTypes Elasticity simgrid ${EXTRA_LIBS})

ADD_EXECUTABLE(test2req test2req.cpp )
TARGET_LINK_LIBRARIES(test2req Elasticity simgrid ${EXTRA_LIBS})

ADD_EXECUTABLE(teastore_login teastore_login.cpp )
TARGET_LINK_LIBRARIES(teastore_login Elasticity simgrid ${EXTRA_LIBS})

ADD_EXECUTABLE(generated_2inst generated_2inst.cpp )
TARGET_LINK_LIBRARIES(generated_2inst Elasticity simgrid ${EXTRA_LIBS})
