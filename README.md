# gui

#### 介绍
gui框架

#### 软件架构
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


#### 使用说明

1. 移植 `driver/, sources/, src/` 到目标项目中;
2. 在 `driver` 文件夹中, 适配底层 `lcd`, `input`, `file system` 的驱动;
3. 配置 `gt_conf.h`, `gt_conf_widgets.h` 宏开关;
4. 可以开始使用.

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request

