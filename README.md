# libspl

[![MIT Licensed](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

[![build](https://github.com/noahorensa/libspl/workflows/test/badge.svg)](https://github.com/noahorensa/pwdman/actions?query=workflow%3Atest)

A C++ container and utility library for building distributed systems. Libspl
is mainly focused on serialization and high-performance parallelism.

## Documentation

Doxygen-compatible documentation is included in the source code. Simply run
`doxygen` in the project root to generate the documentation under the **doc/**
directory.

## Build

To build, simply run:

    make

This will generate the current architecture's `libspl.so` and `libspl.a` under
the **lib/** directory.

## Test

To run unit tests:

    make test

## Copyright

Copyright (c) 2021-2023 Noah Orensa.

Licensed under the MIT license. See **LICENSE** file in the project root for details.
