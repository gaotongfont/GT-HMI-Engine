# GT-HMI-Engine Examples

GT-HMI-Engine 控件库示例集，覆盖 39 个控件的基础用法与进阶特性。

## 快速开始

示例系统通过屏幕栈管理所有示例页面，入口为主菜单。

```c
// 在应用初始化时调用
gt_example_init();
```

## 目录结构

```
examples/
├── gt_examples.h        # 屏幕 ID 枚举 + GT_EXAMPLE_ADD_BACK_BTN 宏
├── gt_examples.mk       # 构建配置
├── gt_example_init.c    # 注册所有示例屏幕，设置首页
├── gt_example_menu.c    # 主导航菜单
└── widgets/             # 39 个控件示例
```

## 示例列表

### 基础控件

| 示例 | 文件 | 要点 |
|------|------|------|
| 按钮 | gt_example_button.c | 点击计数，事件回调 |
| 复选框 | gt_example_checkbox.c | `GT_STATE_PRESSED` 状态检测 |
| 标签 | gt_example_label.c | 多行 / 裁剪 / 省略号 / 自动滚动 5 种模式 |
| 输入框 | gt_example_input.c | 占位符，文本追加 |
| 滑块 | gt_example_slider.c | 范围 0-100，步长 5，颜色定制 |
| 列表 | gt_example_listview.c | 20 项，行高 40px，预选第 2 项 |
| 矩形 | gt_example_rect.c | 基础图形 |
| 线条 | gt_example_line.c | 基础图形 |
| 图片 | gt_example_img.c | 图片显示 |
| 图片按钮 | gt_example_img_button.c | 图片按钮 |

### 进阶布局与样式

| 示例 | 文件 | 要点 |
|------|------|------|
| Flex 布局 | gt_example_flex_layout.c | ROW + SPACE_BETWEEN，运行时切换 FLEX/FIXED |
| 动态样式 | gt_example_dynamic_style.c | 颜色循环、尺寸变化、可见性切换 |
| 交叉布局 | gt_example_cross_layout.c | 交叉轴排列 |

### 交互控件

| 示例 | 文件 |
|------|------|
| 对话框 | gt_example_dialog.c |
| 键盘 | gt_example_keyboard.c |
| 滚轮 | gt_example_roller.c |
| 下拉选择 | gt_example_select.c |
| 单选按钮 | gt_example_radio.c |
| 开关 | gt_example_switch.c |
| 数字输入 | gt_example_input_number.c |
| 表单 | gt_example_form.c |
| 按钮映射 | gt_example_btnmap.c |
| 选项 | gt_example_option.c |

### 数据展示

| 示例 | 文件 |
|------|------|
| 进度条 | gt_example_progress_bar.c |
| 圆弧 | gt_example_arc.c |
| 图表 | gt_example_graphs.c |
| 表格 | gt_example_table.c |
| 日历 | gt_example_calendar.c |
| 时钟 | gt_example_clock.c |
| 状态栏 | gt_example_status_bar.c |
| 条形码 | gt_example_barcode.c |

### 媒体与动画

| 示例 | 文件 |
|------|------|
| 播放器 | gt_example_player.c |
| 翻页视图 | gt_example_view_pager.c |
| 动画 | gt_example_anim1.c |
| Toast 提示 | gt_example_toast.c |
| 聊天界面 | gt_example_chat.c |
| 艺术字 | gt_example_wordart.c |
| 控件分组 | gt_example_group.c |
| 多行文本 | gt_example_textarea.c |

## 编写新示例

```c
#include "../gt_examples.h"

gt_obj_st* gt_example_my_widget(void) {
    gt_obj_st* scr = gt_obj_create(NULL);

    // 创建并配置控件
    gt_obj_st* btn = gt_button_create(scr);
    gt_obj_set_pos(btn, 100, 100);
    gt_obj_add_event_cb(btn, _my_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(scr);  // 右上角返回按钮
    return scr;
}
```

注册步骤：
1. 在 `gt_examples.h` 的 `gt_example_scr_id_et` 枚举中添加新 ID
2. 在 `gt_examples.h` 中声明函数原型
3. 在 `gt_example_init.c` 的 `_example_scr_list` 数组中注册
4. 在 `gt_example_menu.c` 的菜单列表中添加入口

## 导航机制

- 屏幕栈容量：10 层
- 菜单切换动画：`MOVE_LEFT`，时长 300ms
- 返回按钮位置：屏幕右上角 (410, 10)，尺寸 70×30
