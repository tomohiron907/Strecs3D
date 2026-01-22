# Common settings for Strecs3D

# 未指定の場合はDebugビルドにする（デバッグ情報が付加される）
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulting to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkgの設定確認
if(DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
else()
  message(WARNING "vcpkg toolchain not found. Please set VCPKG_ROOT environment variable.")
endif()

# Qt6の検索
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Qt6の詳細設定
if(Qt6_FOUND)
  message(STATUS "Qt6 found: ${Qt6_VERSION}")
  message(STATUS "Qt6 Core: ${Qt6Core_DIR}")
  message(STATUS "Qt6 Widgets: ${Qt6Widgets_DIR}")
else()
  message(FATAL_ERROR "Qt6 not found. Please install Qt6 development libraries.")
endif()

# vcpkgからlibzipを検索
find_package(libzip REQUIRED)
if(libzip_FOUND)
  message(STATUS "libzip found: ${libzip_VERSION}")
else()
  message(FATAL_ERROR "libzip not found. Please install via vcpkg: vcpkg install libzip")
endif()

# vcpkgからlib3mfを検索
find_package(lib3mf REQUIRED)
if(lib3mf_FOUND)
  message(STATUS "lib3mf found: ${lib3mf_VERSION}")
else()
  message(FATAL_ERROR "lib3mf not found. Please install via vcpkg: vcpkg install lib3mf")
endif()

# vcpkgからpugixmlを検索
find_package(pugixml REQUIRED)
if(pugixml_FOUND)
  message(STATUS "pugixml found")
else()
  message(FATAL_ERROR "pugixml not found. Please install via vcpkg: vcpkg install pugixml")
endif()

# vcpkgからgmshを検索
# Updated to use 'gmsh' package name (changed from unofficial-gmsh in newer vcpkg versions)
find_package(gmsh CONFIG REQUIRED)
if(gmsh_FOUND)
  message(STATUS "gmsh found: ${gmsh_VERSION}")
else()
  message(FATAL_ERROR "gmsh not found. Please install via vcpkg: vcpkg install gmsh")
endif()

# vcpkgからOpenCASCADEを検索
find_package(OpenCASCADE CONFIG REQUIRED)
if(OpenCASCADE_FOUND)
  message(STATUS "OpenCASCADE found: ${OpenCASCADE_VERSION}")
  message(STATUS "OpenCASCADE include dirs: ${OpenCASCADE_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "OpenCASCADE not found. Please install via vcpkg: vcpkg install opencascade")
endif()

# vcpkgからnlohmann_jsonを検索（FEM用）
find_package(nlohmann_json 3.2.0 REQUIRED)
if(nlohmann_json_FOUND)
  message(STATUS "nlohmann_json found: ${nlohmann_json_VERSION}")
else()
  message(FATAL_ERROR "nlohmann_json not found. Please install via vcpkg: vcpkg install nlohmann-json")
endif()

# vcpkgからVTKを検索（Qtサポート付き）
find_package(VTK REQUIRED
  COMPONENTS
    CommonCore
    CommonDataModel
    FiltersCore
    FiltersSources
    FiltersGeometry
    FiltersGeneral
    FiltersModeling
    FiltersExtraction
    FiltersHybrid
    FiltersProgrammable
    FiltersTopology
    FiltersVerdict
    IOXML
    IOImage
    IOGeometry
    IOLegacy
    ImagingCore
    ImagingGeneral
    ImagingHybrid
    ImagingMath
    ImagingSources
    RenderingCore
    RenderingOpenGL2
    RenderingFreeType
    RenderingVolume
    RenderingAnnotation
    RenderingContext2D
    RenderingSceneGraph
    RenderingUI
    RenderingVtkJS
    InteractionStyle
    InteractionWidgets
    GUISupportQt
    GUISupportQtSQL
)

if(VTK_FOUND)
  message(STATUS "VTK found: ${VTK_VERSION}")
  include(${VTK_USE_FILE})
else()
  message(FATAL_ERROR "VTK not found. Please install via vcpkg: vcpkg install vtk[qt]")
endif()

# 実行可能ファイルの生成
add_executable(Strecs3D
  main.cpp
  mainwindow.cpp
  UI/mainwindowui.cpp

  UI/widgets/DensitySlider.cpp
  UI/widgets/StressRangeWidget.cpp
  UI/ColorManager.cpp
  UI/widgets/Button.cpp

  UI/widgets/ObjectDisplayOptionsWidget.cpp
  UI/widgets/DisplayOptionsContainer.cpp
  UI/widgets/ObjectListWidget.cpp
  UI/widgets/PropertyWidget.cpp
  UI/widgets/properties/StepPropertyWidget.cpp
  UI/widgets/properties/ConstraintPropertyWidget.cpp
  UI/widgets/properties/LoadPropertyWidget.cpp
  UI/widgets/process/ProcessFlowWidget.cpp
  UI/widgets/process/ProcessManagerWidget.cpp
  UI/widgets/process/RollbackConfirmationDialog.cpp
  UI/widgets/process/AddConstraintDialog.cpp
  UI/widgets/process/AddLoadDialog.cpp
  UI/widgets/process/SelectBedSurfaceDialog.cpp
  UI/widgets/process/steps/ImportStepWidget.cpp
  UI/widgets/process/steps/BoundaryConditionStepWidget.cpp
  UI/widgets/process/steps/SimulationStepWidget.cpp
  UI/widgets/process/steps/InfillStepWidget.cpp
  UI/widgets/CustomCheckBox.cpp
  UI/visualization/SceneRenderer.cpp
  UI/visualization/TurntableInteractorStyle.cpp
  UI/visualization/StepPickerStyle.cpp
  core/processing/VtkProcessor.cpp
  core/processing/StepReader.cpp
  core/processing/StepToStlConverter.cpp
  core/processing/StepTransformer.cpp
  core/processing/3mf/BaseLib3mfProcessor.cpp
  core/processing/3mf/slicers/cura/CuraLib3mfProcessor.cpp
  core/processing/3mf/slicers/bambu/BambuLib3mfProcessor.cpp
  core/processing/3mf/slicers/prusa/PrusaLib3mfProcessor.cpp
  utils/fileUtility.cpp
  utils/tempPathUtility.cpp
  utils/xmlConverter.cpp
  core/application/ApplicationController.cpp
  core/application/MainWindowUIAdapter.cpp
  core/interfaces/IUserInterface.cpp
  core/processing/ProcessPipeline.cpp
  core/ui/UIState.cpp
  UI/visualization/VisualizationManager.cpp
  UI/visualization/ActorFactory.cpp
  core/export/ExportManager.cpp
  core/processing/3mf/slicers/prusa/ModelConverter.cpp
  FEM/SimulationConditionExporter.cpp
  FEM/fem_pipeline.cpp
  FEM/frd2vtu.cpp
  FEM/step2inp.cpp
  FEM/step2inp/MeshGenerator.cpp
  FEM/step2inp/ConstraintSetter.cpp
  FEM/step2inp/MaterialSetter.cpp
  FEM/step2inp/LoadConditionSetter.cpp
  FEM/step2inp/InpWriter.cpp
  FEM/simulation_config.cpp
  resources/resources.qrc
)

# OS別の設定を適用
if(WIN32)
  apply_windows_settings(Strecs3D)
elseif(APPLE)
  apply_macos_settings(Strecs3D)
endif()

# VTK 自動初期化設定 (VTK バージョンが 8.90.0 以上の場合)
if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(
    TARGETS Strecs3D
    MODULES ${VTK_LIBRARIES}
  )
endif()