# Dependency-free host regression tests

Run `python3 tests/run_host_tests.py` with already installed Python 3 and a C++11
compiler (`g++` by default, or set `CXX`). No packages, Arduino libraries, or
firmware toolchains are downloaded or installed. Temporary host binaries are
removed after the run. CI uses the tools already on its hosted Ubuntu runner.

The script extracts the two actual LED animation functions from the main-rig
sketch and compiles them against synthetic Serial, FastLED, delay, and CRGB stubs.
A checked strip rejects every negative or out-of-range LED access. Both animation
paths are tested with 1, 2, 3, 16, 17, and 18 LEDs (including the real configured
count) and speeds of 0, 10, and 50. Assertions cover every displayed frame,
iteration count, final cleared state, and delay argument.

The source fix skips only the invalid neighbor at the strip boundary. Existing
valid pixel colors, animation timing, loop ranges, and servo/radio code remain
unchanged. Passing tests establish these isolated host-level properties only.
They do not compile the entire Arduino firmware, validate FastLED/IRremote/Servo
or RF24 compatibility, validate electrical behavior, or test physical hardware.
There is no device access or firmware flashing. Firmware library/toolchain
modernization still needs a supported board/toolchain decision and real checks.
