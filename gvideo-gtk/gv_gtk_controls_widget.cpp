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

#include "gv_gtk_controls_widget.h"
#include <iostream>

START_GVIDEOGTK_NAMESPACE

GtkPanTilt::GtkPanTilt(libgvideo::GVDevice* device, int index, bool is_pan)
    : Gtk::HBox(true, 2)
{
    _index = index;
    dev = device;
    button_1 = Gtk::manage(new Gtk::Button());
    button_2 = Gtk::manage(new Gtk::Button());
    pt_step = Gtk::manage(new Gtk::SpinButton());
    
    pt_step->set_range(-256,256);
    pt_step->set_increments(64,64);
    
    if(is_pan)
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
    dev->listControls[_index].value = val;
    if(dev->set_control_val (_index))
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[_index].control.name
            << " to " << val << std::endl;
    }
}

void GtkPanTilt::on_button_2()
{
    int val = -(pt_step->get_value_as_int());
    dev->listControls[_index].value = val;
    if(dev->set_control_val (_index))
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[_index].control.name
            << " to " << val << std::endl;
    }
}

GtkControls::GtkControls(libgvideo::GVDevice* device)
    : Gtk::Table(2, device->listControls.size(), false)
{
    dev = device;
    int j = 0;
    int index=0;
    
 for(index=0; index < dev->listControls.size(); index++)
 {   
    Gtk::HBox *hbox = new Gtk::HBox(true, 2); 
    Gtk::Label *control_label= Gtk::manage(new Gtk::Label((char *) dev->listControls[index].control.name));
    control_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
    hbox->pack_start(*control_label, Gtk::PACK_EXPAND_WIDGET);
    
    int val = 0;
    switch(dev->listControls[index].control.type)
    {
        case V4L2_CTRL_TYPE_MENU:
            {
                Gtk::ComboBoxText *control_widget = new Gtk::ComboBoxText();
                for (j=0; j<dev->listControls[index].entries.size(); j++)
                {
                    control_widget->append_text(dev->listControls[index].entries[j]);
                }
                if(dev->get_control_val (index) != 0)
                    control_widget->set_active(dev->listControls[index].control.default_value);
                else
                    control_widget->set_active(dev->listControls[index].value);
                    
                hbox->pack_start(*control_widget, Gtk::PACK_EXPAND_WIDGET);
                //Connect signal handler:
                control_widget->signal_changed().connect(sigc::bind<Gtk::ComboBoxText*, int>(
                    sigc::mem_fun(*this, &GtkControls::on_combo_changed), control_widget, index));

                }
                break;
                
            case V4L2_CTRL_TYPE_BOOLEAN:
                {
                    Gtk::CheckButton *control_widget = new Gtk::CheckButton();
                    if(dev->get_control_val (index) != 0)
                        control_widget->set_active(dev->listControls[index].control.default_value > 0);
                    else
                        control_widget->set_active(dev->listControls[index].value > 0);
                    
                    hbox->pack_start(*control_widget, Gtk::PACK_EXPAND_WIDGET);
                    //Connect signal handler:
                    control_widget->signal_clicked().connect(sigc::bind<Gtk::CheckButton*, int>(
                        sigc::mem_fun(*this, &GtkControls::on_check_button_clicked), control_widget, index));

                }
                break;
                
            case V4L2_CTRL_TYPE_INTEGER:
                {
                    if(dev->listControls[index].control.id == V4L2_CID_PAN_RELATIVE)
                    {
                        GtkPanTilt *control_widget = new GtkPanTilt(dev, index, true);
                        hbox->pack_start(*control_widget, Gtk::PACK_EXPAND_WIDGET);
                    }
                    else if (dev->listControls[index].control.id == V4L2_CID_TILT_RELATIVE)
                    {
                        GtkPanTilt *control_widget = new GtkPanTilt(dev, index, false);
                        hbox->pack_start(*control_widget, Gtk::PACK_EXPAND_WIDGET);
                    }
                    else
                    {
                        Gtk::HScale *control_widget = new Gtk::HScale(
                            dev->listControls[index].control.minimum, 
                            dev->listControls[index].control.maximum, 
                            dev->listControls[index].control.step );
                        if(dev->get_control_val (index) != 0)
                            control_widget->set_value(dev->listControls[index].control.default_value);
                        else
                            control_widget->set_value(dev->listControls[index].value);
                            
                        hbox->pack_start(*control_widget, Gtk::PACK_EXPAND_WIDGET);
                        //Connect signal handler:
                        control_widget->signal_value_changed().connect(sigc::bind<Gtk::HScale*, int>(
                            sigc::mem_fun(*this, &GtkControls::on_hscale_value_changed), control_widget, index));
                    }
                }
                break;
                
            case V4L2_CTRL_TYPE_BUTTON:
                {
                    Gtk::Button *control_widget = new Gtk::Button();
                    hbox->pack_start(*control_widget, Gtk::PACK_EXPAND_WIDGET);
                    //Connect signal handler:
                    control_widget->signal_clicked().connect(sigc::bind<int>(
                        sigc::mem_fun(*this, &GtkControls::on_button_clicked), index));
                }
                break;
            
            default:
                std::cerr << "Control tpye: " << std::hex << std::showbase 
                    << dev->listControls[index].control.type << " not supported\n";
    }
    widgets_list.push_back(hbox);
    
    this->attach(*widgets_list[index], 1, 2, index, index+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
    
  }
    
    show_all_children();
    
    update_widgets();
}

void GtkControls::on_check_button_clicked(Gtk::CheckButton* control_widget, int index)
{
    int val = control_widget->get_active() ? 1 : 0;
    dev->listControls[index].value = val;
    if(!(dev->set_control_val (index)))
        std::cout << dev->listControls[index].control.name << " = " << val << std::endl;
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[index].control.name 
            << " = " << val << std::endl;
            
        if(dev->listControls[index].value) control_widget->set_active(true);
        else control_widget->set_active(false);
    }
    
    update_widgets();
}

void GtkControls::on_combo_changed(Gtk::ComboBoxText* control_widget, int index)
{
    int val = control_widget->get_active_row_number();
    dev->listControls[index].value = val;
    if(!(dev->set_control_val (index)))
            std::cout << dev->listControls[index].control.name << " = " << val << std::endl;
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[index].control.name 
            << " = " << val << std::endl;
        control_widget->set_active(dev->listControls[index].value);
    }
    
    update_widgets();
}

void GtkControls::on_hscale_value_changed(Gtk::HScale* control_widget, int index)
{
    int val = control_widget->get_value();
    dev->listControls[index].value = val;
    if(!(dev->set_control_val (index)))
            std::cout << dev->listControls[index].control.name << " = " << val << std::endl;
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[index].control.name 
            << " = " << val << std::endl;
        control_widget->set_value(dev->listControls[index].value);
    }
}

void GtkControls::on_button_clicked(int index)
{
    dev->listControls[index].value = 1;
    if(!(dev->set_control_val (index)))
    {
        //std::cout << dev->listControls[cindex].name << " = " << val << std::endl;
    }
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[index].control.name << std::endl;
    }
}

void GtkControls::update_widgets()
{
    int i = 0;
    for(i=0; i<dev->listControls.size(); i++)
    {
        if(dev->listControls[i].control.flags & V4L2_CTRL_FLAG_GRABBED)
        {
            widgets_list[i]->set_sensitive(false);
        }
        else
            widgets_list[i]->set_sensitive(true);
    }
}

END_GVIDEOGTK_NAMESPACE
