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

#include <QtGui>

#include <iostream>
#include <sstream>

#include "gv_qt_window.hpp"
#include "gv_qt_controls_widget.hpp"
#include "gvcommon.h"
#include "libgvideo/GVid_v4l2.h"
#include "libgvencoder/GVCodec.h"

START_GVIDEOQT_NAMESPACE

GVComboBox::GVComboBox( int i, QWidget * parent)
    : QComboBox  (parent)
{
    index = i;
}

int GVComboBox::get_index()
{
    return index;
}

GVCheckBox::GVCheckBox( int i, QWidget * parent)
    : QCheckBox  (parent)
{
    index = i;
}

int GVCheckBox::get_index()
{
    return index;
}

GVSlider::GVSlider( int i, QWidget * parent )
    : QSlider  (Qt::Horizontal, parent)
{
    index = i;
}

int GVSlider::get_index()
{
    return index;
}

GVButton::GVButton( int i, const QString name, QWidget * parent )
    : QPushButton  (name, parent)
{
    index = i;
}

int GVButton::get_index()
{
    return index;
}

QtWindow::QtWindow(
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

    gv_VBox = new QVBoxLayout(this);
    gv_VBox->setSpacing( 6 );
    gv_VBox->setMargin( 11 );

    gv_Notebook = new QTabWidget();
    
    //add the buttons
    gv_ButtonBox = new QHBoxLayout();
    gv_ButtonBox->setSpacing( 6 );
    gv_ButtonBox->setMargin( 0 );

    gv_Button_pic= new QPushButton("cap picture");
    gv_Button_pic->setIcon(QIcon(QString(PACKAGE_DATA_DIR) + QString("/pixmaps/gvideo/camera.png")));
    gv_Button_pic->setIconSize(QSize(64,64));
    
    gv_Button_vid= new QPushButton("cap video");
    gv_Button_vid->setIcon(QIcon(QString(PACKAGE_DATA_DIR) + QString("/pixmaps/gvideo/movie.png")));
    gv_Button_vid->setIconSize(QSize(64,64));
    
    gv_Button_Quit= new QPushButton("Quit");
    gv_Button_Quit->setIcon(QIcon(QString(PACKAGE_DATA_DIR) + QString("/pixmaps/gvideo/close.png")));
    gv_Button_Quit->setIconSize(QSize(64,64));
    
    gv_ButtonBox->addWidget(gv_Button_pic);
    gv_ButtonBox->addWidget(gv_Button_vid);
    gv_ButtonBox->addWidget(gv_Button_Quit);
    
    //button signals
    QObject::connect(gv_Button_pic, SIGNAL(clicked()), this, SLOT(on_button_pic()));
    QObject::connect(gv_Button_vid, SIGNAL(clicked()), this, SLOT(on_button_vid()));
    QObject::connect(gv_Button_Quit, SIGNAL(clicked()), this, SLOT(on_button_quit()));
    
    //Add the Notebook, with the button box underneath:
    gv_VBox->addWidget(gv_Notebook);
    QWidget* buttonBox = new QWidget();
    buttonBox->setStyleSheet("QPushButton{background-position: top}");
    buttonBox->setLayout(gv_ButtonBox);
    gv_VBox->addWidget(buttonBox);

    controlPage = new GVControlsWidget( dev, gv_Notebook);
    videoPage = new QWidget(gv_Notebook);
    audioPage = new QWidget(gv_Notebook);
    
    videoTable = new QGridLayout();
    audioTable = new QGridLayout();
    
    //image controls
    int i = 0;
    int j = 0;

    gv_Notebook->addTab(controlPage,QIcon(QString(PACKAGE_DATA_DIR) + QString("/pixmaps/gvideo/image_controls.png")),"Image Controls");
    gv_Notebook->setIconSize(QSize(32,32));
    //video stream definitions
    
    //stream format
    i=0;
    video_format_label= new QLabel("Stream Format: ");
    videoTable->addWidget(video_format_label, i, 0);
    video_format_combo = new QComboBox();
    j = 0;
    for (j=0; j< dev->listVidFormats.size(); j++)
    {
        video_format_combo->addItem(QString(dev->listVidFormats[j].fourcc.c_str()));
    }
    video_format_combo->setCurrentIndex(format);
    videoTable->addWidget(video_format_combo, i, 1);
    //Connect signal handler:
    QObject::connect(video_format_combo, SIGNAL(currentIndexChanged(int)), 
        this, SLOT(on_video_format_combo_changed(int)));
    
    //resolution
    i++;
    resolution_label= new QLabel("Resolution: ");
    videoTable->addWidget(resolution_label, i, 0);
    resolution_combo = new QComboBox();
    j = 0;
    for (j=0; j<dev->listVidFormats[format].listVidCap.size(); j++)
    {
        std::stringstream out;
        out << dev->listVidFormats[format].listVidCap[j].width << "x" 
            << dev->listVidFormats[format].listVidCap[j].height;
        resolution_combo->addItem(QString(out.str().c_str()));
    }
    resolution_combo->setCurrentIndex(resolution);
    videoTable->addWidget(resolution_combo, i, 1);
    //Connect signal handler:
    QObject::connect(resolution_combo, SIGNAL(currentIndexChanged(int)), 
        this, SLOT(on_resolution_combo_changed(int)));
    
    //fps
    i++;
    fps_label= new QLabel("Frame Rate: ");
    videoTable->addWidget(fps_label, i, 0);
    fps_combo = new QComboBox();
    
    j = 0;
    for (j=0; j<dev->listVidFormats[format].listVidCap[resolution].fps.size(); j++)
    {
        std::stringstream out;
        out << dev->listVidFormats[format].listVidCap[resolution].fps[j].num << "/" 
            << dev->listVidFormats[format].listVidCap[resolution].fps[j].denom;
        fps_combo->addItem(QString(out.str().c_str()));
    }
    fps_combo->setCurrentIndex(fps);
    videoTable->addWidget(fps_combo, i, 1);
    //Connect signal handler:
    QObject::connect(fps_combo, SIGNAL(currentIndexChanged(int)), 
        this, SLOT(on_fps_combo_changed(int)));

    //Encoder definitions
    //Video Codec
    i++;
    vcodec_label= new QLabel("Video Codec: ");
    videoTable->addWidget(vcodec_label, i, 0);
    vcodec_combo = new QComboBox();
    
    j = 0;
    for (j=0; j<encoder->vcodec_list.size(); j++)
    {
        vcodec_combo->addItem(QString(encoder->vcodec_list[j].description));
    }
    vcodec_combo->setCurrentIndex(vcodec_ind);
    videoTable->addWidget(vcodec_combo, i, 1);
    //Connect signal handler:
    QObject::connect(vcodec_combo, SIGNAL(currentIndexChanged(int)), 
        this, SLOT(on_vcodec_combo_changed(int)));
    
    videoPage->setLayout(videoTable);
    gv_Notebook->addTab(videoPage,QIcon(QString(PACKAGE_DATA_DIR) + QString("/pixmaps/gvideo/video_controls.png")),"Video Controls");
    
    //audio devices
    i = 0;
    audio_dev_label = new QLabel("Devices: ");
    audioTable->addWidget(audio_dev_label, i, 0);
    audio_dev_combo = new QComboBox();
    j = 0;
    for (j=0; j<audio->listAudioDev.size(); j++)
    {
        audio_dev_combo->addItem(QString(audio->listAudioDev[j].name.c_str()));
    }
    audio_dev_index = 0;
    audio_dev_combo->setCurrentIndex(audio_dev_index);
    audioTable->addWidget(audio_dev_combo, i, 1);
    //Connect signal handler:
    QObject::connect(audio_dev_combo, SIGNAL(currentIndexChanged(int)), 
        this, SLOT(on_audio_dev_combo_changed(int)));

    //Audio definitions
    i++;
    acodec_label= new QLabel("Audio Codec: ");
    audioTable->addWidget(acodec_label, i, 0);
    acodec_combo = new QComboBox();
    
    j = 0;
    for (j=0; j<encoder->acodec_list.size(); j++)
    {
        acodec_combo->addItem(QString(encoder->acodec_list[j].description));
    }
    acodec_combo->setCurrentIndex(acodec_ind);
    audioTable->addWidget(acodec_combo, i, 1);
    //Connect signal handler:
    QObject::connect(acodec_combo, SIGNAL(currentIndexChanged(int)), 
        this, SLOT(on_acodec_combo_changed(int)));
    
    audioPage->setLayout(audioTable);
    gv_Notebook->addTab(audioPage,QIcon(QString(PACKAGE_DATA_DIR) + QString("/pixmaps/gvideo/audio_controls.png")),"Audio Controls");
    
    //save current resolution
    width = dev->listVidFormats[format].listVidCap[resolution].width;
    height = dev->listVidFormats[format].listVidCap[resolution].height;
    //save current frame rate
    frate.num = dev->listVidFormats[format].listVidCap[resolution].fps[fps].num;
    frate.denom = dev->listVidFormats[format].listVidCap[resolution].fps[fps].denom;
    
    setLayout(gv_VBox);
    setWindowTitle("GVideo - QT GUI");
    setWindowIcon(QIcon(QString(PACKAGE_DATA_DIR) + QString("/pixmaps/gvideo/gvideo.png")));


}

