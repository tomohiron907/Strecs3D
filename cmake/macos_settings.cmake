# macOS specific settings for Strecs3D

# macOS用のコンパイラフラグ設定
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "@executable_path;${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")

# macOS用の設定を適用する関数
function(apply_macos_settings TARGET_NAME)
  # macOS用のリンクライブラリ設定（vcpkgから取得したライブラリを使用）
  target_link_libraries(${TARGET_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${VTK_LIBRARIES}
    lib3mf::lib3mf
    libzip::zip
  )

  # macOSでのQtプラグインパス設定
  set_target_properties(${TARGET_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
  )
endfunction()

# Qtプラグインパスを設定（vcpkgのQtを使用する場合）
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
  # フォールバック: HomebrewのQtパス
  set(QT_PLUGIN_PATH "/opt/homebrew/Cellar/qt/6.9.1/share/qt/plugins")
  if(EXISTS "${QT_PLUGIN_PATH}")
    message(STATUS "Found Qt plugins at: ${QT_PLUGIN_PATH}")
  else()
    # 代替パスを試す
    set(QT_PLUGIN_PATH "/opt/homebrew/share/qt/plugins")
    if(EXISTS "${QT_PLUGIN_PATH}")
      message(STATUS "Found Qt plugins at: ${QT_PLUGIN_PATH}")
    else()
      message(WARNING "Qt plugins not found. Please set QT_PLUGIN_PATH manually.")
    endif()
  endif()
endif() 