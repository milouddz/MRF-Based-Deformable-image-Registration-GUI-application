cmake_minimum_required(VERSION 3.18)
project(qt_cuda)

find_package(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
find_package(CUDA REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenMP REQUIRED)
find_package(OpenGL REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS "-lz")
set (CMAKE_CXX_FLAGS "-O3")
set (CMAKE_CXX_FLAGS "-mavx2")
set (CMAKE_CXX_FLAGS "-msse4.2")
set (CMAKE_CXX_FLAGS "-fopenmp")
set(SOURCES
    main.cpp
    mainwindow.ui
    mainwindow.cpp
    mainwindow.h
    cuda_kernel.cu
    imageIOgzType.h
    transformations.h
    primsMST.h
    regularisation.h
    MINDSSCbox.h
    dataCostD.h
    parseArguments.h
    vertex_shader.h
    fragment_shader.h
    shaders.cpp
    shaders.h
    images.qrc

)

set(UI_FILES
    mainwindow.ui
)

set(SKIP_AUTOMOC "mainwindow.cpp")  # Use the filename you want to skip

cuda_add_executable(qt_cuda ${SOURCES} ${UI_HEADERS})
target_link_libraries(qt_cuda ZLIB::ZLIB Qt5::Core Qt5::Gui Qt5::OpenGL ${OPENGL_LIBRARIES} ${OpenMP_CXX_FLAGS} Qt5::Widgets ${ZLIB_LIBRARIES})

#if(OpenMP_CXX_FOUND)
#    target_compile_options(qt_cuda PRIVATE ${OpenMP_CXX_FLAGS})
#endif()

