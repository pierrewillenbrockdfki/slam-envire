# compile shared library
rock_find_qt4(OPTIONAL)
rock_find_qt5(OPTIONAL)

if (ROCK_QT_VERSION_4)
rock_vizkit_plugin(envire-viz
    ElevationGridVisualization.cpp
    EnvireEventListener.cpp
    EnvireVisualization.cpp
    FrameNodeManipulator.cpp
    FrameNodeVisualization.cpp
    GridVisualizationBase.cpp
    ImageRGB24Visualization.cpp
    GridVisualization.cpp
    LaserScanVisualization.cpp
    MapSegmentVisualization.cpp
    MLSVisualization.cpp
    PointcloudVisualization.cpp
    TriMeshVisualization.cpp
    TreeViewListener.cpp
    ItemManipulator.cpp
    TraversabilityGridVisualization.cpp
    MOC ItemManipulator.hpp TreeViewListener.hpp EnvireVisualization.hpp MLSVisualization.hpp PointcloudVisualization.hpp EnvireEventListener.hpp ElevationGridVisualization.hpp MapSegmentVisualization.hpp FrameNodeVisualization.hpp GridVisualization.hpp
    DEPS envire
    DEPS_PKGCONFIG base-viz
    HEADERS EnvireVisualization.hpp EnvireEventListener.hpp TreeViewListener.hpp EnvireWidget.hpp
    EnvironmentItemVisualizer.hpp)
target_link_libraries(envire-viz ${Boost_THREAD_LIBRARY})
target_link_libraries(envire-viz pthread)

rock_executable(envireViewer envireViewer.cpp
    DEPS envire-viz
    DEPS_PKGCONFIG vizkit3d
    LIBS Qt4::QtGui)
endif()

if (ROCK_QT_VERSION_5)
rock_vizkit_plugin_qt5(envire-viz-qt5
    ElevationGridVisualization.cpp
    EnvireEventListener.cpp
    EnvireVisualization.cpp
    FrameNodeManipulator.cpp
    FrameNodeVisualization.cpp
    GridVisualizationBase.cpp
    ImageRGB24Visualization.cpp
    GridVisualization.cpp
    LaserScanVisualization.cpp
    MapSegmentVisualization.cpp
    MLSVisualization.cpp
    PointcloudVisualization.cpp
    TriMeshVisualization.cpp
    TreeViewListener.cpp
    ItemManipulator.cpp
    TraversabilityGridVisualization.cpp
    MOC5 ItemManipulator.hpp TreeViewListener.hpp EnvireVisualization.hpp MLSVisualization.hpp PointcloudVisualization.hpp EnvireEventListener.hpp ElevationGridVisualization.hpp MapSegmentVisualization.hpp FrameNodeVisualization.hpp GridVisualization.hpp
    DEPS envire
    DEPS_PKGCONFIG base-viz-qt5
    HEADERS EnvireVisualization.hpp EnvireEventListener.hpp TreeViewListener.hpp EnvireWidget.hpp
    EnvironmentItemVisualizer.hpp)
target_link_libraries(envire-viz-qt5 ${Boost_THREAD_LIBRARY})
target_link_libraries(envire-viz-qt5 pthread)

rock_executable(envireViewer-qt5 envireViewer.cpp
    DEPS envire-viz-qt5
    DEPS_PKGCONFIG vizkit3d-qt5
    LIBS Qt5::Core Qt5::Gui)
endif()

add_subdirectory(enview)
