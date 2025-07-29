# Tests
The test build uses CMake to generate a build tree for tests using GTest. The CMake imports the GTest and GMock infrastructure as a dependency. A Makefile wrapper is provided to simplify the command line interface, as the CMake commands can be a bit cumbersome. For a list of the available make targets and their descriptions:

`make help`

The default **all** target will build the tests and run the coverage report:

`make`

If successful, the coverage report will be linked to **test/coverage.html** and can be viewed using any standard web browser.

The GTest User's Guide can be found at https://google.github.io/googletest/.

## Supported Host Platforms
- Linux
- Mac OS X

## Dependencies
- GNU Make
- CMake
- gcovr
    - on Linux: pip3 install gcovr
    - on Mac OX X: brew install gcovr

## Adding Tests
By convention, all test units have the **_Test.cxx** suffix on them and should be located in the same directory tree as the source under test. See **src/utilities/mustangpeak_string_helper_Test.cxx** for a simplified example. Test "helpers" use a similar suffix convention of **_Test.hxx**. For example, see **src/test/main_Test.hxx**.

To add a test, the source directly must have a **CMakeList.txt** file in it to inform the build. A good example can be found (and copied from) **/src/utilities/CMakeLists.txt**. Any dependent source files should be added to the **SOURCES** list. Each test unit should be added to the **TESTS** list. If copying an existing **CMakeLists.txt** file to a new directory, ensure that the **project** name is changed to reflect the parent directory name.

When adding a new source directly to the test build, the **test/CMakeLists.txt** must also be updated by adding the subdirectory (add_subdirectory) and updating the **LINK_LIBS** with the added directory name. Search for **utilities** to see an example of the necessary additions.

## GTEST Build Macro
In some cases, it may be necessary to customize the source code under test for the test build. This should be kept to an absolute minimum. The tests are built with the predefined GTEST macro to aid this. It can be used as follows:

```
#if defined(GTEST)
// Custom GTEST build specific stuff here.
#endif
```