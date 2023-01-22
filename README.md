
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

See Reference:

    man deb-version

The library supports a version which matches the Debian version definition.

The `compare()` function handles some special cases for Debian versions
have some characters that are not hanlded in a simple alphanumeric order.

The parts of a Debian version always include an `<epoch>` and a `<release>`.

The `<epoch>` is an integer and if zero, it is ignored when outputting the
version back to a string.

The `<release>` is a string and if empty, it is ignored when outputting
the version back to a string.

**BUGS**

The versiontheca library breaks up versions into parts of numbers and strings
allowing it to clearly distinguish between each element and for Debian
versions, between sections (epoch, upstream version, and release).

There are also versions that Debian accept which are considered invalid
by versiontheca. Mainly, those are versions that have a dangling period
or two periods one after the other such as:

    1.4.
    1..2..7

As a result, some _weird_ versions do not compare the same way in
Debian and in our library.

For example, the version 1.2.0 is canonicalized by versiontheca as 1.2.
That means our library thinks the following two versions are equal:

    3:1.2.0-rc1
    3:1.2-rc1

because the .0 is _ignored_ once the first version gets canonicalized.

The Debian code, however, compares the '.' (0x2E) against the '-' (0x2D)
meaning that the first version is considered larger than the second.

Similarly, you can have dangling period in a Debian version and as a result
the following two versions compare differently in versiontheca:

    1.1.
    1.1+

Here Debian compares '.' (0x2E) against '+' (0x2B) meaning that the first
version is viewed as larger than the second version. In versiontheca, the
periods are viewed as part separators and get removed from the stings. As
a result, we compare and empty string against the '+' character. The first
version is considered smaller (and Debian sees `1.1` as smaller than `1.1+`).

As long as you do not use uncanonicalized versions, you will not encounter
such bugs.

## RPM / Fedora Version

See References:

* `rpmdev-vercmp --help` (from `rpmdevtools` package)
* https://fedoraproject.org/wiki/Packaging:Naming
* https://docs.fedoraproject.org/en-US/packaging-guidelines/Versioning/

The library supports a version which matches the RPM version definition.

Note that the Epoch in an RPM version is a Tag (the `Epoch: <integer>` in
your `.spec` file) and it does not appear in a version string like it does
in a Debian package. The version and release can appear in the string.

The RPM release is found after a dash (`-`).

The allowed characters are uppercase letters (`A-Z`), lowercase letters
(`a-z`), digits (`0-9`) and a few puntuation characters (`_`, `+`, '.',
'~', '^'). The tilde (`~`) sorts before anything else (including an empty
string) and the caret (`^`) sorts after anything else.

# Next & Previous Versions

The library offers a way to compute the next and previous versions. This is
just math, not a real tracking system of which versions existed so far, so
the previous is likely to be wrong in many cases.

The value to change is defined by a position. For example, your build system
should use position 3 as in:

    next of       1.3.2.7
    at position   3
    is            1.3.2.8
                  ^ ^ ^ ^
		  | | | |
		  | | | +-- position 3
		  | | +---- position 2
		  | +------ position 1
		  +-------- position 0

_Note: The position starts at 0._

The previous is more problematic, although the format can be used to
somewhat fix the main issues, 0 - 1 means the largest number has to be
used and that is not really great as it stands. For example, the previous
using position 3 looks like this:

    previous of   1.3.7
    at position   3
    is            1.3.6.4294967295

The next and previous do not touch the epoch or the release information.
It only tweaks the upstream version number. If the position is set to a
number larger than the number of existing parts, then new parts get added
as required just before the operation. The format is used to know whether
to add an integers or a string. If no format is specified or it is shorter
than the specified position, then integers separated by a period are used.

Letters can also be incremented and decremented. In that case, the letter
`A` is the very first one. `Z` + 1 is `a`. The letter `z` is the very last
letter. `z` + 1 is set to `A` and we consider that there is a carry.

The format is used to determine the limits. In case of letters, it is also
used to define the number of letters to use in a given part. In the format,
it is expected to be defined as the last possible value (the first is always
`A`). For example, a major and minor version followed by one letter would
have a format that looks like this: `"9999.9999z"` (the maximum numbers
could be much larger, we support a full 32 bits unsigned number).

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