QtWindow::~QtWindow()
{
    std::cout << "The Window was destroyed\n";
}

void QtWindow::on_video_format_combo_changed(int index)
{
    //get new format
    format = index;

    //block the resolution comboBox changed signal
    resolution_combo->blockSignals(true);
    
    //update resolution_combo
    resolution_combo->clear();
    
    int j = 0;
    for (j=0; j<dev->listVidFormats[format].listVidCap.size(); j++)
    {
        std::stringstream out;
        out << dev->listVidFormats[format].listVidCap[j].width << "x" 
            << dev->listVidFormats[format].listVidCap[j].height;
        resolution_combo->addItem(QString(out.str().c_str()));
        if((width == dev->listVidFormats[format].listVidCap[j].width) &&
            (height == dev->listVidFormats[format].listVidCap[j].height))
            resolution = j;
    }
    if(resolution >= dev->listVidFormats[format].listVidCap.size())
        resolution = dev->listVidFormats[format].listVidCap.size() -1;
    
    //unblock the fps combo changed signal
    resolution_combo->blockSignals(false);
    
    resolution_combo->setCurrentIndex(resolution);

    //apply changes
    std::cout << "format changed to index " << format << std::endl;
    
}

void QtWindow::on_resolution_combo_changed(int index)
{
    //get new resolution
    resolution = index;
    
    //save resolution
    width = dev->listVidFormats[format].listVidCap[resolution].width;
    height = dev->listVidFormats[format].listVidCap[resolution].height;
    
    //block fps comboBox changed signal
    fps_combo->blockSignals(true);
    
    //update fps_combo
    fps_combo->clear();
    int j = 0;
    for (j=0; j<dev->listVidFormats[format].listVidCap[resolution].fps.size(); j++)
    {
        std::stringstream out;
        out << dev->listVidFormats[format].listVidCap[resolution].fps[j].num << "/" 
            << dev->listVidFormats[format].listVidCap[resolution].fps[j].denom;
        fps_combo->addItem(QString(out.str().c_str()));
        if((frate.num == dev->listVidFormats[format].listVidCap[resolution].fps[j].num ) &&
            (frate.denom == dev->listVidFormats[format].listVidCap[resolution].fps[j].denom))
            fps = j;
    }
    if(fps >= dev->listVidFormats[format].listVidCap[resolution].fps.size())
        fps = dev->listVidFormats[format].listVidCap[resolution].fps.size() -1;
    
    //unblock fps comboBox changed signal
    fps_combo->blockSignals(false);
    
    fps_combo->setCurrentIndex(fps);

    //apply changes
    std::cout << "resolution changed to index " << resolution << std::endl;
    th_video->set_format(dev->listVidFormats[format].fourcc, width, height);
}

