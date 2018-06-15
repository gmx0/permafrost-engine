#
#  This file is part of Permafrost Engine. 
#  Copyright (C) 2018 Eduard Permyakov 
#
#  Permafrost Engine is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Permafrost Engine is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import pf
from constants import *

def format_str_for_numlist(numlist):
    ret = "["
    for i, num in enumerate(numlist):
        ret += "{0:.1f}".format(num)
        if i != len(numlist)-1:
            ret += ", "
    ret += "]"
    return ret

class ObjectsTabWindow(pf.Window):

    OBJECTS_MODE_PLACE  = 0
    OBJECTS_MODE_SELECT = 1

    def __init__(self):
        _, resy = pf.get_resolution()
        super(ObjectsTabWindow, self).__init__("ObjectsTab", 
            (0, UI_TAB_BAR_HEIGHT + 1, UI_LEFT_PANE_WIDTH, resy - UI_TAB_BAR_HEIGHT - 1), NK_WINDOW_BORDER)
        self.mode = self.OBJECTS_MODE_PLACE
        self.objects_list = []
        self.selected_object_idx = 0

    def update(self):

        # Mode
        self.layout_row_dynamic(20, 1)
        self.label_colored_wrap("Mode:", (255, 255, 255))

        old_mode = self.mode
        self.layout_row_dynamic(20, 2)
        if self.option_label("Place", self.mode == self.OBJECTS_MODE_PLACE):
            self.mode = self.OBJECTS_MODE_PLACE
        if self.option_label("Select", self.mode == self.OBJECTS_MODE_SELECT):
            self.mode = self.OBJECTS_MODE_SELECT
        self.layout_row_dynamic(10, 1)

        if self.mode != old_mode:
            pf.global_event(EVENT_OBJECTS_TAB_MODE_CHANGED, self.mode)

        if self.mode == self.OBJECTS_MODE_PLACE:
            # Objects
            self.layout_row_dynamic(20, 1)
            self.label_colored_wrap("Objects:", (255, 255, 255))

            def objects_group():
                self.layout_row_static(25, UI_LEFT_PANE_WIDTH-60, 1)
                for i in range(0, len(self.objects_list)):
                    old = self.selected_object_idx
                    on = self.selectable_label(self.objects_list[i], 
                        NK_TEXT_ALIGN_LEFT, i == self.selected_object_idx)
                    if on: 
                        self.selected_object_idx = i
                    if self.selected_object_idx != old:
                        pf.global_event(EVENT_OBJECT_SELECTION_CHANGED, i)

            self.layout_row_static(400, UI_LEFT_PANE_WIDTH-30, 1)
            self.group("Objects", NK_WINDOW_BORDER, objects_group)
        elif self.mode == self.OBJECTS_MODE_SELECT:
            # Selection
            sel_obj_list = pf.get_unit_selection()

            if len(sel_obj_list) == 0:
                return

            self.layout_row_dynamic(20, 1)
            self.label_colored_wrap("Selection:", (255, 255, 255))

            if len(sel_obj_list) > 1:
                def selection_group():
                    self.layout_row_static(25, UI_LEFT_PANE_WIDTH-60, 1)
                    for i in range(0, len(sel_obj_list)):
                        name = "{0} {1}".format(sel_obj_list[i].name, format_str_for_numlist(sel_obj_list[i].pos))
                        on = self.selectable_label(name, NK_TEXT_ALIGN_LEFT, False)
                        if on:
                            pf.global_event(EVENT_OBJECT_SELECTED_UNIT_PICKED, sel_obj_list[i])
                self.layout_row_static(400, UI_LEFT_PANE_WIDTH-30, 1)
                self.group("Selection", NK_WINDOW_BORDER, selection_group)
            else:
                assert(len(sel_obj_list) == 1)
                self.layout_row_dynamic(10, 1)
                self.layout_row_dynamic(20, 1)
                self.label_colored_wrap(sel_obj_list[0].name, (200, 200, 0))

                pos_str = "Position: {0}".format(format_str_for_numlist(sel_obj_list[0].pos))
                self.layout_row_dynamic(20, 1)
                self.label_colored_wrap(pos_str, (255, 255, 255))

                rot_str = "Rotation: {0}".format(format_str_for_numlist(sel_obj_list[0].rotation))
                self.layout_row_dynamic(20, 1)
                self.label_colored_wrap(rot_str, (255, 255, 255))

                scale_str = "Scale: {0}".format(format_str_for_numlist(sel_obj_list[0].scale))
                self.layout_row_dynamic(20, 1)
                self.label_colored_wrap(scale_str, (255, 255, 255))

                select_str = "Selectable: {0}".format("True" if sel_obj_list[0].selectable else "False")
                self.layout_row_dynamic(20, 1)
                self.label_colored_wrap(select_str, (255, 255, 255))

            def on_delete():
                pf.global_event(EVENT_OBJECT_DELETE_SELECTION, None)

            self.layout_row_dynamic(30, 1)
            self.button_label("Delete", on_delete)
