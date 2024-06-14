#pragma once

#include <vector>

#include "liblvgl/lvgl.h"

class Screen {
    public:
        lv_obj_t* screen;

        Screen();

        lv_obj_t* get_parent(
            lv_obj_t* parent
        );

        lv_obj_t* add_obj(
            lv_obj_t* parent
            , lv_coord_t x
            , lv_coord_t y
            , lv_coord_t w
            , lv_coord_t h
        );

        lv_obj_t* add_label(
            lv_obj_t* obj
            , const char* text
            , bool align
        );

        lv_obj_t* add_btn(
            lv_obj_t* parent
            , lv_coord_t x
            , lv_coord_t y
            , lv_coord_t w
            , lv_coord_t h
            , lv_event_cb_t cb
            , lv_event_code_t filter
            , void* user_data
        );
};
