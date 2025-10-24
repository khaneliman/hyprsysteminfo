# Qt 6.10 Migration Notes

## Changes Made

This document describes the changes made to support Qt 6.10.

### Breaking Change

In Qt 6.10, the private API method `QWaylandIntegration::instance()` was removed. This method was previously used to access the Wayland display and enumerate wl_output globals.

### Solution

The code has been updated to use the public Qt Platform Abstraction (QPA) API instead:

1. **Removed dependencies on deprecated private APIs:**
   - `QtWaylandClient::QWaylandIntegration::instance()`
   - `QtWaylandClient::QWaylandDisplay`

2. **New approach:**
   - Use `QGuiApplication::platformNativeInterface()` to access the platform-specific interface
   - Get the native `wl_display` pointer using `nativeResourceForIntegration("wl_display")`
   - Directly interact with the Wayland protocol using `wl_display_get_registry()`
   - Listen for `wl_output` globals using standard Wayland registry listeners

### Key Changes in src/WaylandScreen.cpp

- **Before:** Used `QWaylandIntegration::instance()->display()` to get the display
- **After:** Use `QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("wl_display")`

- **Before:** Iterated over `display->globals()` to find wl_output objects
- **After:** Register a Wayland registry listener to receive global announcements

### Benefits

1. **Forward compatibility:** No longer depends on private Qt APIs that can change between versions
2. **Direct Wayland protocol usage:** More control over the Wayland interaction
3. **Standards compliant:** Uses the recommended QPA approach for platform-specific functionality

### Compatibility

These changes maintain compatibility with Qt 6.5+ while supporting Qt 6.10+. The code uses the standard platform native interface which is part of the public Qt API.

## Testing

To verify the changes work correctly:

1. Build with Qt 6.10
2. Run on a Wayland compositor
3. Verify that screen information is correctly enumerated

## References

- Qt Platform Abstraction: https://doc.qt.io/qt-6/qpa.html
- Wayland Protocol Documentation: https://wayland-book.com/
- Qt 6.10 Release Notes: https://doc.qt.io/qt-6/wayland-and-qt.html
