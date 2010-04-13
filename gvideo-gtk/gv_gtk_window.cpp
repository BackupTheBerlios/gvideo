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
#  GVideo V4L2 video grabber and control panel - GTK main window                #
#                                                                               #
********************************************************************************/

#include <glibmm/object.h>
#include <gtkmm/stock.h>

#include <iostream>
#include <sstream>

#include "gv_gtk_window.h"
#include "gvcommon.h"
#include "libgvideo/GVid_v4l2.h"

START_GVIDEOGTK_NAMESPACE

GtkWindow::GtkWindow(
        libgvideo::GVDevice *dev, 
        libgvaudio::GVAudio *audio, 
        int format /*= 0*/, 
        int resolution /*= 0*/, 
        int fps /*= 0*/ )
{
    gv_ImageLabel= new Gtk::Label("Image");
    gv_VideoLabel= new Gtk::Label("Video");
    gv_AudioLabel= new Gtk::Label("Audio");
    gv_Button_pic= new Gtk::Button("cap picture");
    gv_Button_vid= new Gtk::Button("cap video");
    gv_Button_Quit= new Gtk::Button("Quit");
    set_title("GVideo - GTK GUI");
    set_border_width(10);
    set_default_size(640, 480);

    gv_VBox = new Gtk::VBox();
    gv_Notebook = new Gtk::Notebook();
    gv_ButtonBox = new Gtk::HButtonBox();
    
    add(*gv_VBox);

    
    //Add the Notebook, with the button box underneath:
    gv_Notebook->set_border_width(10);
    gv_VBox->pack_start(*gv_Notebook);
    gv_VBox->pack_start(*gv_ButtonBox, Gtk::PACK_SHRINK);

    gv_ButtonBox->pack_start(*gv_Button_pic, Gtk::PACK_SHRINK);
    gv_Button_pic->signal_clicked().connect(sigc::mem_fun(*this,
              &GtkWindow::on_button_pic) );
    
    gv_ButtonBox->pack_start(*gv_Button_vid, Gtk::PACK_SHRINK);
    gv_Button_vid->signal_clicked().connect(sigc::mem_fun(*this,
              &GtkWindow::on_button_vid) );
    
    gv_ButtonBox->pack_start(*gv_Button_Quit, Gtk::PACK_SHRINK);
    gv_Button_Quit->signal_clicked().connect(sigc::mem_fun(*this,
              &GtkWindow::on_button_quit) );

    //gv_Notebook->signal_switch_page().connect(sigc::mem_fun(*this,
    //          &GtkWindow::on_notebook_switch_page) );
              
              
    controlTable = new Gtk::Table(3, dev->listControls.size(), false);
    videoTable = new Gtk::Table(4, 10, false);
    audioTable = new Gtk::Table(4, 10, false);
    
    //image controls
    int i = 0;
    int j = 0;
    
    for(i=0; i < dev->listControls.size(); i++)
    {
        Gtk::Label  *control_label= new Gtk::Label(dev->listControls[i].name);
        controlTable->attach(*control_label, 0, 1, i, i+1);
        
        switch(dev->listControls[i].type)
        {
            case V4L2_CTRL_TYPE_MENU:
                {
                    Gtk::ComboBoxText* control_widget = new Gtk::ComboBoxText();
                    for (j=0; j<dev->listControls[i].entries.size(); j++)
                    {
                        control_widget->append_text(dev->listControls[i].entries[j]);
                    }
                    control_widget->set_active(dev->listControls[i].default_val);
                    controlTable->attach(*control_widget, 1, 2, i, i+1);
                    //Connect signal handler:
                    control_widget->signal_changed().connect(sigc::bind<Gtk::ComboBoxText*, int>(
                        sigc::mem_fun(*this, &GtkWindow::on_combo_changed), control_widget, i ));

                }
                break;
                
            case V4L2_CTRL_TYPE_BOOLEAN:
                {
                    Gtk::CheckButton *control_widget = new Gtk::CheckButton();
                    control_widget->set_active(dev->listControls[i].default_val);
                    controlTable->attach(*control_widget, 1, 2, i, i+1);
                    //Connect signal handler:
                    control_widget->signal_clicked().connect(sigc::bind<Gtk::CheckButton*, int>(
                        sigc::mem_fun(*this, &GtkWindow::on_check_button_clicked), control_widget, i ));

                }
                break;
                
            case V4L2_CTRL_TYPE_INTEGER:
                {
                    Gtk::HScale *control_widget = new Gtk::HScale(
                        dev->listControls[i].min, 
                        dev->listControls[i].max, 
                        dev->listControls[i].step );
                    control_widget->set_value(dev->listControls[i].default_val);
                    controlTable->attach(*control_widget, 1, 2, i, i+1);
                    //Connect signal handler:
                    control_widget->signal_value_changed().connect(sigc::bind<Gtk::HScale*, int>(
                        sigc::mem_fun(*this, &GtkWindow::on_hscale_value_changed), control_widget, i ));
                }
                break;
        }
    }
    
    //video stream definitions
    
    //stream format
    i=0;
    video_format_label= new Gtk::Label("Stream Format: ");
    videoTable->attach(*video_format_label, 0, 1, i, i+1);
    video_format_combo = new Gtk::ComboBoxText();
    j = 0;
    for (j=0; j< dev->listVidFormats.size(); j++)
    {
        video_format_combo->append_text(dev->listVidFormats[j].fourcc);
    }
    video_format_combo->set_active(format);
    videoTable->attach(*video_format_combo, 1, 2, i, i+1);
    //Connect signal handler:
    video_format_combo->signal_changed().connect(sigc::mem_fun(*this, &GtkWindow::on_video_format_combo_changed));
    
    //resolution
    i++;
    resolution_label= new Gtk::Label("Resolution: ");
    videoTable->attach(*resolution_label, 0, 1, i, i+1);
    resolution_combo = new Gtk::ComboBoxText();
    j = 0;
    for (j=0; j<dev->listVidFormats[format].listVidCap.size(); j++)
    {
        std::stringstream out;
        out << dev->listVidFormats[format].listVidCap[j].width << "x" 
            << dev->listVidFormats[format].listVidCap[j].height;
        resolution_combo->append_text(out.str());
    }
    resolution_combo->set_active(resolution);
    videoTable->attach(*resolution_combo, 1, 2, i, i+1);
    //Connect signal handler:
    resolution_combo->signal_changed().connect(sigc::mem_fun(*this, &GtkWindow::on_resolution_combo_changed));
    
    //fps
    i++;
    fps_label= new Gtk::Label("Frame Rate: ");
    videoTable->attach(*fps_label, 0, 1, i, i+1);
    fps_combo = new Gtk::ComboBoxText();
    
    j = 0;
    for (j=0; j<dev->listVidFormats[format].listVidCap[resolution].fps.size(); j++)
    {
        std::stringstream out;
        out << dev->listVidFormats[format].listVidCap[resolution].fps[j].num << "/" 
            << dev->listVidFormats[format].listVidCap[resolution].fps[j].denom;
        fps_combo->append_text(out.str());
    }
    fps_combo->set_active(fps);
    videoTable->attach(*fps_combo, 1, 2, i, i+1);
    //Connect signal handler:
    fps_combo->signal_changed().connect(sigc::mem_fun(*this, &GtkWindow::on_fps_combo_changed));

    //Audio definitions
    
    //audio devices
    i = 0;
    audio_dev_label = new Gtk::Label("Devices: ");
    audioTable->attach(*audio_dev_label, 0, 1, i, i+1);
    audio_dev_combo = new Gtk::ComboBoxText();
    j = 0;
    for (j=0; j<audio->listAudioDev.size(); j++)
    {
        audio_dev_combo->append_text(audio->listAudioDev[j].name);
    }
    audio_dev_combo->set_active(0);
    audioTable->attach(*audio_dev_combo, 1, 2, i, i+1);
    //Connect signal handler:
    audio_dev_combo->signal_changed().connect(sigc::mem_fun(*this, &GtkWindow::on_audio_dev_combo_changed));
    
    //Add the Notebook pages:
    gv_Notebook->append_page(*controlTable, *gv_ImageLabel);
    gv_Notebook->append_page(*videoTable, *gv_VideoLabel);
    gv_Notebook->append_page(*audioTable, *gv_AudioLabel);
    
    show_all_children();
}


