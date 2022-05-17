# libspl

A C++ container and utility library for building distributed systems. Libspl
is mainly focused on serialization and the efficient use of parallelism.

## Documentation

Doxygen-compatible documentation is included in the source code. Simply run
`doxygen` in the project root to generate the documentation under the **doc/**
directory.

## Build

To build, simply run:

    make

This will generate the current architecture's `libspl.so` under the **lib/**
directory.

## Test

To run unit tests:

    make test
