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

#include "gv_qt_pan-tilt_widget.hpp"
#include "gvcommon.h"
#include <iostream>
#include <vector>

START_GVIDEOQT_NAMESPACE

GVPanTiltWidget::GVPanTiltWidget( libgvideo::GVDevice* device, int index, bool is_pan, QWidget * parent)
    : QWidget  (parent)
{
    _index = index;
    _is_pan = is_pan;
    dev = device;
    //add the buttons
    gv_ButtonBox = new QHBoxLayout();
    gv_ButtonBox->setSpacing( 1 );
    gv_ButtonBox->setMargin( 0 );

    pt_step = new QSpinBox();
    pt_step->setMinimum(-256);
    pt_step->setMaximum(256);
    pt_step->setSingleStep(64);
    gv_Button_1= new QPushButton();
    gv_Button_2= new QPushButton();
    if(_is_pan)
    {
        pt_step->setPrefix("step  ");
        pt_step->setValue(-128);
        gv_Button_1->setText("Left");
        gv_Button_2->setText("Right");
    }
    else //is Tilt
    {
        pt_step->setPrefix("step  ");
        pt_step->setValue(-128);
        gv_Button_1->setText("Up");
        gv_Button_2->setText("Down");
    }
    gv_ButtonBox->addWidget(gv_Button_1);
    gv_ButtonBox->addWidget(gv_Button_2);
    gv_ButtonBox->addWidget(pt_step);
    
    this->setLayout(gv_ButtonBox);
    
    //connect signals
    QObject::connect(gv_Button_1, SIGNAL(clicked()), this, SLOT(on_button_1()));
    QObject::connect(gv_Button_2, SIGNAL(clicked()), this, SLOT(on_button_2()));
}

void GVPanTiltWidget::on_button_1()
{
    int val = pt_step->value();
    if(dev->set_control_val (_index, val))
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[_index].name.c_str()
            << " to " << val << std::endl;
    }
}

void GVPanTiltWidget::on_button_2()
{
    int val = -(pt_step->value());
    if(dev->set_control_val (_index, val))
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[_index].name.c_str()
            << " to " << val << std::endl;
    }
}

END_GVIDEOQT_NAMESPACE
