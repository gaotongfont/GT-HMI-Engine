/**
 * change HMI widget name to gui widget name
 * @param {HMI widget name} label
 * @returns gui widget name
 */
export function hmiLabel2guiLabel(label){
    var list = {
        "按钮":"btn",
        "图片":"img",
        "标签":"label",
        "文本区":"textarea",
        "输入框":"input",
        "图片按钮":"imgbtn",
        "键盘":"keypad",
        "开关":"switch",
        "分组":"group",
        "滑动条":"slider",
        "进度条":"progress_bar",
        "单选框":"radio",
        "复选框":"checkbox",
        "列表":"listview",
        "矩形":"rect",
        "直线":"line",
        "曲线":"qline",
        "屏幕":"screen"
    }
    return list[label]
}
/**
 * all interfaces for gui widgets style function
 */
export var list_interface = {
    "com":{
        "pos":
        'gt_obj_style_set_pos(?,?,?);',

        "size":
        'gt_obj_style_set_size(?,?,?);',

        "spec.opa":
        'gt_obj_style_set_opa(?,GT_OPA_?);',

        "text.fontStyleSize":
        'gt_?_style_set_font_size(?, ?);',

        "text.fontStyleCn":
        'gt_?_style_set_font_family_cn(?,?);',

        "text.fontStyleEn":
        'gt_?_style_set_font_family_en(?,?);',

        "text.fontColor":
        'gt_?_style_set_font_color(?,gt_color_hex(?));',

        "visible":
        'gt_obj_set_visible(?,?);',
    },
    "btn":{
        "text.value":
        'gt_btn_style_set_text(?,"?");',

        "color.bgcolor":
        'gt_btn_style_set_color_background(?,gt_color_hex(?));',

        "btn.radius":
        "gt_btn_style_set_radius(?,?);",
    },
    "checkbox":{
        "text.value":
        'gt_checkbox_style_set_text(?,"?");',

        "box.checked":
        'gt_obj_style_set_state(?,?);',
    },
    "img":{
        "image.url":
        'gt_img_style_set_src(?,"?");',
    },
    "imgbtn":{
        "imageButton.url":
        'gt_imgbtn_style_set_src(?,"?");',

        "imageButton.urlPress":
        'gt_imgbtn_style_set_src_press(?,"?");',

        "imageButton.urlRelease":
        'gt_imgbtn_style_set_src_release(?,"?");',
    },
    "input":{
        "text.value":
        'gt_input_style_set_value(?,"?");',

        "input.placeholder":
        'gt_input_style_set_placeholder(?,"?");',

        /* event */
        "color.bgcolor":
        "gt_input_style_set_bg_color(?,gt_color_hex(?));"
    },
    "label":{
        "text.value":
        'gt_label_style_set_text(?,"?");',
    },
    "progress_bar":{
        "bar.start":
        'gt_progress_bar_style_set_start_end(?,?,?);',

        "bar.pos":
        'gt_progress_bar_style_set_pos(?,?);',

        "bar.colorAct":
        'gt_progress_bar_style_set_color_act(?,gt_color_hex(?));',

        "bar.colorIna":
        'gt_progress_bar_style_set_color_ina(?,gt_color_hex(?));',

        'bar.orientation':
        'gt_progress_bar_style_set_dir(?,?);'
    },
    "radio":{
        "text.value":
        'gt_radio_style_set_text(?,"?");',

        "box.checked":
        'gt_radio_style_set_selected(?);',
    },
    "slider":{
        "bar.start":
        'gt_slider_style_set_start_end(?,?,?);',

        "bar.pos":
        'gt_slider_style_set_pos(?,?);',

        "bar.colorAct":
        'gt_slider_style_set_color_act(?,gt_color_hex(?));',

        "bar.colorIna":
        'gt_slider_style_set_color_ina(?,gt_color_hex(?));',

        'bar.tagVisible':
        'gt_slider_style_set_tag_visible(?,?);',

        'bar.tag':
        'gt_slider_style_set_tag(?,"?");',

        'bar.orientation':
        'gt_slider_style_set_dir(?,?);'
    },
    "switcher":{
        "switch.colorAct":
        'gt_switch_style_set_color_act(?,gt_color_hex(?));',

        "switch.colorIna":
        'gt_switch_style_set_color_ina(?,gt_color_hex(?));',

        "switch.colorPoint":
        'gt_switch_style_set_color_point(?,gt_color_hex(?));',

        "switch.choice":
        'gt_obj_style_set_state(?,?);',
    },
    "rect":{
        "rect.radius":
        'gt_rect_style_set_radius(?,?);',

        "rect.isFill":
        'gt_rect_style_set_fill(?,?);',

        "rect.border":
        'gt_rect_style_set_border(?,?);',

        "rect.bgcolor":
        "gt_rect_style_set_bg_color(?,gt_color_hex(?));",

        "rect.borderColor":
        "gt_rect_style_set_color_border(?,gt_color_hex(?));",

        /* event */
        "color.bgcolor":
        "gt_rect_style_set_bg_color(?,gt_color_hex(?));",
    },
    "textarea":{
        "text.value":
        'gt_textarea_style_set_text(?, "?");',

        "textarea.items":
        'gt_textarea_style_add_str(?,"?",?,gt_color_hex(?));',

       /* event */
       "color.bgcolor":
       "gt_textarea_style_set_bg_color(?,gt_color_hex(?));"
    },
    "listview":{
        "listview.max":
        'gt_listview_style_set_cnt_show(?,?);',

        "listview.items":
        'gt_listview_style_add_item(?,"?");',
    },
    "keypad":{
        "keypad.boardColor":
        "gt_keypad_style_set_color_board(?,gt_color_hex(?));",

        "keypad.keyColor":
        "gt_keypad_style_set_color_key(?,gt_color_hex(?));",

        "keypad.ctrlColor":
        "gt_keypad_style_set_color_ctrl_key(?,gt_color_hex(?));",

        "keypad.relevanceInput":
        "gt_keypad_style_set_target(?,?);",
    },
    "screen":{
        "color.bgcolor":
        'gt_screen_set_bgcolor(?,gt_color_hex(?));',
    },


    //////anim
    "anim":{
        "pos":
        'static gt_anim_param_st param;\n' +
        '\tgt_anim_param_init(&param);\n' +
        '\tgt_anim_param_set_time(&param, ?, ?);\n' +
        '\tgt_anim_param_set_dst_point(&param, ?, ?);\n' +
        '\tgt_anim_param_set_path_type(&param, GT_ANIM_PATH_TYPE_OVERSHOOT);\n' +
        '\tgt_anim_pos_move(?, &param);\n'
    },
}