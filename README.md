Entropia File System Watcher ![efsw](https://web.ensoft.dev/efsw/efsw-logo.svg)
============================

[![build status](https://img.shields.io/github/actions/workflow/status/SpartanJ/efsw/main.yml?branch=master)](https://github.com/SpartanJ/efsw/actions?query=workflow%3Abuild)

**efsw** is a C++ cross-platform file system watcher and notifier.

**efsw** monitors the file system asynchronously for changes to files and directories by watching a list of specified paths, and raises events when a directory or file change.

**efsw** supports recursive directories watch, tracking the entire sub directory tree.

**efsw** currently supports the following platforms:

* Linux via [inotify](http://en.wikipedia.org/wiki/Inotify)

* Windows via [I/O Completion Ports](http://en.wikipedia.org/wiki/IOCP)

* macOS via [FSEvents](http://en.wikipedia.org/wiki/FSEvents) or [kqueue](http://en.wikipedia.org/wiki/Kqueue)

* FreeBSD/BSD via [kqueue](http://en.wikipedia.org/wiki/Kqueue)

* OS-independent generic watcher
(polling the disk for directory snapshots and comparing them periodically)

If any of the backend fails to start for any reason, it will fallback to the OS-independent implementation.
This should never happen, except for the Kqueue implementation; see `Platform limitations and clarifications`.

**Code License**
--------------
[MIT License](http://www.opensource.org/licenses/mit-license.php)

**Some example code:**
--------------------

```c++
// Inherits from the abstract listener class, and implements the the file action handler
class UpdateListener : public efsw::FileWatchListener {
  public:
    void handleFileAction( efsw::WatchID watchid, const std::string& dir,
                           const std::string& filename, efsw::Action action,
                           std::string oldFilename ) override {
        switch ( action ) {
            case efsw::Actions::Add:
                std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added"
                          << std::endl;
                break;
            case efsw::Actions::Delete:
                std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete"
                          << std::endl;
                break;
            case efsw::Actions::Modified:
                std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Modified"
                          << std::endl;
                break;
            case efsw::Actions::Moved:
                std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Moved from ("
                          << oldFilename << ")" << std::endl;
                break;
            default:
                std::cout << "Should never happen!" << std::endl;
        }
    }
};

// Create the file system watcher instance
// efsw::FileWatcher allow a first boolean parameter that indicates if it should start with the
// generic file watcher instead of the platform specific backend
efsw::FileWatcher* fileWatcher = new efsw::FileWatcher();

// Create the instance of your efsw::FileWatcherListener implementation
UpdateListener* listener = new UpdateListener();

// Add a folder to watch, and get the efsw::WatchID
// It will watch the /tmp folder recursively ( the third parameter indicates that is recursive )
// Reporting the files and directories changes to the instance of the listener
efsw::WatchID watchID = fileWatcher->addWatch( "/tmp", listener, true );

// Adds another directory to watch. This time as non-recursive.
efsw::WatchID watchID2 = fileWatcher->addWatch( "/usr", listener, false );

// For Windows, adds another watch, specifying to use a bigger buffer, to not miss events
// (do not use for network locations, see efsw.hpp for details).
efsw::WatchID watchID3 = fileWatcher->addWatch( "c:\\temp", listener, true, { (BufferSize, 128*1024) } );

// Start watching asynchronously the directories
fileWatcher->watch();

// Remove the second watcher added
// You can also call removeWatch by passing the watch path ( it must end with an slash or backslash
// in windows, since that's how internally it's saved )
fileWatcher->removeWatch( watchID2 );
```

**Dependencies**
--------------
None :)

**Compiling**
------------
To generate project files you will need to [download and install](https://premake.github.io/download) [Premake](https://premake.github.io/docs/What-Is-Premake)

Then you can generate the project for your platform by just going to the project directory where the premake4.lua file is located and executing:

`premake5 gmake2` to generate project Makefiles, then `cd make/*YOURPLATFORM*/`, and finally `make` or `make config=release_x86_64` ( it will generate the static lib, the shared lib and the test application ).

or

`premake5 vs2022` to generate Visual Studio 2022 project.

or

`premake5 xcode4` to generate Xcode 4 project.

There is also a cmake file that I don't officially support but it works just fine, provided by [Mohammed Nafees](https://github.com/mnafees) and improved by [Eugene Shalygin](https://github.com/zeule).

**Packages**
------------

Community has been kind and contributed the recipes for different popular packages systems: [Conan](https://conan.io/center/recipes/efsw), [vcpkg](https://vcpkg.io/en/package/efsw), [xmake-repo](https://github.com/xmake-io/xmake-repo/blob/dev/packages/e/efsw/xmake.lua). I don't personally maintain those packages so they could be not up to date, but they tend to be quite up to date.

**Platform limitations and clarifications**
-------------------------------------------

Directory paths are expected to be encoded as UTF-8 strings in all platforms.

handleFileAction returns UTF-8 strings in all platforms.

File modification events may be reported multiple times during a copy operation, typically after each write flush. This behavior is inherent to how file watchers operate and cannot be altered. If you need to open the file after receiving a modification event, it is recommended to wait for a reasonable period of time after the last event to avoid potential issues.

Windows and FSEvents macOS implementation can't follow symlinks ( it will ignore followSymlinks() and allowOutOfScopeLinks() ).

Kqueue implementation is limited by the maximum number of file descriptors allowed per process by the OS. In the case of reaching the file descriptors limit ( in BSD around 18000 and in macOS around 10240 ), it will fallback to the generic file watcher.

macOS will use only Kqueue if the macOS version is below 10.5. This implementation needs to be compiled separately from the macOS >= 10.5 implementation, since there's no way to compile FSEvents backend in macOS below 10.5.

FSEvents for macOS Lion and beyond in some cases will generate more actions than in reality ocurred, since fine-grained implementation of FSEvents doesn't give the order of the actions retrieved. In some cases I need to guess/approximate the order of them.

Generic watcher relies on the inode information to detect file and directories renames/move. Since Windows has no concept of inodes as Unix platforms do, there is no current reliable way of determining file/directory movement on Windows without help from the Windows API ( this is replaced with Add/Delete events ).

Linux versions below 2.6.13 are not supported, since inotify wasn't implemented yet. I'm not interested in supporting older kernels, since I don't see the point. If someone needs this, open an issue in the issue tracker and I may consider implementing a dnotify backend.

OS-independent watcher, Kqueue and FSEvents for macOS below 10.5 keep cache of the directories structures, to be able to detect changes in the directories. This means that there's a memory overhead for these backends.

**Useful information**
--------------------
The project also comes with a C API wrapper, contributed by [Sepul Sepehr Taghdisian](https://github.com/septag).

There's a string manipulation class not exposed in the efsw header ( efsw::String ) that can be used to make string encoding conversion.

**Clarifications**
----------------

This software started as a fork of the [simplefilewatcher](http://code.google.com/p/simplefilewatcher/) by James Wynn (james[at]jameswynn.com), [MIT licensed](http://www.opensource.org/licenses/mit-license.html).

The icon used for the project is part of the [Haiku®'s Icons](http://www.haiku-inc.org/haiku-icons.html), [MIT licensed](http://www.opensource.org/licenses/mit-license.html).
