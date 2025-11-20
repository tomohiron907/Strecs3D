# macOS specific settings for Strecs3D

# vcpkgのみを使用するようにパス設定（Homebrewのパスを無視）
# CMAKE_PREFIX_PATHをvcpkgのみに制限（プロジェクトルートのvcpkg_installedを使用）
set(CMAKE_PREFIX_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" CACHE PATH "" FORCE)

# find_packageでシステム環境変数のパス（Homebrewなど）を無視
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)

# Homebrewのパスを明示的に除外
list(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH "/usr/local" "/opt/homebrew" "/opt/local")
list(REMOVE_ITEM CMAKE_PREFIX_PATH "/usr/local" "/opt/homebrew" "/opt/local")

# OpenGL/GLUの検索でHomebrewを無視
set(CMAKE_IGNORE_PATH "/opt/homebrew" "/opt/homebrew/lib" "/opt/homebrew/include"
    "/usr/local" "/usr/local/lib" "/usr/local/include")

# macOSのフレームワークを優先
set(CMAKE_FIND_FRAMEWORK FIRST)
set(OpenGL_GL_PREFERENCE LEGACY)

# macOS用のコンパイラフラグ設定
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "@executable_path;${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")

# macOS用の設定を適用する関数
function(apply_macos_settings TARGET_NAME)
  # OpenCASCADEのインクルードディレクトリを追加
  target_include_directories(${TARGET_NAME} PRIVATE ${OpenCASCADE_INCLUDE_DIR})

  # macOS用のリンクライブラリ設定（vcpkgから取得したライブラリを使用）
  target_link_libraries(${TARGET_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${VTK_LIBRARIES}
    lib3mf::lib3mf
    libzip::zip
    $<IF:$<TARGET_EXISTS:gmsh::shared>,gmsh::shared,gmsh::lib>
    ${OpenCASCADE_LIBRARIES}
    nlohmann_json::nlohmann_json
  )

  # macOSでのQtプラグインパス設定
  set_target_properties(${TARGET_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
  )
endfunction()

# Qtプラグインパスを設定（vcpkgのQtを使用）
# vcpkgでインストールしたQtのプラグインパスを自動検出
if(Qt6_DIR)
  get_filename_component(QT_INSTALL_PREFIX "${Qt6_DIR}/../../../" ABSOLUTE)
  set(QT_PLUGIN_PATH "${QT_INSTALL_PREFIX}/plugins")
  if(EXISTS "${QT_PLUGIN_PATH}")
    message(STATUS "Found Qt plugins at: ${QT_PLUGIN_PATH}")
  else()
    message(WARNING "Qt plugins not found at ${QT_PLUGIN_PATH}")
  endif()
else()
  message(WARNING "Qt6_DIR not set. Please ensure Qt6 is installed via vcpkg.")
endif() 