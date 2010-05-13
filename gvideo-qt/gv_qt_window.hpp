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

#ifndef GVQTWINDOW_H
#define GVQTWINDOW_H

#include <QtGui>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSignalMapper>

#include <vector>
#include "libgvideo/GVid_v4l2.h"
#include "libgvaudio/GVAudio.h"
#include "gv_video_threads.h"

namespace gvideoqt {

class GVComboBox: public QComboBox
{
  protected:
    int index;
  public:
    GVComboBox(int i, QWidget * parent = 0);
    int get_index();
};

class GVCheckBox: public QCheckBox
{
  protected:
    int index;
  public:
    GVCheckBox(int i, QWidget * parent = 0);
    int get_index();
};

class GVSlider: public QSlider
{
  protected:
    int index;
  public:
    GVSlider(int i, QWidget * parent = 0);
    int get_index();
};

class QtWindow : public QWidget
{
    Q_OBJECT
    
  private slots:
    void on_button_quit();
    void on_button_pic();
    void on_button_vid();
    //v4l2 widget controls
    void on_check_button_clicked(QWidget* control);
    void on_combo_changed(QWidget* control);
    void on_slider_changed(QWidget* control);
    void on_video_format_combo_changed(int index);
    void on_resolution_combo_changed(int index);
    void on_fps_combo_changed(int index);
    void on_audio_dev_combo_changed(int index);
    void on_vcodec_combo_changed(int index);
    void on_acodec_combo_changed(int index);
    
  private:
    //Signal handlers:
    QSignalMapper *CombosignalMapper;
    QSignalMapper *ChecksignalMapper;
    QSignalMapper *SlidersignalMapper;
    //Child widgets:
    QTabWidget* gv_Notebook;

    QWidget* controlPage;
    QWidget* videoPage;
    QWidget* audioPage;
    QGridLayout* controlTable;
    QGridLayout* videoTable;
    QGridLayout* audioTable;
      
    QLabel  *fps_label;
    QComboBox* fps_combo;
    QLabel  *video_format_label;
    QComboBox* video_format_combo;
    QLabel  *resolution_label;
    QComboBox* resolution_combo;
    QLabel  *vcodec_label;
    QComboBox* vcodec_combo;
    
    QLabel *audio_dev_label;
    QComboBox* audio_dev_combo;
    QLabel  *acodec_label;
    QComboBox* acodec_combo;

    
    QVBoxLayout* gv_VBox;
    QHBoxLayout* gv_ButtonBox;
    QPushButton* gv_Button_pic;
    QPushButton* gv_Button_vid;
    QPushButton* gv_Button_Quit;
    
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
    QtWindow( libgvideo::GVDevice* _dev, 
        libgvaudio::GVAudio* _audio,
        libgvencoder::GVCodec*  _encoder = NULL,
        GVVideoThreads* _th_video = NULL,
        int _format = 0, 
        int _resolution = 0, 
        int _fps = 0);
        
    virtual ~QtWindow();
};

};
#endif
