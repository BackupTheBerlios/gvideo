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

#ifndef GVGTKWINDOW_H
#define GVGTKWINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/table.h>
#include <gtkmm/box.h>
#include <gtkmm/paned.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scale.h>
#include <gtkmm/notebook.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>

#include <vector>
#include "libgvideo/GVid_v4l2.h"
#include "libgvaudio/GVAudio.h"
#include "gv_video_threads.h"
#include "gv_gtk_controls_widget.h"

START_GVIDEOGTK_NAMESPACE

class GtkWindow : public Gtk::Window
{
  protected:
    //Signal handlers:
    void on_button_quit();
    void on_button_pic();
    void on_button_vid();
    // video format controls
    void on_video_format_combo_changed();
    void on_resolution_combo_changed();
    sigc::connection signalResolutionCombo;
    void on_fps_combo_changed();
    sigc::connection signalFpsCombo;
    // audio controls
    void on_audio_dev_combo_changed();
    // codec controls
    void on_vcodec_combo_changed();
    void on_acodec_combo_changed();
    //void on_notebook_switch_page(GtkNotebookPage* page, guint page_num);
      
    //Child widgets:
    Gtk::Notebook* gv_Notebook;
    Gtk::Label* gv_ImageLabel;
    Gtk::Label* gv_VideoLabel;
    Gtk::Label* gv_AudioLabel;
    Gtk::Table* controlTable;
    Gtk::Table* videoTable;
    Gtk::Table* audioTable;
      
    Gtk::Label  *fps_label;
    Gtk::ComboBoxText* fps_combo;
    Gtk::Label  *video_format_label;
    Gtk::ComboBoxText* video_format_combo;
    Gtk::Label  *resolution_label;
    Gtk::ComboBoxText* resolution_combo;
    Gtk::Label  *vcodec_label;
    Gtk::ComboBoxText* vcodec_combo;
    
    Gtk::Label  *audio_dev_label;
    Gtk::ComboBoxText* audio_dev_combo;
    Gtk::Label  *acodec_label;
    Gtk::ComboBoxText* acodec_combo;

    
    Gtk::VPaned* gv_VPaned;
    Gtk::HButtonBox* gv_ButtonBox;
    Gtk::Button* gv_Button_pic;
    Gtk::Button* gv_Button_vid;
    Gtk::Button* gv_Button_Quit;
    
    //video defs
    int width;
    int height;
    libgvideo::GVFps frate;
    int format, resolution, fps;
    libgvideo::GVDevice* dev;
    libgvencoder::GVCodec* encoder;
    int audio_dev_index;
    unsigned vcodec_ind;
    unsigned acodec_ind;
    libgvaudio::GVAudio* audio;
    GVVideoThreads* th_video;
    
  public:
  
    GtkWindow( libgvideo::GVDevice* _dev, 
        libgvaudio::GVAudio* _audio,
        libgvencoder::GVCodec*  _encoder = NULL,
        GVVideoThreads* _th_video = NULL,
        int _format = 0, 
        int _resolution = 0, 
        int _fps = 0);
        
    virtual ~GtkWindow();
};

END_GVIDEOGTK_NAMESPACE
#endif