void QtWindow::on_fps_combo_changed(int index)
{
    //get new fps
    fps = index;
    
    //save frame rate
    frate.num = dev->listVidFormats[format].listVidCap[resolution].fps[fps].num;
    frate.denom = dev->listVidFormats[format].listVidCap[resolution].fps[fps].denom;
    
    //apply changes
    std::cout << "fps changed to index " << fps << std::endl;
    dev->set_fps(&frate);
}

void QtWindow::on_vcodec_combo_changed(int index)
{
    vcodec_ind = index;
}

void QtWindow::on_acodec_combo_changed(int index)
{
    acodec_ind = index;
}

void QtWindow::on_audio_dev_combo_changed(int index)
{
    audio_dev_index = index;
}

void QtWindow::on_button_pic()
{
    th_video->cap_image("file_gvideo.jpg");
}

void QtWindow::on_button_vid()
{
    if(th_video->is_capturing_video())
    {
        gv_Button_vid->setText("cap video");
        th_video->stop_video_capture();
    }
    else
    {
        gv_Button_vid->setText("stop video");
        th_video->start_video_capture(audio, audio_dev_index, vcodec_ind, acodec_ind);
    }
}

void QtWindow::on_button_quit()
{
    //std::cout << "The Quit Button was clicked\n";
    close();
}

END_GVIDEOQT_NAMESPACE
