cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project (strokeRender)

# OPENGL
find_package (OpenGL REQUIRED)
find_package (GLUT REQUIRED)

include_directories (${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

file (GLOB project_SOURCES "*.cpp")

add_executable (strokeRender ${project_SOURCES})

target_link_libraries (strokeRender ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

