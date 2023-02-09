# gui

## 介绍

gui框架

### 软件架构

软件架构说明

| 目录 | 说明 |
| :--- | :--- |
| doc/ | 相关文档说明资料 |
| driver/ | 底层驱动相关的移植代码 |
| examples/ | 控件调用示例 |
| sources/ | gui 内部使用的images素材 |
| test/ | 单元测试代码 |
| src/ | gui 框架代码 |
| \|-gt_conf.h | 配置 |
| \|-core/ | gui 核心代码 |
| \|-extra/ | 拓展控件 |
| \|-font/ | 字体 |
| \|-hal/ | 中间件, 中间层 |
| \|-others/ | 不好分类的代码 |
| \|-utils/ | 工具类 |
| \|-widgets/ | 控件 |
| \|-\|-gt_conf_widgets.h | 控件开关宏 |

### 编译库

1. vscode 安装 **cmake**, **cmake tool** 插件;

2. `Ctrl + shift + p`, 在输入框, 配置 `>CMake` 的 **kits** 工具链, 依次执行 `>CMake Configure`, `>CMake build`;

3. `lib, a` 文件存放在 `{default_word_space}/build/lib` 文件夹下.

gui根目录下的 CmakeList.txt 执行如下脚本:

```cmake

cmake_minimum_required(VERSION 3.0.0)
project(gt_gui)

file(GLOB_RECURSE SRC_LIST "src" "*.c") #递归搜索
add_library(gt_gui ${SRC_LIST})

````

备用编译库的脚本:

```cmake

cmake_minimum_required(VERSION 3.10)

project(gt_gui VERSION 1.0)

# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,dynamic_lookup")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined")

message(STATUS "project name: ${PROJECT_NAME}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# add sub directories
add_subdirectory(src)
add_subdirectory(driver)

```

## 使用说明

1. 移植 `driver/, sources/, src/` 到目标项目中;
2. 在 `driver` 文件夹中, 适配底层 `lcd`, `input`, `file system` 的驱动;
3. 配置 `gt_conf.h`, `gt_conf_widgets.h` 宏开关;
4. 可以开始使用.
