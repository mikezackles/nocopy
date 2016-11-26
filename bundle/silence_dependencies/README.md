This header defines two macros that prevent external header dependencies from
generating warnings. This is useful for ensuring that headers are error-free
without getting false-positives from dependencies. To use it, add the include
directory to your compiler's search path and do something like this:

```c++
#include <silence_dependencies/ignore_warnings.hpp>
SD_BEGIN_IGNORE_WARNINGS
#include <first_dependency.hpp>
#include <second_dependency.hpp>
SD_END_IGNORE_WARNINGS
```

To re-enable warnings, just define `SD_ENABLE_WARNINGS`.
