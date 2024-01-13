# Overview of Quality Controls

Quality controls are implemented as a set of Arduino sketches located under the folder [src/QualityControls](../../src/QualityControls/). There are two kinds of quality controls:

1. **Unit tests**: to reveal bugs in the behavior of a single module.
2. **Integration tests**: to reveal bugs in the interchange of data from the tested module to other modules and their behavior as a group.

Those are **not** fully automated test. Inside each sketch's folder, a file named "README.md" can be found. This file describes the **testing procedure**:

1. Required hardware (if any), to be built in a prototype board.
2. Required software tools (if any).
3. Steps to follow.
4. Expected output.

Each quality test is expected to *reveal bugs*, so they *success* when the expected output (or behavior) does *not* match the actual output (or behavior). They *fail* otherwise.
This means that all the test have to fail in order to pass the quality controls.
All unit tests must fail before going into integration tests. They can be run in any sequence.
However, integration tests must be run in a certain sequence provided by the [integration strategy](../../src/QualityControls/IntegrationTests/README.md).

Before testing, source files must be put in place by following the [source code setup procedure](./sourcesSetup_en.md).
