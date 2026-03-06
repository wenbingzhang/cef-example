// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "resource_util.h"

#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

#include "include/base/cef_logging.h"

#pragma comment(lib, "shlwapi.lib")

namespace shared {

namespace {

bool FileExists(const char* path) {
  FILE* f = fopen(path, "rb");
  if (f) {
    fclose(f);
    return true;
  }
  return false;
}

bool ReadFileToString(const char* path, std::string& data) {
  // Implementation adapted from base/file_util.cc
  FILE* file = fopen(path, "rb");
  if (!file)
    return false;

  char buf[1 << 16];
  size_t len;
  while ((len = fread(buf, 1, sizeof(buf), file)) > 0)
    data.append(buf, len);
  fclose(file);

  return true;
}

}  // namespace

// Retrieve the directory containing resource files on Windows.
// Returns the path to the "resources" subdirectory next to the executable.
// Uses forward slashes for path separators which are supported on Windows.
bool GetResourceDir(std::string& dir) {
  // Use a larger buffer to support long paths on Windows 10+
  char path[MAX_PATH * 2];

  // Get the path to the current executable.
  if (GetModuleFileNameA(NULL, path, sizeof(path)) == 0) {
    return false;
  }

  // Remove the executable name to get the directory.
  PathRemoveFileSpecA(path);

  // Append "resources" subdirectory.
  // Note: Windows APIs accept forward slashes as path separators,
  // which provides better cross-platform consistency.
  dir = std::string(path);
  dir.append("/resources");
  return true;
}

bool GetResourceString(const std::string& resource_path,
                       std::string& out_data) {
  std::string path;
  if (!GetResourceDir(path))
    return false;

  // Use forward slash for path separator (cross-platform compatible)
  path.append("/");
  path.append(resource_path);

  return ReadFileToString(path.c_str(), out_data);
}

CefRefPtr<CefStreamReader> GetResourceReader(const std::string& resource_path) {
  std::string path;
  if (!GetResourceDir(path))
    return nullptr;

  // Use forward slash for path separator (cross-platform compatible)
  path.append("/");
  path.append(resource_path);

  if (!FileExists(path.c_str()))
    return nullptr;

  return CefStreamReader::CreateForFile(path);
}

}  // namespace shared
