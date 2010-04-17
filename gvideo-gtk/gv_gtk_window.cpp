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
#include "libgvencoder/GVCodec.h"

START_GVIDEOGTK_NAMESPACE

GtkWindow::GtkWindow(
        libgvideo::GVDevice* _dev, 
        libgvaudio::GVAudio* _audio,
        GVVideoThreads* _th_video /*= NULL*/,
        int _format /*= 0*/, 
        int _resolution /*= 0*/, 
        int _fps /*= 0*/ )
{
    dev = _dev;
    audio = _audio;
    th_video = _th_video;
    format = _format;
    resolution = _resolution;
    fps = _fps;

    vcodec_ind = 0;
    acodec_ind = 0;
    
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
        controlTable->attach(*control_label, 0, 1, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
        
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
                    controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
                    //Connect signal handler:
                    control_widget->signal_changed().connect(sigc::bind<Gtk::ComboBoxText*, int>(
                        sigc::mem_fun(*this, &GtkWindow::on_combo_changed), control_widget, i ));

                }
                break;
                
            case V4L2_CTRL_TYPE_BOOLEAN:
                {
                    Gtk::CheckButton *control_widget = new Gtk::CheckButton();
                    control_widget->set_active(dev->listControls[i].default_val);
                    controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
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
                    controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
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
    videoTable->attach(*video_format_combo, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
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
    videoTable->attach(*resolution_combo, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
    //Connect signal handler:
    signalResolutionCombo = resolution_combo->signal_changed().connect(
        sigc::mem_fun(*this, &GtkWindow::on_resolution_combo_changed));
    
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
    videoTable->attach(*fps_combo, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
    //Connect signal handler:
    signalFpsCombo = fps_combo->signal_changed().connect(
        sigc::mem_fun(*this, &GtkWindow::on_fps_combo_changed));

    //Encoder definitions
    /*get a new encoder instance*/
    libgvencoder::GVCodec*  encoder= new libgvencoder::GVCodec();
    
    //Video Codec
    i++;
    vcodec_label= new Gtk::Label("Video Codec: ");
    videoTable->attach(*vcodec_label, 0, 1, i, i+1);
    vcodec_combo = new Gtk::ComboBoxText();
    
    j = 0;
    for (j=0; j<encoder->vcodec_list.size(); j++)
    {
        vcodec_combo->append_text(encoder->vcodec_list[j].description);
    }
    vcodec_combo->set_active(vcodec_ind);
    videoTable->attach(*vcodec_combo, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
    //Connect signal handler:
    vcodec_combo->signal_changed().connect(
        sigc::mem_fun(*this, &GtkWindow::on_vcodec_combo_changed));
    
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
    audio_dev_index = 0;
    audio_dev_combo->set_active(audio_dev_index);
    audioTable->attach(*audio_dev_combo, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
    //Connect signal handler:
    audio_dev_combo->signal_changed().connect(sigc::mem_fun(*this, &GtkWindow::on_audio_dev_combo_changed));

    //Audio definitions
    i++;
    acodec_label= new Gtk::Label("Audio Codec: ");
    audioTable->attach(*acodec_label, 0, 1, i, i+1);
    acodec_combo = new Gtk::ComboBoxText();
    
    j = 0;
    for (j=0; j<encoder->acodec_list.size(); j++)
    {
        acodec_combo->append_text(encoder->acodec_list[j].description);
    }
    acodec_combo->set_active(acodec_ind);
    audioTable->attach(*acodec_combo, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
    //Connect signal handler:
    acodec_combo->signal_changed().connect(
        sigc::mem_fun(*this, &GtkWindow::on_acodec_combo_changed));
        
    delete encoder;
    
    //Add the Notebook pages:
    gv_Notebook->append_page(*controlTable, *gv_ImageLabel);
    gv_Notebook->append_page(*videoTable, *gv_VideoLabel);
    gv_Notebook->append_page(*audioTable, *gv_AudioLabel);
    
    //save current resolution
    width = dev->listVidFormats[format].listVidCap[resolution].width;
    height = dev->listVidFormats[format].listVidCap[resolution].height;
    //save current frame rate
    frate.num = dev->listVidFormats[format].listVidCap[resolution].fps[fps].num;
    frate.denom = dev->listVidFormats[format].listVidCap[resolution].fps[fps].denom;
    
    show_all_children();
}


GtkWindow::~GtkWindow()
{
    std::cout << "The Window was destroyed\n";
}

void GtkWindow::on_check_button_clicked(Gtk::CheckButton* checkButton, int cindex)
{
    int val = checkButton->get_active() ? 1 : 0;
    if(!(dev->set_control_val (cindex, val)))
            std::cout << dev->listControls[cindex].name << " = " << val << std::endl;
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[cindex].name << " = " << val << std::endl;
        dev->get_control_val (cindex, &val);
        if(val) checkButton->set_active(true);
        else checkButton->set_active(false);
    }
}

void GtkWindow::on_combo_changed(Gtk::ComboBoxText* comboBox, int cindex)
{
    int val = comboBox->get_active_row_number();
    
   if(!(dev->set_control_val (cindex, val)))
            std::cout << dev->listControls[cindex].name << " = " << val << std::endl;
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[cindex].name << " = " << val << std::endl;
        dev->get_control_val (cindex, &val);
        comboBox->set_active(val);
    }
}

void GtkWindow::on_hscale_value_changed(Gtk::HScale* hScale, int cindex)
{
    int val = hScale->get_value();
    
    if(!(dev->set_control_val (cindex, val)))
            std::cout << dev->listControls[cindex].name << " = " << val << std::endl;
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[cindex].name << " = " << val << std::endl;
        dev->get_control_val (cindex, &val);
        hScale->set_value(val);
    }
}

void GtkWindow::on_video_format_combo_changed()
{
    //get new format
    format = video_format_combo->get_active_row_number();
    
    //block the resolution comboBox changed signal
    signalResolutionCombo.block();
    
    //update resolution_combo
    resolution_combo->clear_items();
    
    int j = 0;
    for (j=0; j<dev->listVidFormats[format].listVidCap.size(); j++)
    {
        std::stringstream out;
        out << dev->listVidFormats[format].listVidCap[j].width << "x" 
            << dev->listVidFormats[format].listVidCap[j].height;
        resolution_combo->append_text(out.str());
        if((width == dev->listVidFormats[format].listVidCap[j].width) &&
            (height == dev->listVidFormats[format].listVidCap[j].height))
            resolution = j;
    }
    if(resolution >= dev->listVidFormats[format].listVidCap.size())
        resolution = dev->listVidFormats[format].listVidCap.size() -1;
    
    //unblock the fps combo changed signal
    signalResolutionCombo.unblock();
    
    resolution_combo->set_active(resolution);

    //apply changes
    std::cout << "format changed to index " << format << std::endl;
    
}

void GtkWindow::on_resolution_combo_changed()
{
    //get new resolution
    resolution = resolution_combo->get_active_row_number();
    
    //save resolution
    width = dev->listVidFormats[format].listVidCap[resolution].width;
    height = dev->listVidFormats[format].listVidCap[resolution].height;
    
    //block fps comboBox changed signal
    signalFpsCombo.block();
    
    //update fps_combo
    fps_combo->clear_items();
    int j = 0;
    for (j=0; j<dev->listVidFormats[format].listVidCap[resolution].fps.size(); j++)
    {
        std::stringstream out;
        out << dev->listVidFormats[format].listVidCap[resolution].fps[j].num << "/" 
            << dev->listVidFormats[format].listVidCap[resolution].fps[j].denom;
        fps_combo->append_text(out.str());
        if((frate.num == dev->listVidFormats[format].listVidCap[resolution].fps[j].num ) &&
            (frate.denom == dev->listVidFormats[format].listVidCap[resolution].fps[j].denom))
            fps = j;
    }
    if(fps >= dev->listVidFormats[format].listVidCap[resolution].fps.size())
        fps = dev->listVidFormats[format].listVidCap[resolution].fps.size() -1;
    
    //unblock fps comboBox changed signal
    signalFpsCombo.unblock();
    
    fps_combo->set_active(fps);
    
    //apply changes
    std::cout << "resolution changed to index " << resolution << std::endl;
    th_video->set_format(dev->listVidFormats[format].fourcc, width, height);
}

void GtkWindow::on_fps_combo_changed()
{
    //get new fps
    fps = fps_combo->get_active_row_number();
    
    //save frame rate
    frate.num = dev->listVidFormats[format].listVidCap[resolution].fps[fps].num;
    frate.denom = dev->listVidFormats[format].listVidCap[resolution].fps[fps].denom;
    
    //apply changes
    std::cout << "fps changed to index " << fps << std::endl;
    dev->set_fps(&frate);
}

void GtkWindow::on_vcodec_combo_changed()
{
    vcodec_ind = vcodec_combo->get_active_row_number();
}

void GtkWindow::on_acodec_combo_changed()
{
    acodec_ind = acodec_combo->get_active_row_number();
}

void GtkWindow::on_audio_dev_combo_changed()
{
    audio_dev_index = audio_dev_combo->get_active_row_number();
}

void GtkWindow::on_button_pic()
{
    th_video->cap_image("file_gvideo.jpg");
}

void GtkWindow::on_button_vid()
{
    if(th_video->is_capturing_video())
    {
        gv_Button_vid->set_label("cap video");
        th_video->stop_video_capture();
    }
    else
    {
        gv_Button_vid->set_label("stop video");
        th_video->start_video_capture(audio, audio_dev_index, vcodec_ind, acodec_ind);
    }
}

void GtkWindow::on_button_quit()
{
    std::cout << "The Quit Button was clicked\n";
    hide();
}

END_GVIDEOGTK_NAMESPACE
