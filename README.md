# EmbedLog

![CMake](https://github.com/joeinman/EmbedLog/actions/workflows/cmake.yml/badge.svg)

EmbedLog is a lightweight hardware agnostic logging library designed for embedded systems.

## Features:

- **Multiple Log Levels**: Supports logging at various severity levels (INFO, WARNING, ERROR, DEBUG).
- **Lightweight**: Designed to keep the memory footprint low, making it suitable for embedded applications.
- **Customizable**: The library can be easily customized to work with different microcontrollers by providing the appropriate output and timestamp functions.

## Basic Example:

```cpp
#include <EmbedLog/EmbedLog.hpp>
#include <pico/stdlib.h>
#include <stdio.h>

using namespace EmbedLog;

int main()
{
    EmbedLog::EmbedLog logge(
        []() { stdio_init_all(); },
        []() { stdio_deinit_all(); },
        [](const std::string& message) { printf("%s", message.c_str()); },
        []() { return to_us_since_boot(get_absolute_time()); }
    );
    logger.open();

    while (1)
    {
        logger.log(INFO, "Hello, info world!\n");
        sleep_ms(1000);
    }
}
```