GtkWindow::~GtkWindow()
{
    std::cout << "The Window was destroyed\n";
}

void GtkWindow::on_check_button_clicked(Gtk::CheckButton* checkButton, int cindex)
{
    std::cout << "The Button for control " << cindex << " was clicked: state="
        << (checkButton->get_active() ? "true" : "false")
        << std::endl;
}

void GtkWindow::on_combo_changed(Gtk::ComboBoxText* comboBox, int cindex)
{
    std::cout << "combo from control " << cindex 
    << " changed to index " << comboBox->get_active_row_number()
    << std::endl;
}

void GtkWindow::on_hscale_value_changed(Gtk::HScale* hScale, int cindex)
{
    std::cout << "value for control " << cindex 
    << " changed to " << hScale->get_value()
    << std::endl;
}

void GtkWindow::on_video_format_combo_changed()
{
    //update resolution_combo and fps_combo
}

void GtkWindow::on_resolution_combo_changed()
{
    //update fps_combo
}

void GtkWindow::on_fps_combo_changed()
{

}

void GtkWindow::on_audio_dev_combo_changed()
{

}

void GtkWindow::on_button_pic()
{
    std::cout << "The Picture Button was clicked\n";
}

void GtkWindow::on_button_vid()
{
    std::cout << "The Video Button was clicked\n";
}

void GtkWindow::on_button_quit()
{
    std::cout << "The Quit Button was clicked\n";
    hide();
}

END_GVIDEOGTK_NAMESPACE
