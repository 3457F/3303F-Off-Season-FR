#include "main.h"

Screen::Screen() {
    screen = lv_obj_create(NULL);
}

lv_obj_t* Screen::get_parent(
    lv_obj_t* parent
) {
    if (parent != nullptr) {
        return parent;
    } else {
        return screen;
    }
}

lv_obj_t* Screen::add_obj(
    lv_obj_t* parent
    , lv_coord_t x
    , lv_coord_t y
    , lv_coord_t w
    , lv_coord_t h
) {
    lv_obj_t* new_obj = lv_obj_create(this->get_parent(parent));

    lv_obj_set_pos(new_obj, x, y);
    lv_obj_set_size(new_obj, w, h);

    return new_obj;
}

lv_obj_t* Screen::add_label(
    lv_obj_t* obj
    , const char* text
    , bool align
) {
    lv_obj_t* new_label = lv_label_create(obj);
    lv_label_set_text(new_label, text);

    if (align) {
        lv_obj_center(new_label);
    }
}

lv_obj_t* Screen::add_btn(
    lv_obj_t* parent
    , lv_coord_t x
    , lv_coord_t y
    , lv_coord_t w
    , lv_coord_t h
    , lv_event_cb_t cb
    , lv_event_code_t filter
    , void* user_data
) {
    lv_obj_t* new_btn = lv_btn_create(this->get_parent(parent));

    lv_obj_set_pos(new_btn, x, y);
    lv_obj_set_size(new_btn, w, h);

    lv_obj_add_event_cb(new_btn, cb, filter, user_data);

    return new_btn;
}


