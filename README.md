```
cmake -G "Ninja Multi-Config" -S . -B build

cmake -G "Visual Studio 16 2019" -A x64 -S . -B build

cmake -G "Visual Studio 17 2022" -A x64 -S . -B build

cmake --build build --config Debug

cmake -G "Unix Makefiles" -S . -B build -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug

```

```
/**
 * @file filename.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once
```
