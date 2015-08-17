find_package(OpenGL REQUIRED) # GLU
find_package(X11 REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OPENGL_LIBRARY} ${GLEW_LIBRARY} ${X11_LIBRARIES})

set(PROGRAM glx)
add_executable(${PROGRAM} ${COMMON_SOURCES} main.cpp)
target_link_libraries(${PROGRAM} ${LIBS})

set(PROGRAM texture_storage_fbo_crash)
add_executable(${PROGRAM} ${COMMON_SOURCES} texture_storage_fbo_crash.cpp)
target_link_libraries(${PROGRAM} ${LIBS})
