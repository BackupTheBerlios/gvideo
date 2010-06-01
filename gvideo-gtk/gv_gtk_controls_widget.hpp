/*******************************************************************************#
#           gvideo              http://gvideo.berlios.de                        #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#                                                                               #
# This program is free software; you can redistribute it and/or modify          #
# it under the terms of the GNU General Public License as published by          #
# the Free Software Foundation; either version 2 of the License, or             #
# (at your option) any later version.                                           #
#                                                                               #
# This program is distributed in the hope that it will be useful,               #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 #
# GNU General Public License for more details.                                  #
#                                                                               #
# You should have received a copy of the GNU General Public License             #
# along with this program; if not, write to the Free Software                   #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     #
#                                                                               #
********************************************************************************/

/*******************************************************************************#
#                                                                               #
#  GVideo V4L2 video grabber and control panel - Gtk  pan-tilt widget           #
#                                                                               #
********************************************************************************/

#ifndef GVGTK_PAN_TILT_WIDGET_H
#define GVGTK_PAN_TILT_WIDGET_H



#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/scale.h>

#include <vector>
#include "libgvideo/GVid_v4l2.h"
#include "gvcommon.h"

START_GVIDEOGTK_NAMESPACE

class GtkPanTilt : public Gtk::HBox
{
  protected:
    //Signal handlers:
    void on_button_1();
    void on_button_2();
    Gtk::Button *button_1;
    Gtk::Button *button_2;
    Gtk::SpinButton *pt_step;
    int _index;
    libgvideo::GVDevice* dev;
  public:
    GtkPanTilt(libgvideo::GVDevice* device, int index, bool is_pan);
};


class GtkControls: public Gtk::Table
{
  protected:
    void on_button_clicked(int index);
    void on_check_button_clicked(Gtk::CheckButton* control_widget, int index);
    void on_combo_changed(Gtk::ComboBoxText* control_widget, int index);
    void on_hscale_value_changed(Gtk::HScale* control_widget, int index);
    std::vector<Gtk::HBox*> widgets_list;
    void update_widgets();
    libgvideo::GVDevice* dev;
    
  public:
    GtkControls(libgvideo::GVDevice* device);
};


END_GVIDEOGTK_NAMESPACE
#endif
