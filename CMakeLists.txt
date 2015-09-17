cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project      (strokeRender)

# OPENGL
find_package (OpenGL REQUIRED)
find_package (GLUT REQUIRED)
find_package (GLEW REQUIRED)

link_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

file(GLOB project_SOURCES "*.cpp")

ADD_EXECUTABLE  (strokeRender ${project_SOURCES})

TARGET_LINK_LIBRARIES (strokeRender ${OPENGL_LIBRARIES}
                                      ${GLUT_LIBRARIES}
                                      ${GLEW_LIBRARIES})

