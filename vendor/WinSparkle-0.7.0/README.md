[![Build status](https://ci.appveyor.com/api/projects/status/acsuqjheafef29m1?svg=true)](https://ci.appveyor.com/project/vslavik/winsparkle)
[![Crowdin](https://d322cqt584bo4o.cloudfront.net/winsparkle/localized.png)](https://crowdin.com/project/winsparkle)

 About
-------

WinSparkle is a plug-and-forget software update library for Windows
applications. It is heavily inspired by the Sparkle framework for OS X
written by Andy Matuschak and others, to the point of sharing the same 
updates format (appcasts) and having very similar user interface.

See https://winsparkle.org for more information about WinSparkle.

Documentation: [wiki](https://github.com/vslavik/winsparkle/wiki) and
the [winsparkle.h header](https://github.com/vslavik/winsparkle/blob/master/include/winsparkle.h).


 Using prebuilt binaries
-------------------------

The easiest way to use WinSparkle is to download the prebuilt `WinSparkle.dll`
binary.

 Building from sources
-----------------------

If you prefer to build WinSparkle yourself, you can do so.  You'll have to
compile from a git checkout; some of the dependencies are included as git
submodules.

Check the sources out and initialize the submodules:

    $ git clone git://github.com/vslavik/winsparkle.git
    $ cd winsparkle
    $ git submodule init
    $ git submodule update

To compile the library, just open `WinSparkle.sln` (or the one corresponding to
your compiler version) solution and build it.

At the moment, projects for Visual C++ (2008 and up) are provided, so you'll
need that (Express/Community edition suffices). In principle, there's nothing
in the code preventing it from being compiled by other compilers.

There are also unsupported CMake build files in the cmake directory.

 DSA signatures
---------------

WinSparkle uses exactly same mechanism for signing and signature verification
as [Sparkle Project](https://sparkle-project.org/documentation/#dsa-signatures)
does. Its tools and verification methods are fully compatible.

You may use any compatible way to sign your update.
To achieve this, you need to sign SHA1 (in binary form) of your update file
with DSA private key, using SHA1 digest.

WinSparkle provides tools to generate keys and sign the update using OpenSSL.

You need `openssl.exe` available on Windows to use those tools (available as
[precompiled binary][OpenSSL binaries]).

Alternatively, you can generate keys and sign your updates even on macOS or Linux,
using [tools provided by Sparkle project](https://github.com/sparkle-project/Sparkle/tree/master/bin).

#### Prepare signing with DSA signatures:

 - First, make yourself a pair of DSA keys. This needs to be done only once.
 WinSparkle includes a tool to help: `bin\generate_keys.bat`
 - Back up your private key (dsa_priv.pem) and keep it safe. You don’t want
 anyone else getting it, and if you lose it, you may not be able to issue any
 new updates.
 - Add your public key (dsa_pub.pem) to your project either as Windows resource,
 or any other suitable way and provide it using WinSparkle API.

#### Sign your update

When your update is ready (e.g. `Updater.exe`), sign it and include signature
to your appcast file:

 - Sign: `bin\sign_update.bat Updater.exe dsa_priv.pem`
 - Add standard output of previos command as `sparkle:dsaSignature` attribute
 of `enclosure` node of your appcast file.
 Alternatively `sparkle:dsaSignature` can be a child node of `enclosure`.


 Where can I get some examples?
--------------------------------

Download the sources archive and have a look at the
[examples/](https://github.com/vslavik/winsparkle/tree/master/examples) folder.


 Using latest development versions
-----------------------------------

If you want to stay at the bleeding edge and use the latest, not yet released,
version of WinSparkle, you can get its sources from public repository.
WinSparkle uses git and and the sources are hosted on GitHub at
https://github.com/vslavik/winsparkle

WinSparkle uses submodules for some dependencies, so you have to initialize
them after checking the tree out:

    $ git clone git://github.com/vslavik/winsparkle.git
    $ cd winsparkle
    $ git submodule init
    $ git submodule update

Then compile WinSparkle as described above; no extra steps are required.

[OpenSSL source]: https://www.openssl.org/source/
[OpenSSL binaries]: https://wiki.openssl.org/index.php/Binaries
