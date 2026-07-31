cmake_minimum_required(VERSION 3.16)
project(OCCT_Industrial VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic Meta-Object Compiler (MOC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ✅ 1. ADDED Qml and Quick3D to find_package
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL WebSockets Qml Quick3D)
find_package(OpenCASCADE REQUIRED)

# Tell the compiler where to find the Eigen3 math headers for KDL
include_directories(/usr/include/eigen3)

qt_add_executable(appOCCT
    main.cpp
    OcctWidget.h OcctWidget.cpp
    MainWindow.h MainWindow.cpp
    LeftPanel.h LeftPanel.cpp
    RightPanel.h RightPanel.cpp



    LineGeometry.h LineGeometry.cpp


    scurve.h scurve.cpp
    kinematic.h kinematic.cpp
    ClientBackend.h ClientBackend.cpp
    ur_kinematics.h






)
# ✅ 2. ADDED Qt6::Qml and Qt6::Quick3D to target_link_libraries
target_link_libraries(appOCCT
    PRIVATE
    # --- Qt Libraries ---
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::WebSockets
    Qt6::Qml
    Qt6::Quick3D

    # --- OCCT Core & Math ---
    TKernel
    TKMath

    # --- OCCT Geometry & Topology ---
    TKG2d
    TKG3d
    TKGeomBase
    TKGeomAlgo
    TKBRep
    TKTopAlgo
    TKPrim
    TKBO
    TKShHealing

    # --- OCCT Mesh & Visualization ---
    TKMesh
    TKRWMesh
    TKService
    TKV3d
    TKOpenGl

    # --- OCCT Basic Data Exchange (STL, STEP, IGES) ---
    TKSTL
    TKXSBase
    TKSTEPBase
    TKSTEPAttr
    TKSTEP209
    TKSTEP
    TKIGES

    # --- OCCT XDE (Extended Data Exchange for Colors & Assemblies) ---
    TKCAF        # TDocStd_Document
    TKLCAF       # Lite CAF (Labels, Trees)
    TKVCAF       # XCAFPrs_AISObject (Viewer integration)
    TKXCAF       # XCAFApp, XCAFDoc_ShapeTool, XCAFDoc_ColorTool
    TKXDESTEP    # Specialized XDE STEP Reader

    # --- External Libraries ---
    orocos-kdl
)


target_include_directories(appOCCT PRIVATE ${OpenCASCADE_INCLUDE_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/step" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")