# Overview of Quality Controls

There are two **complementary** approaches to software testing in this project:

- Hardware-independent:

  This is a set of test units that do not require specific hardware.
  They are all automated.

- Hardware-dependent:

  This is a set of test units that require specific hardware or
  just an ESP32 DevKit board.
  With a few exceptions, these are **not automated**.

There is an automated build process for both sets.
This build process will compile (and link) all the test units,
and then run automated tests not requiring specific hardware.
Manual testing is required for hardware dependent quality control.

Each test is expected to *catch bugs*,
so they *succeed* when the expected output (or behavior)
does *not* match the actual output (or behavior).
Otherwise they *fail*.
This means that all the test have to fail in order to pass the quality control.

## Manual testing

The quality control is implemented as a set of Arduino sketches
located under the [src/QualityControl](../../src/QualityControl/) folder.
There are two kinds of quality controls:

1. **Unit tests**:
   to find bugs in the behavior of a single module.
2. **Integration tests**:
   to find bugs in the exchange of data from the tested module
   to other modules and their behavior as a group.

Inside each sketch folder there is a file named "README.md".
This file describes the **test procedure**:

1. The required hardware (if any), to be built in a prototype board.
2. The required software tools (if any).
3. Steps to follow.
4. Expected output.

All unit tests must fail before proceeding to integration tests.
They may be run in any order.
However, integration tests must be run in a particular order provided by the
[integration strategy](../../src/QualityControl/IntegrationTests/README.md).

Before testing, source files must be set up by following the
[source code setup procedure](./sourcesSetup_en.md).
