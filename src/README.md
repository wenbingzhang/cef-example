# Scheme Handler Example Application

This directory contains the "scheme_handler" target which demonstrates how to handle resource requests using [CefSchemeHandlerFactory](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-scheme-handler).

See the [shared library](../shared) target for details common to all executable targets.

## Implementation Overview

The "scheme_handler" target is implemented as follows:

 * Define the target-specific [CMake](https://cmake.org/) build configuration in the [CMakeLists.txt](CMakeLists.txt) file.
 * Call the shared [entry point functions](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-entry-point-function) that initialize, run and shut down CEF.
     * Uses a minimal client implementation based on the original minimal target.
 * Implement the [shared::Create*ProcessApp](../shared/app_factory.h) functions to create a [CefApp](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-cefapp) instance appropriate to the [process type](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-processes).
     * Browser process: [app_browser_impl.cc](app_browser_impl.cc) implements the `shared::CreateBrowserProcessApp` method.
         * Register the custom scheme name in [OnRegisterCustomSchemes](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-request-handling).
         * Register the custom scheme handler factory by calling `RegisterSchemeHandlerFactory` implemented in [scheme_handler_impl.cc](scheme_handler_impl.cc).
         * Create the initial [CefBrowser](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-cefbrowser-and-cefframe) instance.
     * Other processes: [app_subprocess_impl.cc](app_subprocess_impl.cc) implements the `shared::CreateRendererProcessApp` and `shared::CreateOtherProcessApp` methods.
         * Register the custom scheme name in [OnRegisterCustomSchemes](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-request-handling).
 * Provide a concrete [CefClient](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-cefclient) implementation to handle [CefBrowser](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md#markdown-header-cefbrowser-and-cefframe) callbacks.
      * Uses a minimal client implementation based on the original minimal target.
 * Windows resource loading implementation in [resource_util_win.cc](../shared/resource_util_win.cc).
     * Implements the [shared::GetResourceDir](../shared/resource_util.h) method to locate the resources directory alongside the executable.
     * Copies [logo.png](resources/logo.png) and [scheme_handler.html](resources/scheme_handler.html) to a resources folder during build.

## Configuration

See the [shared library](../shared) target for configuration details.

## Setup and Build

See the [shared library](../shared) target for setup and build instructions.
