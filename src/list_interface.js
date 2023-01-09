export var list_interface = {
    "com":{
        "pos":
        'gt_obj_style_set_pos(?,?,?);',

        "size":
        'gt_obj_style_set_size(?,?,?);',

        "spec.opa":
        'gt_obj_style_set_opa(?,GT_OPA_?);',

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
        'gt_btn_style_set_color_background(?,gt_color_hex(?));'
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
        'gt_slider_style_set_tag(?,"?");'
    },
    "rect":{
        "radius":
        'gt_rect_style_set_radius(?,?);',

        "color.bgcolor":
        "gt_rect_style_set_bg_color(?,gt_color_hex(?));"
    },
    "screen":{
        "color.bgcolor":
        'gt_screen_set_bgcolor(?,gt_color_hex(?));',
    },
}