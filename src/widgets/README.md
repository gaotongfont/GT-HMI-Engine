# GT-HMI Widgets

所有控件均继承自 `gt_obj_st`，通过 `gt_obj_class_st` 注册 init / deinit / event 回调。
创建控件后通过事件系统驱动重绘，支持透明度、焦点、布局等通用属性。

---

## 基础显示

| 控件 | 文件 | 说明 |
|------|------|------|
| **Label** | `gt_label.h` | 文本标签，支持多行、字体、颜色、对齐方式 |
| **Img** | `gt_img.h` | 图片显示，支持 PNG/SJPG/GIF/RAW，可旋转缩放（需 `GT_USE_IMG_TRANSFORM`） |
| **Rect** | `gt_rect.h` | 矩形色块，支持圆角、填充色、边框 |
| **Line** | `gt_line.h` | 直线绘制，支持颜色和线宽 |
| **Arc** | `gt_arc.h` | 圆弧/扇形，支持起止角度、宽度、颜色，常用于仪表盘 |
| **WordArt** | `gt_wordart.h` | 艺术字，支持渐变色文字效果 |

---

## 交互控件

| 控件 | 文件 | 说明 |
|------|------|------|
| **Btn** | `gt_btn.h` | 按钮，支持文字、背景色、圆角，响应点击事件 |
| **ImgBtn** | `gt_imgbtn.h` | 图片按钮，用图片作为按钮背景，支持按下态切换图片 |
| **Checkbox** | `gt_checkbox.h` | 复选框，支持选中/未选中状态切换 |
| **Radio** | `gt_radio.h` | 单选按钮，同组内互斥选择 |
| **Switch** | `gt_switch.h` | 开关控件，滑动切换 on/off 状态 |
| **Slider** | `gt_slider.h` | 滑动条，拖动设置数值，支持水平/垂直方向 |
| **InputNumber** | `gt_input_number.h` | 数字输入框，带加减按钮，限定数值范围 |
| **Option** | `gt_option.h` | 选项控件，单选/多选选项列表 |

---

## 文本输入

| 控件 | 文件 | 说明 |
|------|------|------|
| **Input** | `gt_input.h` | 单行文本输入框，支持普通文本和密码模式 |
| **Textarea** | `gt_textarea.h` | 多行文本区域，支持滚动和只读模式 |
| **Keypad** | `gt_keypad.h` | 软键盘，配合 Input/Textarea 使用，支持自定义按键布局 |

---

## 进度与数据展示

| 控件 | 文件 | 说明 |
|------|------|------|
| **ProgressBar** | `gt_progress_bar.h` | 进度条，支持水平/垂直方向，可设置颜色和进度值 |
| **Graphs** | `gt_graphs.h` | 折线图/曲线图/柱状图/散点图，支持多数据系列 |
| **Spectrum** | `gt_spectrum.h` | 频谱图，柱状频谱显示，适用于音频可视化 |
| **SpectrumLine** | `gt_spectrum_line.h` | 线型频谱图，折线形式的频谱显示 |
| **Table** | `gt_table.h` | 表格控件，支持多行多列，可设置单元格内容和样式 |

---

## 列表与滚动

| 控件 | 文件 | 说明 |
|------|------|------|
| **Listview** | `gt_listview.h` | 列表视图，支持滚动、条目点击、图文混排 |
| **Roller** | `gt_roller.h` | 滚轮选择器，上下滚动选择选项，触发 `GT_EVENT_TYPE_UPDATE_VALUE` |
| **Select** | `gt_select.h` | 下拉选择框，由输入框和列表组合而成 |
| **ViewPager** | `gt_view_pager.h` | 翻页视图，支持左右滑动切换页面 |
| **BtnMap** | `gt_btnmap.h` | 按钮矩阵，批量创建规则排列的按钮组 |

---

## 动画与媒体

| 控件 | 文件 | 说明 |
|------|------|------|
| **Gif** | `gt_gif.h` | GIF 动图播放控件 |
| **Player** | `gt_player.h` | 帧序列播放器，按规则切换内容，支持轮播/动画效果 |
| **MediaPlayer** | `gt_media_player.h` | 媒体播放器 UI，包含播放/暂停/进度等控制元素 |
| **Lrc** | `gt_lrc.h` | 歌词滚动控件，同步显示 LRC 格式歌词 |

---

## 弹窗与通知

| 控件 | 文件 | 说明 |
|------|------|------|
| **Dialog** | `gt_dialog.h` | 模态对话框，支持标题、内容、确认/取消按钮 |
| **Toast** | `gt_toast.h` | 轻提示，短暂显示后自动消失 |

---

## 时间与日期

| 控件 | 文件 | 说明 |
|------|------|------|
| **Clock** | `gt_clock.h` | 时钟控件，支持模拟表盘或数字显示 |
| **Calendar** | `gt_calendar.h` | 日历控件，支持月份切换和日期选择 |

---

## 编码与识别

| 控件 | 文件 | 说明 |
|------|------|------|
| **QRCode** | `gt_qrcode.h` | 二维码生成控件，输入字符串自动生成 QR 码图像 |
| **QrExt** | `gt_qr_ext.h` | 扩展二维码控件，支持更多 QR 码配置选项 |
| **Barcode** | `gt_barcode.h` | 条形码生成控件 |

---

## 富文本

| 控件 | 文件 | 说明 |
|------|------|------|
| **Markdown** | `gt_markdown.h` | Markdown 渲染控件，基于 md4c 库，支持 h1~h6、段落、有序/无序列表等基础语法 |
| **Chat** | `gt_chat.h` | 聊天气泡控件，支持左右对齐的消息气泡布局 |

---

## 布局与容器

| 控件 | 文件 | 说明 |
|------|------|------|
| **Group** | `gt_group.h` | 控件组，管理一组成员控件的焦点和事件 |
| **CrossLayout** | `gt_cross_layout.h` | 十字布局容器，子控件按上下左右方向自动排列 |
| **StatusBar** | `gt_status_bar.h` | 状态栏，固定显示在屏幕顶部，显示系统状态信息 |

> **Flex 布局**：通过 `gt_layout_init(container, &ct)` 为任意容器（如 `gt_rect`）启用 Flex 排列，
> 支持 ROW / COLUMN 方向、SPACE_BETWEEN / CENTER 等对齐方式。
> 运行时可用 `gt_layout_set_type(container, GT_LAYOUT_TYPE_FLEX/FIXED)` 动态切换，
> 切换时自动保存/恢复子元素相对坐标。

---

## 内部基础类

| 文件 | 说明 |
|------|------|
| `gt_obj.h` | 所有控件的基类，定义通用属性（位置、尺寸、透明度、事件等） |
| `gt_obj_class.h` | 控件类注册机制，定义 `gt_obj_class_st` 结构和创建接口 |
| `gt_conf_widgets.h` | 控件功能开关宏，通过 `GT_CFG_ENABLE_*` 按需裁剪编译 |
| `gt_template.h` | 新控件开发模板，提供标准的 init/deinit/event 框架 |
