# 迹 (Trace)

一个跨平台文件监控系统，实时监测和记录计算机上的所有文件变动的应用。

## 功能特性

- 多平台支持：Windows、macOS、Linux
- 实时文件变动监控
- SQLite日志记录
- 图形化界面查看每日变动
- 守护进程服务模式
- 数据恢复辅助功能

## 项目结构

```
Trace/
├── CMakeLists.txt          # 顶层 CMake 构建配置
├── cmake/                  # CMake 辅助模块
├── src/
│   ├── core/               # 核心监控库
│   ├── daemon/             # 守护进程
│   ├── gui/                # 图形界面
│   ├── updater/            # 更新程序
│   ├── uninstaller/        # 卸载程序
│   └── common/             # 公共工具
├── config/                 # 配置文件
├── scripts/                # 平台辅助脚本
├── packaging/              # 安装包制作
├── tests/                  # 测试
└── docs/                   # 文档
```

## 许可证

MIT License - 详见 [LICENSE](LICENSE)