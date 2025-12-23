# 示例应用（Aegis）

本目录包含 C89 DDD+CQRS 框架的示例应用，帮助开发者快速上手框架使用。

---

## 📁 目录结构

```
examples/
├── minimal_app/         # 最小示例
│   ├── main.c           # 主程序
│   └── CMakeLists.txt   # 构建配置
│
└── event_driven/        # 事件驱动示例
    ├── main.c           # 主程序
    └── CMakeLists.txt   # 构建配置
```

---

## 🚀 快速开始

### 前提条件

1. **推荐从仓库根目录构建（一次编译全部）**：
   ```bash
   cmake -S . -B build
   cmake --build build -j4
   ```

2. **安装必要工具**：
   - CMake >= 3.10
   - GCC 或其他 C89 兼容编译器

### 编译并运行示例

#### 方式 1：独立编译单个示例

```bash
# minimal_app 示例
cd examples/minimal_app
mkdir build && cd build
cmake ..
make
./minimal_app

# event_driven 示例
cd examples/event_driven
mkdir build && cd build
cmake ..
make
./event_driven
```

#### 方式 2：从根目录编译所有示例

```bash
cd c_ddd
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON
make
./examples/minimal_app/minimal_app
./examples/event_driven/event_driven
```

---

## 📚 示例说明

### 1. minimal_app - 最小示例（DDD依赖正确）

**功能**：
- ✅ 演示组合根（Entry）如何注入仓储端口（平台层提供实现）
- ✅ 演示 CQRS：命令创建/修改聚合，查询读取 DTO
- ✅ 演示领域事件订阅（统计/打印），无全局可变状态

**适用场景**：
- 快速了解 Aegis 的 DDD + CQRS + 严格DI 组装方式
- 参考“业务字段不属于框架”的建模方式（示例：充电桩功率）

**执行流程（概览）**：
1. `aegis_entry_platform_init()` 初始化平台依赖并提供写仓储接口
2. `aegis_entry_init_all()` 初始化 runtime（trace/event bus/app）
3. 注册示例模块：`aegis_app_init_register_modules()`
4. 发起命令：创建 Charger → 设置功率
5. 发起查询：读取 Charger DTO

**预期输出（示例）**：
```
[领域事件] ENTITY_CREATED: id=...
[领域事件] POWER_CHANGED: id=... old=... new=...
[查询结果] id=... model=... power=...
```

---

### 2. event_driven - 事件驱动示例

**功能**：
- ✅ 演示领域事件总线（同步订阅 + 异步订阅 + 全局监听）
- ✅ 演示事件历史与统计（用于排障/追溯）

**适用场景**：
- 学习领域事件总线的使用
- 理解同步和异步事件处理的区别
- 掌握事件订阅配置方法
- 了解事件历史和追溯功能

**执行流程（概览）**：
1. 初始化 Trace 与 EventBus（注册 3 类订阅：创建/更新/全局）
2. 发布 `DOMAIN_EVENT_ENTITY_CREATED`（同步处理）
3. 发布 `DOMAIN_EVENT_ENTITY_UPDATED`（异步队列处理）
4. 打印事件历史与统计

**事件订阅配置**：
```c
static const AegisEventSubscription g_subscriptions[] = {
    /* 事件类型                     处理器函数          同步?  优先级 */
    {DOMAIN_EVENT_ENTITY_CREATED,  on_entity_created,   TRUE,  0},
    {DOMAIN_EVENT_ENTITY_UPDATED,  on_entity_updated,   FALSE, 1},
    {0,                            on_all_events,       FALSE, 10}
};
```

**预期输出（示例）**：
```
[同步事件] ENTITY_CREATED: ID=..., 类型=..., 时间戳=...
[异步事件] ENTITY_UPDATED: ID=..., 时间戳=...
[事件历史] 最近 16 条记录:
```

---

## 🎯 学习路径

建议按以下顺序学习示例：

1. **minimal_app** → 了解框架基本使用
2. **event_driven** → 掌握事件驱动架构

---

## 📖 相关文档

- [框架 README](../../README_NEW.md)
- [领域事件总线设计](../../docs/领域事件总线设计.md)
- [快速使用指南](../../docs/快速使用指南.md)
- [项目总结报告](../../docs/项目总结报告.md)

---

## ⚙️ 自定义配置

### 修改事件队列大小

编辑 `CMakeLists.txt`，添加编译参数：
```cmake
add_compile_definitions(
    DOMAIN_EVENT_QUEUE_SIZE=64        # 增大事件队列
    DOMAIN_EVENT_HISTORY_SIZE=32      # 增大事件历史
)
```

### 切换目标平台

修改 `CMakeLists.txt` 中的 `TARGET_PLATFORM`：
```cmake
set(TARGET_PLATFORM "stm32f4")  # 改为 STM32F4
```

---

## 🐛 常见问题

### Q1: 编译时找不到头文件？

**解决**：确保框架库已编译，或在 CMake 中添加：
```cmake
if(NOT TARGET c_ddd_framework)
    add_subdirectory(${FRAMEWORK_DIR} ${CMAKE_BINARY_DIR}/framework)
endif()
```

### Q2: 链接时找不到符号？

**解决**：检查是否链接了平台移植层：
```cmake
target_link_libraries(your_app
    c_ddd_framework
    your_app_port
)
```

### Q3: 运行时段错误？

**解决**：检查是否初始化了必要模块：
```c
aegis_entry_platform_init(...);  /* 平台依赖初始化（注入仓储/时钟等） */
aegis_entry_init_all(&rt, &cfg); /* 初始化 runtime（会初始化 event bus 等） */
```

---

## 🤝 贡献指南

欢迎提交新的示例应用！请遵循以下规范：

1. **命名**：使用小写+下划线（如 `sensor_monitor`）
2. **结构**：包含 `main.c` 和 `CMakeLists.txt`
3. **注释**：添加详细的中文注释
4. **README**：更新本文档，添加示例说明

---

**作者**：jack liu
**日期**：2025-12-23
**版本**：v1.0
