# ArkCompiler Toolchain

ArkCompiler Toolchain provides debugging and profiling tools for ArkTS applications in the OpenHarmony ecosystem. It implements the ChromeDevTools Protocol (CDP) to enable features like breakpoints, stepping, CPU profiling, and heap snapshot analysis through DevEco Studio.

## Architecture Overview

```
DevEco Studio
      |
      v
Inspector Layer (inspector/)  <-- Session management & message forwarding
      |
      v
WebSocket Communication (websocket/)
      |
      v
Tooling Layer (tooling/)
      |
      +-- ArkTS-Dyn (dynamic/)  <-- Runtime debugging & profiling
      +-- ArkTS-Sta (static/)    <-- Static analysis & debugging
      +-- Hybrid Debugging (hybrid_step/)
```

The toolchain works with ArkCompiler Runtime to provide runtime information and executes debugging/profiling commands.

## Directory Structure

```
arkcompiler/toolchain/
|-- adapter/              # Adaptation layer for simulator and previewer components
|-- common/               # Shared utilities (logging, macros)
|-- docs/                 # Documentation for debugging and profiling features
|-- figures/              # Architecture diagrams
|-- inspector/            # Debug protocol integration layer
|   |-- inspector.cpp     # Main inspector implementation
|   |-- ws_server.cpp     # WebSocket server
|   `-- connect_inspector.cpp
|-- platform/             # Platform-specific implementations
|   |-- file.h            # Platform-independent file interface
|   |-- unix/             # Unix-specific implementations
|   `-- windows/          # Windows-specific implementations
|-- test/                 # Testing infrastructure
|   |-- autotest/         # Automated tests
|   |-- fuzztest/         # Fuzz testing
|   |-- resource/         # Test resources
|   `-- ut/               # Unit tests
|-- tooling/              # Core debugging and profiling tools
|   |-- dynamic/          # ArkTS-Dyn implementation
|   |   |-- agent/        # Runtime agents (runtime, dom, page, profiling, debugger)
|   |   |-- backend/      # Backend executors and hooks
|   |   |-- base/         # Protocol types and utilities
|   |   |-- client/       # Client tools (ark_multi, ark_cli)
|   |   `-- utils/        # Utilities
|   |-- static/           # ArkTS-Sta implementation
|   |   |-- connection/   # Connection management
|   |   |-- debugger/     # Core debugger functionality
|   |   |-- evaluation/   # Expression evaluation engine
|   |   |-- types/        # Protocol types and serialization
|   |   `-- json_serialization/
|   `-- hybrid_step/      # Hybrid debugging step flags
|-- websocket/            # WebSocket protocol implementation
|   |-- server/           # Server implementation
|   |-- client/           # Client implementation
|   |-- frame_builder.cpp # Frame building utilities
|   `-- handshake_helper.cpp
|-- BUILD.gn              # Main build configuration
|-- bundle.json           # Component metadata
|-- toolchain.gni         # Toolchain build configuration
`-- toolchain_config.gni  # Toolchain configuration options
```

## Protocol Domains

The toolchain implements CDP domains for different debugging/profiling features:

- **Debugger**: Breakpoints, stepping, call frame evaluation, exception handling
- **Profiling**: CPU profiling via stack sampling or instrumentation
- **HeapProfiler**: Heap snapshots and allocation tracking
- **Runtime**: Heap memory usage, object property inspection

## Key Features

- **Breakpoint debugging**: Regular, conditional, and exception breakpoints
- **Stepping**: Step into, step out, step over, continue
- **Watch expressions**: Monitor and modify variables during debugging
- **Multi-instance debugging**: Debug multiple VM instances simultaneously (e.g., main thread + workers)
- **Hot reload**: Apply code changes without restarting the application
- **Hybrid debugging**: Step between ArkTS and C++ functions seamlessly
- **CPU profiling**: Identify performance bottlenecks with time and call frequency data
- **Heap analysis**: Inspect memory usage, object references, and allocation patterns

## Build System

Uses GN (Generate Ninja) as the primary build system. Key build targets:

- `ark_debugger`: Main debugger binary
- `libark_ecma_debugger` / `libark_tooling`: Dynamic debugging library (ArkTS-Dyn)
- `libarkinspector_plus` / `arkinspector`: Static analysis library (ArkTS-Sta)
- `libark_client`: Client library
- `ark_multi`, `arkdb`: Command-line debugging tools

See ./README_zh.md for build instructions.

## Platform Support

- OpenHarmony (OHOS)
- Linux
- Windows
- Android
- iOS
- macOS

## Related Repositories

- ets_runtime: ArkTS runtime implementation
- runtime_core: Core runtime components

## Documentation

- ./docs: Detailed feature documentation
- ./README_zh.md: Repository overview (Chinese)

## Development Notes

- The `tooling/dynamic` directory hosts ArkTS-Dyn, the `tooling/static` directory hosts ArkTS-Sta, and the rest of the code is common to both ArkTS-Dyn and ArkTS-Sta.
- The code comments in this repository should be written in English.
- The commit message should be written in English.
- Don't create commits directly. Have them reviewed.
