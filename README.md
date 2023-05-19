# GT-HMI-Engine

## 介绍

GT-HMI ：专为国内嵌入式GUI设计开发打造的免费神器！

GT-HMI（Human Machine Interface） 国产开源GUI框架及永久免费设计软件。它提供创建具有易于使用的图形元素，精美的视觉效果和低内存占用的嵌入式GUI所需的一切。针对国内市场及用户需求，使用习惯进行了优化设计，为国内工程师量身打造！

产品由中国高通GENITOP倾心打造，继承了中国高通30年的汉显技术及汉字技术积累。产品分为上位机GT-HMI Designer和下位机GT-HMI Engine。

## 特性

- 上位机设计软件GT-HMI Designer永久免费，下位机技术框架GT-HMI Engine开源，使用无拘束；

- GT-HMI Designer内嵌下位机技术框架编译器，可自动生成下位机代码；

- 提供GT-HMI模块，已适配GT-HMI上下位机软件及驱动，可以用于前期开发调试，也可以直接作为显示模块使用；

- 多平台兼容，移植便捷；

- 上位机设计软件GT-HMI Designer；

- 可直接生成交互代码，免去写代码的繁琐工作；

- 模拟器仿真即见即所得；

- 内置了大量常用的组件，如按钮、文本框、进度条、单选框等；

- 支持高通全系列点阵/矢量字库，支持中外文及小语种，多种字号及字体；

- 内置中英文及数字输入法；

- 持续更新的GUI示例库和UI资源库；

- 下位机技术框架GT-HMI Engine；

- 纯C语言编写，使用无门槛；

- 小巧高效，不限平台，最小仅需24K RAM+32K FLASH，可运行在Cortex-ARM M0\M3等小资源平台；

- 移植便捷，切换平台只需移植定时器、TP和LCD接口，并提供移植教程及示例；

- 丰富详实的例程，配套的开发套件易于上手；

- 支持自定义功能开发，可用GT-HMI Engine自定义控件和功能，组合进GT-HMI Designer生成的标准代码；

- 带系统（如RTOS、Linux等）和裸机均可使用。


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
