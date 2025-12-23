# Aegis Project Overview (English)

## One-liner

Aegis is a C89 DDD + CQRS framework for embedded/MCU systems. It focuses on **static memory**, **strict dependency injection**, and **hard constraint checks** to enforce architecture discipline and keep iterative development predictable and maintainable.

## Embedded pain points Aegis targets

- Heap fragmentation and timing uncertainty from `malloc/free`.
- Unsafe calls from ISR leading to rare, hard-to-debug failures.
- Architecture erosion (cross-layer coupling) as the codebase grows.
- “Tribal knowledge” conventions that don’t scale with team size.
- Traceability requirements (e.g., functional safety) that are hard to keep consistent over time.

## Approach

- DDD layers (Domain / Application / Entry / Infrastructure + Common) + CQRS separation to constrain complexity.
- Entry as the composition root: dependencies (repo/clock/etc.) are assembled in the platform layer and injected, keeping business code independent of Infrastructure.
- `constraint_checks` enforces naming, layering, ISR safety, traceability, no heap, and no mutable globals at build time.

## Key capabilities

- Domain modeling: Aggregate/AggregateRoot, ValueObject, DomainService, Repository interfaces (read/write split).
- CQRS: ISR-safe command queue, command routing/execution, query dispatcher returning DTOs.
- Domain events: sync/async subscriptions, event history and statistics.
- Productivity: module registration, payload macros (compile-time size checks), scaffolding generator.
- Ports: runnable `x86_sim` port; STM32F030 porting example as reference.

## Fit / Non-fit

- Fit: bare-metal or minimal RTOS MCU projects that value determinism, constraints, and traceability.
- Non-fit: systems that require heavy runtime dynamism, plugins, or heap-centric designs.




# Quick Start (Aegis / English)

This is the shortest “copy/paste and run” path. For a deeper guide, see `docs/USAGE.en.md`.

## 0) Where things are

- `framework/`: the core framework (C89, static memory, DDD+CQRS, strict DI)
- `framework/port/<platform>/`: ports (HAL/critical/entry_platform)
- `application/`: demo app (composition root / Entry)
- `examples/`: sample apps (start with `minimal_app`)
- `tools/`: hard constraint checks (naming, layering, ISR safety, traceability, no heap, no mutable globals)

## 1) Build & Run (x86_sim)

From the repo root:
```bash
cmake -S . -B build
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

Run examples:
```bash
./examples/minimal_app/minimal_app
./examples/event_driven/event_driven
```

## 2) Constraint Checks (DDD + CQRS + DI)

```bash
cmake --build build --target constraint_checks
```

These checks are part of the framework contract: failing fast is the goal.

## 3) Architecture in 30 seconds: layers + DI

- Entry (composition root) assembles dependencies and wires modules.
- Application coordinates use-cases (command/query handlers). No business rules. No Infrastructure includes.
- Domain holds business rules (aggregates/value objects/domain services/events). Depends on Common only.
- Infrastructure/Port provides implementations (repo + HW adapters). May depend on Domain interfaces.

Hard rule: Entry/Application must not `#include "infrastructure_*.h"`; implementations are assembled in `entry_platform` and injected.

## 4) Minimal startup flow (composition root)

Typical order (see `examples/minimal_app/main.c`):
1) `aegis_entry_platform_init(now_ms_fn, ctx)`
2) `aegis_entry_platform_get_write_repo()`
3) inject `write_repo` into `aegis_entry_init_all(&runtime, &cfg)`
4) register modules
5) drive main loop via `aegis_entry_main_loop_once()` or `aegis_entry_main_loop()`

## 5) Add a business module (scaffold + macros)

Generate scaffolding:
```bash
python3 tools/scaffold_module.py --name charger --out examples/charger
```

Module entry signature (fixed):
```c
AegisErrorCode charger_application_register(AegisAppRuntime* app, void* ctx);
```

Compose modules in Entry (strict DI):
```c
#include "app_module.h"
#include "app_macros.h"

AegisAppModule modules[1];
APP_MODULE_SET(&modules[0], charger_application_register, &charger_module);
APP_REGISTER_MODULES(ret, &runtime.app, modules);
```

Use payload macros in handlers (compile-time size constraints, less boilerplate):
```c
#include "app_macros.h"
APP_CMD_PAYLOAD_GET(ret, cmd, &payload);
APP_QUERY_RESULT_PAYLOAD_SET(ret, resp, &dto);
```

## 6) CQRS flow: command vs query

- Commands: enqueue in ISR with `aegis_app_cmd_enqueue()` only; execute in the main loop (Application handlers orchestrate the use-case).
- Queries: synchronous (`aegis_app_query_execute()`), return DTO via `AegisQueryResponse` payload.

## 7) Porting: STM32F030 example

Reference port code is in `framework/port/stm32f030/`:
- `port_critical.c`: PRIMASK-based critical section
- `port_hal_gpio.c`: register-level GPIO
- `port_hal_timer.c`: SysTick-based tick + software timers (ms)
- `entry_platform.c`: platform assembly (default in-mem repo + now_ms injection)

Build for the port (MCU builds usually disable tests/examples):
```bash
cmake -S . -B build/stm32f030 -DTARGET_PLATFORM=stm32f030 -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF
cmake --build build/stm32f030 -j4
```

Note: this is a porting example; it does not include startup files, linker scripts, or clock tree init.
