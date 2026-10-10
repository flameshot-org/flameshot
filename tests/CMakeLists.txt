find_package(Qt6 REQUIRED COMPONENTS Core Test)

set(CMAKE_AUTOMOC ON)

# The unit under test is pure QtCore, so it is compiled directly rather than
# linked against the flameshot executable target.
add_executable(tst_monitorfocus
  tst_monitorfocus.cpp
  ${CMAKE_SOURCE_DIR}/src/utils/monitorfocus.cpp
)

target_include_directories(tst_monitorfocus PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_monitorfocus PRIVATE Qt6::Core Qt6::Test)

add_test(NAME monitorfocus COMMAND tst_monitorfocus)

add_executable(tst_dprscaling
  tst_dprscaling.cpp
  ${CMAKE_SOURCE_DIR}/src/utils/dprscaling.cpp
)

target_include_directories(tst_dprscaling PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_dprscaling PRIVATE Qt6::Core Qt6::Test)

add_test(NAME dprscaling COMMAND tst_dprscaling)

add_executable(tst_resizehandles
  tst_resizehandles.cpp
  ${CMAKE_SOURCE_DIR}/src/utils/resizehandles.cpp
)

target_include_directories(tst_resizehandles PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_resizehandles PRIVATE Qt6::Core Qt6::Test)

add_test(NAME resizehandles COMMAND tst_resizehandles)
