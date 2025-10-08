# SRTOS - Simple RTOS

## Overview

SRTOS is a simple preemptive Real-Time Operating System (RTOS) specifically developed with safety and learning in mind. All SRTOS code is written with the highest level of safety in mind, and compiled with strict flags. Also, code is heavily-documented on purpose, to ensure a high-level of user understanding. Static-allocation and stack overflow detection are required. Default stack overflow and hardfault handlers are provided, but users are encouraged to provide custom implementations. All SRTOS code strictly conforms to the style guidelines specified in `STYLE.md` in the root directory of this repository. Examples are provided in the `Examples/` directory of this repository, and tests are provided in the `Tests/` directory of this repository.

## Targets

Currently, SRTOS is written for the STM32F411E-DISCOVERY board only. A portable version of SRTOS is being developed, and all supported boards will be listed in this section as they are updated. Please refer to this list for all portability information.

- **STM32F411E-DISCOVERY**
  - Fully Supported

## Compilation

The easiest way to build and program SRTOS to the STM32F411E-DISCOVERY (or other STM32 boards, when ports are available) is to use the [STM32CUBEIDE](https://www.st.com/en/development-tools/stm32cubeide.html).

SRTOS is compiled using GCC's `arm-none-eabi-gcc` toolchain, with the following custom warning compilation flags: `-Wall -Wextra -Werror -pedantic -Wconversion`

You don't _need_ to use these flags when compiling, but they are **highly recommended** and **required when contributing** (more on this in the contributing section).

1. **Cloning**
   - You can either download the code from GitHub directly or clone the repo. How you do this is up to you and your goals. The **recommended** usage is to clone the repo directly in STM32CUBEIDE (or your IDE of choice) using plugins like [Egit](https://projects.eclipse.org/projects/technology.egit) for Eclipse based IDEs.
2. **Programming**
   - For programming, if using the STM32CUBEIDE, after importing the code, you can use the builtin program functionality. On most STM32 development boards, you also have a debugger available. To use the debugger, just use the Debug feature of the IDE. This functionality is dependent on your IDE choice. If your IDE does not specifically implement programming, then you must install all necessary software required (there are many resources that cover this topic). **Please Note**: ensure the linkerscript defines all necessary variables as specified in `DESIGN.md` (if you aren't using the default linkerscript).

## Guides and Design

For getting started guides, please refer to `GUIDES.md` in the root directory of this repository. It is **highly recommended** to read about the design of SRTOS in `DESIGN.md` in the root of this repository.

## Repository Structure

_This section is constantly changing as new features are being added._

- All source files (`.c`) that are required for building SRTOS are located in `Src/`
- All header files (`.h`) that are required for building SRTOS are located in `Inc/`
- All source files that contain example usages of SRTOS are located in `Examples/`
- All source files that contain tests of SRTOS are located in `Tests/`
- All source files and markdown files (`.md`) that contain code and information about tests are located in `Tests/`
- A default startup and linkerscript is provided for the STM32F411E-DISCOVERY. The location of these scripts is not specified here, because they may be changing as the file structure design changes.

## Contributing

Contributions are greatly appreciated and highly recommended. If you have added a feature, port, or bug fix, please create a pull-request. Ensure the code you have written conforms to all rules in `STYLE.md` before creating a request. Please read `DESIGN.md` before altering code to ensure you have a solid grasp on the system's behavior.