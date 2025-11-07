// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "resource_util.h"

#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

#include "include/base/cef_logging.h"
#include "include/wrapper/cef_stream_resource_handler.h"

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

// Provider implementation for loading file resources from the filesystem.
class FileResourceProvider : public CefResourceManager::Provider {
 public:
  explicit FileResourceProvider(const std::string& root_url)
      : root_url_(root_url) {
    DCHECK(!root_url.empty());
  }

  bool OnRequest(scoped_refptr<CefResourceManager::Request> request) override {
    CEF_REQUIRE_IO_THREAD();

    const std::string& url = request->url();
    if (url.find(root_url_) != 0L) {
      // Not handled by this provider.
      return false;
    }

    CefRefPtr<CefResourceHandler> handler;

    const std::string& relative_path = url.substr(root_url_.length());
    if (!relative_path.empty()) {
      CefRefPtr<CefStreamReader> stream =
          GetResourceReader(relative_path.data());
      if (stream.get()) {
        handler = new CefStreamResourceHandler(
            request->mime_type_resolver().Run(url), stream);
      }
    }

    request->Continue(handler);
    return true;
  }

 private:
  std::string root_url_;

  DISALLOW_COPY_AND_ASSIGN(FileResourceProvider);
};

}  // namespace

// Retrieve the directory containing resource files on Windows.
bool GetResourceDir(std::string& dir) {
  char path[MAX_PATH];

  // Get the path to the current executable.
  if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
    return false;
  }

  // Remove the executable name to get the directory.
  PathRemoveFileSpecA(path);

  // Append "resources" subdirectory.
  strcat_s(path, "\\resources");
  dir = std::string(path);
  return true;
}

CefResourceManager::Provider* CreateBinaryResourceProvider(
    const std::string& url_path) {
  return new FileResourceProvider(url_path);
}

bool GetResourceString(const std::string& resource_path,
                       std::string& out_data) {
  std::string path;
  if (!GetResourceDir(path))
    return false;

  path.append("\\");
  path.append(resource_path);

  return ReadFileToString(path.c_str(), out_data);
}

CefRefPtr<CefStreamReader> GetResourceReader(const std::string& resource_path) {
  std::string path;
  if (!GetResourceDir(path))
    return nullptr;

  path.append("\\");
  path.append(resource_path);

  if (!FileExists(path.c_str()))
    return nullptr;

  return CefStreamReader::CreateForFile(path);
}

}  // namespace shared
