# font

字体引擎，支持点阵字体和矢量字体，覆盖中文、英文及多国语言。

## 文件说明

| 文件 | 说明 |
|------|------|
| `gt_font.c/h` | 字体引擎核心：字体加载、字符宽高查询、点阵数据获取 |
| `gt_symbol.c/h` | 内置符号字体（图标字符） |
| `gt_custom_font.c/h` | 自定义字体注册接口 |
| `gt_font_config.c/h` | 字体配置（family 表、range-table）— 位于 `driver/` 目录 |

## 字体配置

在 `driver/gt_font_config.c` 中维护 family 映射表，family ID 与字体文件一一对应：

```c
gt_btn_set_font_family(btn, 2);   /* 使用 family 2 的字体 */
```

## 常用 API

```c
/* 获取字符点阵数据 */
_gt_font_dot_ret_st gt_font_get_dot(gt_font_st *font, uint32_t uni_or_gbk);

/* 查询字符宽高（_by_option 版本跳过 range-table 查表，性能更好） */
uint8_t gt_font_get_one_word_width_by_option(uint32_t uni, gt_font_st *font, uint16_t option);
uint8_t gt_font_get_one_word_height_by_option(uint32_t uni, gt_font_st *font, uint16_t option);
```
