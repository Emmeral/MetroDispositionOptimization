set(BINARY ${CMAKE_PROJECT_NAME})

set(BOOST_INCLUDE_LIBRARIES program_options)
set(BOOST_ENABLE_CMAKE ON)


Include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 9.1.0 # or a later release
)

FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.81.0/boost-1.81.0.7z # downloading a zip release speeds up the download
        USES_TERMINAL_DOWNLOAD TRUE
        GIT_PROGRESS TRUE
        DOWNLOAD_NO_EXTRACT FALSE
)

FetchContent_MakeAvailable(Boost)



file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(SOURCES ${SOURCES})


add_executable(${BINARY}_run ${SOURCES})
add_library(${BINARY}_lib STATIC ${SOURCES})

list(APPEND Name run lib)
list(APPEND Link PRIVATE PUBLIC)

foreach (i IN ZIP_LISTS Name Link)
    target_link_libraries(${BINARY}_${i_0} ${i_1} fmt::fmt Boost::program_options)
endforeach ()
