# libdecor - A client-side decorations library for Wayland client

libdecor is a library that can help Wayland clients draw window
decorations for them. It aims to provide multiple backends that implements the
decoration drawing.


## Dependencies

Required:
- `meson` >= 0.47
- `ninja`
- `wayland-client` >= 1.18
- `wayland-protocols` >= 1.15
- `wayland-cursor`
- `cairo`
- `pangocairo`

Recommended:
- `dbus-1` (to query current cursor theme)

Optional
- `egl` (to build EGL example)
- `opengl`
- `xkbcommon` (to build cairo demo)

Install via apt:
`sudo apt install meson libwayland-dev wayland-protocols libpango1.0-dev libdbus-1-dev libegl-dev libopengl-dev libxkbcommon-dev`

Install via dnf:
`sudo dnf install meson wayland-devel wayland-protocols-devel pango-devel dbus-devel mesa-libEGL-devel libglvnd-devel libxkbcommon-devel`

Newer meson versions can be installed via pip: `pip3 install -U meson`.

## Build & Install

### Quick Start

To build and run the example program:
1. `meson build -Dinstall_demo=true && meson compile -C build`
2. `meson devenv -C build libdecor-demo`

### Release Builds

The library and default plugins can be built and installed via:
1. `meson build --buildtype release`
2. `meson install -C build`

where `build` is the build directory that will be created during this process.

This will install by default to `/usr/local/`. To change this set the `prefix` during built, e.g. `meson build --buildtype release -Dprefix=$HOME/.local/`.

Plugins will be installed into the same directory and from thereon will be selected automatically depending on their precedence. This behaviour can be overridden at runtime by setting the environment variable `LIBDECOR_PLUGIN_DIR` and pointing it to a directory with a valid plugin.

### Debug and Development Builds

During development and when debugging, it is recommended to enable the AddressSanitizer and increase the warning level:
1. `meson build -Dinstall_demo=true -Db_sanitize=address -Dwarning_level=3`
2. `meson compile -C build`

You may have to install `libasan6` (apt) or `libasan` (dnf). Otherwise linking will fail.

By default `libdecor` will look for plugins in the target directory of the installation. Therefore, when running the demos directly from the `build` directory, no plugins will be found and the fallback plugin without any decorations will be used.

On Meson 0.58.0 and above, this can be corrected using `devenv`, i.e., to run the demo:

`meson devenv -C build libdecor-demo`

On older Meson versions, the search path for plugins can be overridden by the environment variable `LIBDECOR_PLUGIN_DIR`. To use the `cairo` plugin, point to the plugin directory:

`export LIBDECOR_PLUGIN_DIR=build/src/plugins/cairo/`

and run the demo:

`./build/demo/libdecor-demo`.


### Code of Conduct

libdecor follows the Contributor Covenant, found at:
https://www.freedesktop.org/wiki/CodeOfConduct

Please conduct yourself in a respectful and civilised manner when interacting
with community members on mailing lists, IRC, or bug trackers. The community
represents the project as a whole, and abusive or bullying behaviour is not
tolerated by the project.
