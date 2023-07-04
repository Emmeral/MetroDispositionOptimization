


set(BINARY generator)

file(GLOB_RECURSE GENERATOR_SOURCES LIST_DIRECTORIES false *.h *.cpp)
set(SOURCES ${GENERATOR_SOURCES})

add_executable(${BINARY} ${GENERATOR_SOURCES})

target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib)
