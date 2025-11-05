cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# vcpkgのインストール先をプロジェクトルートに固定
set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed" CACHE PATH "vcpkg install directory")

# vcpkgツールチェーンファイルの設定
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(Strecs3D LANGUAGES CXX)

# Qt のMOCを自動有効化
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# OS別設定をインクルード（共通設定より前に）
if(WIN32)
  include(cmake/windows_settings.cmake)
elseif(APPLE)
  include(cmake/macos_settings.cmake)
else()
  message(FATAL_ERROR "Unsupported operating system. Only Windows and macOS are supported.")
endif()

# 共通設定をインクルード
include(cmake/common_settings.cmake)
