# How to contribute

**You may still contribute even if you don't have software development skills or in-depth knowledge of this project.**

This guide discusses several ways to contribute to the open source/hardware community around this project.
If you find another way to contribute, do not hesitate.

## Improve documentation

The author is not a native English speaker.
If you find errata or unclear text, please open an [issue][4].

## Report unexpected firmware behavior (bugs)

1. Ensure the "bug" was not already reported by searching on GitHub under [Issues][1],
   both open and closed.

2. If you find a similar **open** report:

   - Add a post to the existing thread. Explain the details of your case.
   - Add any relevant information not known before.

3. If you find a similar **closed** report, it should solve your problem.

   - If not, add a post to the existing thread.
   - If you don't have permission to post, create a [new issue][2].
     Place a mention to the existing report.

4. If you did not find any similar report, create a [new issue][2],
   using the given template.

Before posting a new issue, check you are using the latest project version
and the tested dependencies (see [Software tools](../doc/skills_en.md)).

## Make proposals, help other people, share concerns or ask any question

Participate in the [discussion forums][3]:

- Use the best-matching category (general, ideas, ...)
- Check for similar thoughts both in **closed** and open threads.

## Create social media

- Share your experience.
- Show your custom devices.
- Contribute with a tutorial.
- Provide a link to your own project.
- Tell others about this project.

## Share your knowledge

There are many aspects not fully covered by this project.
Use the [discussion forums][3] to share your knowledge about:

- Hardware that fits button boxes or sim wheels the best.
- Sim-wheel housing acquisitions or design.
- Skills to build homemade circuits.
- Tools.
- Etc.

## Contribute new hardware

You may contribute with new open hardware designs.
Consider adding design files for printed circuit boards.
Consider adding design files for sim-wheel housings.
Use the [discussion forums][3].

## Contribute new code

This is a **pull-request policy**.
Target the `development` branch when submitting a pull-request.

### New features

Code adding new features will be rejected unless previously discussed.
Expose your idea in the [discussion forums][3] first.
New features have an impact on software design that needs to be thoroughly examined.

### Bug fixes

Code fixing a bug will be rejected unless previously [reported][1].

- All relevant quality controls must be updated along with the new code,
  including new test units when needed.
- You must run all relevant quality controls.
- Provide an explicit declaration that all relevant quality controls failed
  (so, the software passed the Q controls).
- You must contribute to the documentation as you do with code.

### Evolutionary maintenance

There are a few software/development dependencies in this project:

- [Espressif/ESP-Arduino](https://github.com/espressif/arduino-esp32)
- [h2zero/NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)
- [Arduino/Arduino-IDE](https://github.com/arduino/arduino-ide)

A change in those dependencies may prevent the firmware from compiling or working as expected.
If you find this kind of failure, open a new [issue][2] first.
If you test a new version of those dependencies and it works as expected,
please, [inform][3].

All integration tests must run again in response to these changes.
The author makes his best to keep the code up to date with them.

[1]: https://github.com/afpineda/OpenSourceSimWheelESP32/issues
[2]: https://github.com/afpineda/OpenSourceSimWheelESP32/issues/new?template=bug_report.md
[3]: https://github.com/afpineda/OpenSourceSimWheelESP32/discussions
[4]: https://github.com/afpineda/OpenSourceSimWheelESP32/issues/new?template=errata.md
