
# Full Coverage

At the moment, the library is poorly covered, although the most important
parts are already checked pretty well, we want to finish that up with all
the different types of versions and every possible error type.

# Fix the Traits vs Data

Right now a trait includes the parts of the version when that should be in
the versiontheca class.

i.e. we should not have to create more than one version of the trait and
use that same trait any number of times.

