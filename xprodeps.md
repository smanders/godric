# godric dependencies

|project|license [^_l]|description [dependencies]|version|source|diff [^_d]|
|-------|-------------|--------------------------|-------|------|----------|
|<a id='godric' />[godric](https://github.com/smanders/godric)| |tool for sorting files into structured directories based on filename patterns [pvt deps: _boost, wxWidgets, wxx_]| | |  [intro]|
|<a id='boost' />[boost](http://www.boost.org/ 'Boost website')|[BSL-1.0](http://www.boost.org/users/license.html 'Boost Software License')|libraries that give C++ a boost [deps: _bzip2, zlib_]|[xpv1.91.0.1](https://github.com/externpro/boost/releases/tag/xpv1.91.0.1 'release')|[repo](https://github.com/externpro/boost 'github.com/externpro/boost') [upstream](https://github.com/boostorg/boost 'github.com/boostorg/boost')|[diff](https://github.com/externpro/boost/compare/boost-1.91.0...xpv1.91.0.1 'github.com/externpro/boost/compare/boost-1.91.0...xpv1.91.0.1') [native]|
|<a id='wxWidgets' />[wxWidgets](http://wxwidgets.org/)|[wxWindows](https://wxwidgets.org/about/licence/ 'essentially LGPL with an exception')|Cross-Platform C++ GUI Library|[xpv3.1.0.6](https://github.com/externpro/wxWidgets/releases/tag/xpv3.1.0.6 'release')|[repo](https://github.com/externpro/wxWidgets 'github.com/externpro/wxWidgets') [upstream](https://github.com/wxWidgets/wxWidgets 'github.com/wxWidgets/wxWidgets')|[diff](https://github.com/externpro/wxWidgets/compare/v3.1.0...xpv3.1.0.6 'github.com/externpro/wxWidgets/compare/v3.1.0...xpv3.1.0.6') [intro(msw), native(unix)]|
|<a id='wxx' />[wxx](https://github.com/externpro/wxx)|[wxWindows](http://wxcode.sourceforge.net/ 'wxWindows Library License')|wxWidget-based extra components [deps: _wxwidgets_]|[xpv26.02](https://github.com/externpro/wxx/releases/tag/xpv26.02 'release')|[repo](https://github.com/externpro/wxx 'github.com/externpro/wxx')|[diff](https://github.com/externpro/wxx/compare/v0...xpv26.02 'github.com/externpro/wxx/compare/v0...xpv26.02') [intro]|
|<a id='bzip2' />[bzip2](https://sourceware.org/bzip2/)|[bzip2-1.0.6](https://spdx.org/licenses/bzip2-1.0.6.html 'BSD-like, modified zlib license')|lossless block-sorting data compression library|[xpv1.0.8.5](https://github.com/externpro/bzip2/releases/tag/xpv1.0.8.5 'release')|[repo](https://github.com/externpro/bzip2 'github.com/externpro/bzip2') [upstream](https://github.com/opencor/bzip2 'github.com/opencor/bzip2')|[diff](https://github.com/externpro/bzip2/compare/bzip2-1.0.8...xpv1.0.8.5 'github.com/externpro/bzip2/compare/bzip2-1.0.8...xpv1.0.8.5') [intro]|
|<a id='zlib' />[zlib](https://zlib.net/ 'zlib website')|[Zlib](https://zlib.net/zlib_license.html 'zlib/libpng license, see https://en.wikipedia.org/wiki/Zlib_License')|a general-purpose lossless data-compression library|[xpv1.3.2.1](https://github.com/externpro/zlib/releases/tag/xpv1.3.2.1 'release')|[repo](https://github.com/externpro/zlib 'github.com/externpro/zlib') [upstream](https://github.com/madler/zlib 'github.com/madler/zlib')|[diff](https://github.com/externpro/zlib/compare/v1.3.2...xpv1.3.2.1 'github.com/externpro/zlib/compare/v1.3.2...xpv1.3.2.1') [patch]|

![deps](xprodeps.svg 'dependencies')

Dependency version check: all 5 parent-manifest versions match pinned versions.

|diff  |description|
|------|-----------|
|patch |diff modifies/patches existing cmake|
|intro |diff introduces cmake|
|auto  |diff adds cmake to replace autotools/configure/make|
|native|diff adds cmake but uses existing build system|
|bin   |diff adds cmake to repackage binaries built elsewhere|
|fetch |diff adds cmake and utilizes FetchContent|

[^_l]: see [SPDX License List](https://spdx.org/licenses/ '') for a list of commonly found licenses
[^_d]: see table above with description of diff
