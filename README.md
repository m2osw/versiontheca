
<p align="center">
<img alt="Versiontheca" title="Versiontheca -- a library to parse versions for validity and comparison."
src="https://snapwebsites.org//sites/snapwebsites.org/files/images/versiontheca.png" width="200" height="200"/>
</p>

# Versiontheca

A library to manage version strings in C++ and with a command line tool.

## Basics About Versions

Versions are generally defined by the project using them. However, some
systems have versions defined for all the projects defined within them.
For exampe, Debian, RedHat, and other distribution of Linux make use
of one specific scheme to allow all the packages to use a type of version
allowing comparisons to work.

Here are some definitions based on various sources:

    <epoch>:<major>.<minor>.<patch>.<build>-<revision>~<release>

* `<epoch>`

  The `<epoch>` is used to properly sort a package based on its version.
  There are really rare project that made use _strange_ numbers make the
  `<major>` version go backward. To support these packages inside Debian,
  the Debian maintainers decided to add support for an _epoch_. By default,
  this number is 0 and thus unspecified in the canonicalized version string.

* `<major>.<minor>.<patch>.<build>`

  These four parameters are the standard version number.

  The `<major>` version is expected to change only when a major modification
  is applied to your project (i.e. totally different from the previous
  version).

  The `<minor>` version is changed whenever you do a breaking change in your
  project but nothing so major that users would have to rewrite everything.
  In most cases, when this version number increases, the users have to rebuild
  their project so it remains compatible.

  The `<patch>` version number represents a bug fix or small enhancement that
  is transparent to the user. In other words, whether the user runs against
  the newer version or the older version, their code will link appropriately
  without the need to recompile. The newer version, though, may have bug fixes
  meaning that it will work better.

  The `<build>` version is used by the build system(s) when it rebuilds the
  package. It does not imply a change to the library other than a comment
  here, documentation there, or a very small fix to an installation script.

  More such numbers can be used by your project, although it is rather rare
  that you would want to use more than 4 numbers as described above. When
  you add more numbers, the `<build>` is probably to remain the last of
  all those numbers.

  The canonicalized version does not output the `.0` for `<patch>` and
  `<build>` if not required (i.e. if the patch is 0 but the build number
  is not, then the patch is still output as `.0`).

  The Debian parser supports one or more such number (the `<minor>` and
  further numbers are all optional, although even when zero, the `<minor>`
  number is usually included in the canonicalized string).

  When defined as numbers, the values are viewed as decimal numbers.

  Debian views letters as separate names, so a value of "35ax" is viewed
  as two parts: the number 35 and the string "ax".

* `<revision>`

  The debian versioning mechanism allows for a separate revision. This is
  generally a string such as "alpha" or "beta" or "rc1".

* `<release>`

  Again, the debian versioning system allows for a `<release>` name. This
  is used to distinguish packages for different releases of the operating
  system (i.e. bionic vs focal).

  We use that in our own packages because we want to compile multiple
  releases of the same version of the package in our launchpad account.

## Basic Versions

The library supports a basic version class. This is limited to any number
of numbers separated by periods. You must have at least one number (major
version).

    <major>[.<minor>[.<patch>[ ... ]]]

You can further limit the number of entries before calling `set_version()`.

## Decimal Versions

The library supports a decimal version class. This is similar to a basic
version limited to two numbers (`<major>[.<minor>]`). The result can be
retrieved as a double floating point instead of two separate integers.

## Debian Versions

The library supports a version which matches the Debian version definition.

The `compare()` function handles some special cases for Debian versions
have some characters that are not hanlded in a simple alphanumeric order.

The parts of a Debian version always include an `<epoch>` and a `<release>`.

The `<epoch>` is an integer and if zero, it is ignored when outputting the
version back to a string.

The `<release>` is a string and if empty, it is ignored when outputting
the version back to a string.

# Where does the name come from?

The suffix -theca comes from Latin and Greek. It means _library_, _gallery_,
or more generally _collection of things_.

This library is expected to support all kinds of versions, although, at this
time, it is specific to Debian packages.

See: [_-teca_ suffix on Wiktionary](https://en.wiktionary.org/wiki/-teca).

# Source of this project

The debian version handling found in this library was first created for the
[wpkg project](https://sourceforge.net/projects/unigw/).

# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/versiontheca/issues).


_This file is part of the [snapcpp project](https://snapwebsites.org/)._
