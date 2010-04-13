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
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/table.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scale.h>
#include <gtkmm/notebook.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>

#include <vector>
#include "libgvideo/GVid_v4l2.h"
#include "libgvaudio/GVAudio.h"

START_GVIDEOGTK_NAMESPACE

class GtkWindow : public Gtk::Window
{
protected:
  //Signal handlers:
  void on_button_quit();
  void on_button_pic();
  void on_button_vid();
  void on_check_button_clicked(Gtk::CheckButton* checkButton, int cindex);
  void on_combo_changed(Gtk::ComboBoxText* comboBox, int cindex);
  void on_hscale_value_changed(Gtk::HScale* hScale, int cindex);
  void on_video_format_combo_changed();
  void on_resolution_combo_changed();
  void on_fps_combo_changed();
  void on_audio_dev_combo_changed();
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
  
  Gtk::Label  *audio_dev_label;
  Gtk::ComboBoxText* audio_dev_combo;
  
  Gtk::VBox* gv_VBox;
  Gtk::HButtonBox* gv_ButtonBox;
  Gtk::Button* gv_Button_pic;
  Gtk::Button* gv_Button_vid;
  Gtk::Button* gv_Button_Quit;
  
public:
  
  GtkWindow( libgvideo::GVDevice *dev, 
        libgvaudio::GVAudio *audio, 
        int format = 0, 
        int resolution = 0, 
        int fps = 0);
        
  virtual ~GtkWindow();
};

END_GVIDEOGTK_NAMESPACE
#endif
