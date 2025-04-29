# GeneralLogger

**GeneralLogger** is a **asynchronous logging library**, packaged as a C++ DLL.

It supports:
- Multi-threaded, non-blocking logging
- Asynchronous disk writes with minimal locking
- Log levels: INFO, ERROR, WARNING, DEBUG, GENERAL, CRITICAL
- Automatic creation of log files and folders
- Easy integration into any C++ project (Windows/Linux ready)

---

## Features

- Asynchronous buffered logging to minimise runtime cost
- Fixed memory buffer to avoid dynamic allocation overhead
- Thread-safe and scalable
- Log message formatting with:
  - `[TIME] [THREAD_ID] [LEVEL] message`
- Cross-platform export macro (`__declspec(dllexport)` / `__attribute__((visibility))`)

---

## Usage Example

```cpp
#include "eLogger.h"
#include <thread>

int main() {
    Logger logger("logs/my_log.txt", 2); // Flush every 2 seconds

    logger.LogInfo("Application started");
    logger.LogError("An error occurred");
    logger.LogWarning("Low memory warning");
    logger.LogDebug("Debugging details");
    logger.LogGeneral("General log message");
    logger.LogCritical("Critical error!");
    return 0;
}
