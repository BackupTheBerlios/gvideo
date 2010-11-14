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
#  GVideo V4L2 video grabber and control panel - Qt PanTilt widget              #
#                                                                               #
********************************************************************************/

#ifndef GVQT_PAN_TILT_WIDGET_H
#define GVQT_PAN_TILT_WIDGET_H

#include <vector>
#include <QtGui>
#include <QTabWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSignalMapper>

#include "libgvideo/GVid_v4l2.h"

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

class GVButton: public QPushButton
{
  protected:
    int index;
  public:
    GVButton(int i, const QString name = "", QWidget * parent = 0);
    int get_index();
};

class GVPanTiltWidget: public QWidget
{
  Q_OBJECT
    
  private slots:
    void on_button_1();
    void on_button_2();
  protected:
    QHBoxLayout* gv_ButtonBox;
    QSpinBox* pt_step;
    QPushButton* gv_Button_1;
    QPushButton* gv_Button_2;
    int _index;
    bool _is_pan;
    libgvideo::GVDevice* dev;
  public:
    GVPanTiltWidget(libgvideo::GVDevice* device, int index, bool is_pan, QWidget * parent = 0);
    int get_index();
    bool IsPan();
};

class GVControlEnt: public QWidget
{
  protected:
    QHBoxLayout *hbox;
  public:
    GVControlEnt(QLabel *label, QWidget *parent = 0);
    void addWidget(QWidget *control_widget);    
};

class GVControlsWidget: public QWidget
{
  Q_OBJECT
  
  private slots:
    //v4l2 widget controls
    void on_check_button_clicked(QWidget* control);
    void on_combo_changed(QWidget* control);
    void on_slider_changed(QWidget* control);
    void on_button_clicked(QWidget* control);
  
  protected:
    libgvideo::GVDevice* dev;
    QVBoxLayout *controlTable;
    //Signal handlers:
    QSignalMapper *CombosignalMapper;
    QSignalMapper *ChecksignalMapper;
    QSignalMapper *SlidersignalMapper;
    QSignalMapper *ButtonsignalMapper;
    std::vector<QWidget *> widgets_list;
    void update_widgets();
    
  public:
    GVControlsWidget(libgvideo::GVDevice* device, QWidget *parent = 0);
};

}; //end namespace
#endif
