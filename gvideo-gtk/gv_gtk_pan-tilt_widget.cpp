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

#include "gv_gtk_pan-tilt_widget.h"
#include <iostream>

START_GVIDEOGTK_NAMESPACE

GtkPanTilt::GtkPanTilt(libgvideo::GVDevice* device, int index, bool is_pan)
    : Gtk::HBox(true, 2)
{
    _is_pan = is_pan;
    _index = index;
    dev = device;
    button_1 = Gtk::manage(new Gtk::Button());
    button_2 = Gtk::manage(new Gtk::Button());
    pt_step = Gtk::manage(new Gtk::SpinButton());
    
    pt_step->set_range(-256,256);
    pt_step->set_increments(64,64);
    
    if(_is_pan)
    {
        button_1->set_label("Left");
        button_2->set_label("Right");
        pt_step->set_value(-128);
    }
    else
    {
        button_1->set_label("Up");
        button_2->set_label("Down");
        pt_step->set_value(-128);
    }
    
    this->pack_start(*button_1, Gtk::PACK_EXPAND_WIDGET);
    this->pack_start(*button_2, Gtk::PACK_EXPAND_WIDGET);
    this->pack_start(*pt_step, Gtk::PACK_EXPAND_WIDGET);
    
    //connect button signals
    button_1->signal_clicked().connect(sigc::mem_fun(*this,
              &GtkPanTilt::on_button_1) );
    button_2->signal_clicked().connect(sigc::mem_fun(*this,
              &GtkPanTilt::on_button_2) );
              
    show_all_children();
}

void GtkPanTilt::on_button_1()
{
    int val = pt_step->get_value_as_int();
    if(dev->set_control_val (_index, val))
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[_index].name.c_str()
            << " to " << val << std::endl;
    }
}

void GtkPanTilt::on_button_2()
{
    int val = -(pt_step->get_value_as_int());
    if(dev->set_control_val (_index, val))
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[_index].name.c_str()
            << " to " << val << std::endl;
    }
}


END_GVIDEOGTK_NAMESPACE
