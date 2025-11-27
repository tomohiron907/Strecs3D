# Windows specific settings for Strecs3D

# Windows環境でのvcpkgサポート
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

# Windows用のコンパイラフラグ設定
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

# MinGW環境でのQt6設定
if(MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_DEPRECATED_WARNINGS")
endif()

# Windows環境でのlib3mf設定（common_settings.cmakeで検索されるため、ここでは設定のみ）
set(LIB3MF_ROOT "$ENV{USERPROFILE}/libs/lib3mf/lib3mf-2.3.2-Windows" CACHE PATH "Path to lib3mf installation")
set(LIB3MF_LIB lib3mf::lib3mf)

# Windows環境でのlibzip設定
find_package(libzip CONFIG QUIET)
if(libzip_FOUND)
  message(STATUS "Found libzip via vcpkg")
  set(LIBZIP_LIBRARIES libzip::zip)
  set(LIBZIP_INCLUDE_DIRS "")
else()
  # 従来の方法で検索
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBZIP REQUIRED libzip)
  message(STATUS "Found libzip version: ${LIBZIP_VERSION}")
  
  if(LIBZIP_LIBRARY_DIRS)
    link_directories(${LIBZIP_LIBRARY_DIRS})
  endif()
endif()

# Windows用の設定を適用する関数
function(apply_windows_settings TARGET_NAME)
  # Windows用のインクルードパス設定
  # lib3mfはcommon_settings.cmakeで検索済みのため、vcpkgの設定を使用
  # OpenCASCADEのインクルードディレクトリを追加
  target_include_directories(${TARGET_NAME} PRIVATE ${OpenCASCADE_INCLUDE_DIR})

  if(NOT libzip_FOUND)
    target_include_directories(${TARGET_NAME} PRIVATE
      ${LIBZIP_INCLUDE_DIRS}
    )
  endif()

  # Windows用のリンクライブラリ設定
  target_link_libraries(${TARGET_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${VTK_LIBRARIES}
    ${LIB3MF_LIB}
    ${LIBZIP_LIBRARIES}
    pugixml::pugixml
    $<IF:$<TARGET_EXISTS:gmsh::shared>,gmsh::shared,gmsh::lib>
    ${OpenCASCADE_LIBRARIES}
    nlohmann_json::nlohmann_json
  )

  # MinGW環境での追加設定
  if(MINGW)
    # MinGW用のQt6ライブラリパス設定
    if(Qt6Core_DIR)
      get_filename_component(QT_LIB_DIR "${Qt6Core_DIR}/../" ABSOLUTE)
      message(STATUS "Qt6 library directory: ${QT_LIB_DIR}")
    endif()
    
    # MinGW用のリンクフラグ
    set_target_properties(${TARGET_NAME} PROPERTIES
      LINK_FLAGS "-static-libgcc -static-libstdc++"
    )
  endif()

  # Windows環境での設定
  # Debug時はコンソールを表示、Release時はGUIアプリとして実行
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${TARGET_NAME} PROPERTIES
      WIN32_EXECUTABLE TRUE
      VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/$<CONFIG>;$ENV{PATH}"
    )
  else()
    set_target_properties(${TARGET_NAME} PROPERTIES
      WIN32_EXECUTABLE FALSE
      VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/$<CONFIG>;$ENV{PATH}"
    )
  endif()
endfunction()

# Qtプラグインパスの設定
if(Qt6_DIR)
  get_filename_component(QT_ROOT "${Qt6_DIR}/../../../" ABSOLUTE)
  set(QT_PLUGIN_PATH "${QT_ROOT}/plugins")
  message(STATUS "Qt plugins path: ${QT_PLUGIN_PATH}")
endif() 