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
#include <gtkmm/settings.h>

#include <iostream>
#include <sstream>

#include "gv_gtk_window.h"
#include "gv_gtk_pan-tilt_widget.h"
#include "gvcommon.h"
#include "libgvideo/GVid_v4l2.h"
#include "libgvencoder/GVCodec.h"

START_GVIDEOGTK_NAMESPACE

GtkWindow::GtkWindow(
        libgvideo::GVDevice* _dev, 
        libgvaudio::GVAudio* _audio,
        libgvencoder::GVCodec*  _encoder /*=NULL*/,
        GVVideoThreads* _th_video /*= NULL*/,
        int _format /*= 0*/, 
        int _resolution /*= 0*/, 
        int _fps /*= 0*/ )
{
    dev = _dev;
    audio = _audio;
    /*get a new encoder instance*/
    encoder= _encoder;
    th_video = _th_video;
    format = _format;
    resolution = _resolution;
    fps = _fps;

    vcodec_ind = 0;
    acodec_ind = 0;
    
    Gtk::Settings::get_default()->property_gtk_button_images() = true;
    
    gv_ImageLabel= Gtk::manage(new Gtk::Label("Image"));
    Gtk::Image* gv_image_pic = Gtk::manage(
        new Gtk::Image(std::string(PACKAGE_DATA_DIR) + std::string("/pixmaps/gvideo/image_controls.png")));
    Gtk::HBox* tab1 = Gtk::manage(new Gtk::HBox(false,2));
    tab1->pack_start(*gv_image_pic, false, false, 1);
    tab1->pack_start(*gv_ImageLabel, false, false, 1);
    tab1->show_all_children();
    
    gv_VideoLabel= Gtk::manage(new Gtk::Label("Video"));
    Gtk::Image* gv_video_pic = Gtk::manage(
        new Gtk::Image(std::string(PACKAGE_DATA_DIR) + std::string("/pixmaps/gvideo/video_controls.png")));
    Gtk::HBox* tab2 = Gtk::manage(new Gtk::HBox(false,2));
    tab2->pack_start(*gv_video_pic, false, false, 1);
    tab2->pack_start(*gv_VideoLabel, false, false, 1);
    tab2->show_all_children();
    
    gv_AudioLabel= Gtk::manage(new Gtk::Label("Audio"));
    Gtk::Image* gv_audio_pic = Gtk::manage(
        new Gtk::Image(std::string(PACKAGE_DATA_DIR) + std::string("/pixmaps/gvideo/audio_controls.png")));
    Gtk::HBox* tab3 = Gtk::manage(new Gtk::HBox(false,2));
    tab3->pack_start(*gv_audio_pic, false, false, 1);
    tab3->pack_start(*gv_AudioLabel, false, false, 1);
    tab3->show_all_children();
    
    gv_Button_pic= Gtk::manage(new Gtk::Button("cap picture"));
    Gtk::Image* gv_pic_img = Gtk::manage(
        new Gtk::Image(std::string(PACKAGE_DATA_DIR) + std::string("/pixmaps/gvideo/camera.png")));
    gv_Button_pic->set_image(*gv_pic_img);
    gv_Button_pic->set_image_position(Gtk::POS_TOP);
    
    gv_Button_vid= Gtk::manage(new Gtk::Button("cap video"));
    Gtk::Image* gv_vid_img = Gtk::manage(
        new Gtk::Image(std::string(PACKAGE_DATA_DIR) + std::string("/pixmaps/gvideo/movie.png")));
    gv_Button_vid->set_image(*gv_vid_img);
    gv_Button_vid->set_image_position(Gtk::POS_TOP);
    
    gv_Button_Quit= Gtk::manage(new Gtk::Button("Quit"));
    Gtk::Image* gv_quit_img = Gtk::manage(
        new Gtk::Image(std::string(PACKAGE_DATA_DIR) + std::string("/pixmaps/gvideo/close.png")));
    gv_Button_Quit->set_image(*gv_quit_img);
    gv_Button_Quit->set_image_position(Gtk::POS_TOP);
    
    set_title("GVideo - GTK GUI");
    set_border_width(10);
    set_default_size(480, 640);

    gv_VPaned = new Gtk::VPaned();
    gv_Notebook = Gtk::manage(new Gtk::Notebook());
    gv_ButtonBox = Gtk::manage(new Gtk::HButtonBox());
    gv_ButtonBox->set_layout(Gtk::BUTTONBOX_SPREAD);
    gv_ButtonBox->set_homogeneous(true);
    
    add(*gv_VPaned);

    
    //Add the Notebook, with the button box underneath:
    gv_Notebook->set_border_width(10);
    gv_VPaned->add1(*gv_Notebook);
    gv_VPaned->add2(*gv_ButtonBox);
    
    gv_VPaned->set_position(640-100);

    gv_ButtonBox->pack_start(*gv_Button_pic, true, true, 2 );
    gv_Button_pic->signal_clicked().connect(sigc::mem_fun(*this,
              &GtkWindow::on_button_pic) );
    
    gv_ButtonBox->pack_start(*gv_Button_vid, true, true, 2 );
    gv_Button_vid->signal_clicked().connect(sigc::mem_fun(*this,
              &GtkWindow::on_button_vid) );
    
    gv_ButtonBox->pack_start(*gv_Button_Quit, true, true, 2 );
    gv_Button_Quit->signal_clicked().connect(sigc::mem_fun(*this,
              &GtkWindow::on_button_quit) );
              
              
    controlTable = Gtk::manage(new Gtk::Table(3, dev->listControls.size(), false));
    videoTable = Gtk::manage(new Gtk::Table(4, 10, false));
    audioTable = Gtk::manage(new Gtk::Table(4, 10, false));
    
    //image controls
    int i = 0;
    int j = 0;
    
    for(i=0; i < dev->listControls.size(); i++)
    {
        Gtk::Label  *control_label= Gtk::manage(new Gtk::Label(dev->listControls[i].name));
        control_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
        controlTable->attach(*control_label, 0, 1, i, i+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
        int val = 0;
        
        switch(dev->listControls[i].type)
        {
            case V4L2_CTRL_TYPE_MENU:
                {
                    Gtk::ComboBoxText* control_widget = Gtk::manage(new Gtk::ComboBoxText());
                    for (j=0; j<dev->listControls[i].entries.size(); j++)
                    {
                        control_widget->append_text(dev->listControls[i].entries[j]);
                    }
                    if(dev->get_control_val (i, &val) != 0)
                        control_widget->set_active(dev->listControls[i].default_val);
                    else
                        control_widget->set_active(val);
                    controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
                    //Connect signal handler:
                    control_widget->signal_changed().connect(sigc::bind<Gtk::ComboBoxText*, int>(
                        sigc::mem_fun(*this, &GtkWindow::on_combo_changed), control_widget, i ));

                }
                break;
                
            case V4L2_CTRL_TYPE_BOOLEAN:
                {
                    Gtk::CheckButton *control_widget = Gtk::manage(new Gtk::CheckButton());
                    if(dev->get_control_val (i, &val) != 0)
                        control_widget->set_active(dev->listControls[i].default_val > 0);
                    else
                        control_widget->set_active(val > 0);
                    controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
                    //Connect signal handler:
                    control_widget->signal_clicked().connect(sigc::bind<Gtk::CheckButton*, int>(
                        sigc::mem_fun(*this, &GtkWindow::on_check_button_clicked), control_widget, i ));

                }
                break;
                
            case V4L2_CTRL_TYPE_INTEGER:
                {
                    if(dev->listControls[i].id == V4L2_CID_PAN_RELATIVE)
                    {
                        GtkPanTilt *control_widget = new GtkPanTilt(dev, i, true);
                        controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
                    }
                    else if (dev->listControls[i].id == V4L2_CID_TILT_RELATIVE)
                    {
                        GtkPanTilt *control_widget = new GtkPanTilt(dev, i, false);
                        controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
                    }
                    else
                    {
                        Gtk::HScale *control_widget = Gtk::manage(new Gtk::HScale(
                            dev->listControls[i].min, 
                            dev->listControls[i].max, 
                            dev->listControls[i].step ));
                        if(dev->get_control_val (i, &val) != 0)
                            control_widget->set_value(dev->listControls[i].default_val);
                        else
                            control_widget->set_value(val);
                        controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
                        //Connect signal handler:
                        control_widget->signal_value_changed().connect(sigc::bind<Gtk::HScale*, int>(
                            sigc::mem_fun(*this, &GtkWindow::on_hscale_value_changed), control_widget, i ));
                    }
                }
                break;
                
            case V4L2_CTRL_TYPE_BUTTON:
                {
                    Gtk::Button *control_widget = Gtk::manage(new Gtk::Button());
                    controlTable->attach(*control_widget, 1, 2, i, i+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
                    //Connect signal handler:
                    control_widget->signal_clicked().connect(sigc::bind<Gtk::Button*, int>(
                        sigc::mem_fun(*this, &GtkWindow::on_button_clicked), control_widget,i ));
                }
                break;
            
            default:
                std::cerr << "Control tpye: " << std::hex << std::showbase 
                    << dev->listControls[i].type << " not supported\n";
        }
    }
    
    //video stream definitions
    
    //stream format
    i=0;
    video_format_label= Gtk::manage(new Gtk::Label("Stream Format: "));
    video_format_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
    videoTable->attach(*video_format_label, 0, 1, i, i+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
    video_format_combo = Gtk::manage(new Gtk::ComboBoxText());
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
    resolution_label= Gtk::manage(new Gtk::Label("Resolution: "));
    resolution_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
    videoTable->attach(*resolution_label, 0, 1, i, i+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
    resolution_combo = Gtk::manage(new Gtk::ComboBoxText());
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
    fps_label= Gtk::manage(new Gtk::Label("Frame Rate: "));
    fps_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
    videoTable->attach(*fps_label, 0, 1, i, i+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
    fps_combo = Gtk::manage(new Gtk::ComboBoxText());
    
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
    //Video Codec
    i++;
    vcodec_label= Gtk::manage(new Gtk::Label("Video Codec: "));
    vcodec_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
    videoTable->attach(*vcodec_label, 0, 1, i, i+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
    vcodec_combo = Gtk::manage(new Gtk::ComboBoxText());
    
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
    audio_dev_label = Gtk::manage(new Gtk::Label("Devices: "));
    audio_dev_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
    audioTable->attach(*audio_dev_label, 0, 1, i, i+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
    audio_dev_combo = Gtk::manage(new Gtk::ComboBoxText());
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
    acodec_label= Gtk::manage(new Gtk::Label("Audio Codec: "));
    acodec_label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
    audioTable->attach(*acodec_label, 0, 1, i, i+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
    acodec_combo = Gtk::manage(new Gtk::ComboBoxText());
    
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
    
    Gtk::ScrolledWindow *scrolled1 = new Gtk::ScrolledWindow();
    Gtk::ScrolledWindow *scrolled2 = new Gtk::ScrolledWindow();
    Gtk::ScrolledWindow *scrolled3 = new Gtk::ScrolledWindow();
    
    scrolled1->add(*controlTable);
    scrolled2->add(*videoTable);
    scrolled3->add(*audioTable);
    //Add the Notebook pages:
    gv_Notebook->append_page(*scrolled1, *tab1);
    gv_Notebook->append_page(*scrolled2, *tab2);
    gv_Notebook->append_page(*scrolled3, *tab3);
    
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

void GtkWindow::on_button_clicked(Gtk::Button* Button, int cindex)
{
    if(!(dev->set_control_val (cindex, 1)))
    {
        //std::cout << dev->listControls[cindex].name << " = " << val << std::endl;
    }
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[cindex].name << std::endl;
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
