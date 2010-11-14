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

#include "gv_qt_controls_widget.hpp"
#include "gvcommon.h"
#include <iostream>
#include <vector>

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
    dev->listControls[_index].value = val;
    if(dev->set_control_val (_index))
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[_index].control.name
            << " to " << val << std::endl;
    }
}

void GVPanTiltWidget::on_button_2()
{
    int val = -(pt_step->value());
    dev->listControls[_index].value = val;
    if(dev->set_control_val (_index))
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[_index].control.name
            << " to " << val << std::endl;
    }
}

void GVControlsWidget::update_widgets()
{
    int i = 0;
    for(i=0; i<dev->listControls.size(); i++)
    {
        if(dev->listControls[i].control.flags & V4L2_CTRL_FLAG_GRABBED)
        {
            widgets_list[i]->setEnabled(false);
        }
        else
            widgets_list[i]->setEnabled(true);
    }
}

GVControlEnt::GVControlEnt(QLabel *label, QWidget *parent)
    : QWidget (parent)
{
    hbox = new QHBoxLayout();
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );
    
    hbox->addWidget(label,Qt::AlignLeft);
    
    this->setLayout(hbox);
}

void GVControlEnt::addWidget(QWidget *control_widget)
{
    hbox->addWidget(control_widget, Qt::AlignRight);
}

GVControlsWidget::GVControlsWidget( libgvideo::GVDevice* device, QWidget * parent)
    : QWidget  (parent)
{
    dev = device;
    int i = 0;
    int j = 0;
    CombosignalMapper = NULL;
    ChecksignalMapper = NULL;
    SlidersignalMapper = NULL;
    ButtonsignalMapper = NULL;
    
    controlTable = new QVBoxLayout();
    controlTable->setSpacing( 6 );
    controlTable->setMargin( 0 );
    
    for(i=0; i < dev->listControls.size(); i++)
    {
        QLabel  *control_label= new QLabel(QString((char *) dev->listControls[i].control.name));
        GVControlEnt *ctrl_ent = new GVControlEnt(control_label);
        int val = 0;
        
        switch(dev->listControls[i].control.type)
        {
            case V4L2_CTRL_TYPE_MENU:
                {
                    GVComboBox* control_widget = new GVComboBox(i);
                    ctrl_ent->addWidget(control_widget);
                    widgets_list.push_back(ctrl_ent);
                    
                    for (j=0; j<dev->listControls[i].entries.size(); j++)
                    {
                        control_widget->addItem(QString(dev->listControls[i].entries[j].c_str()));
                    }
                    if(dev->get_control_val (i) != 0)
                        control_widget->setCurrentIndex(dev->listControls[i].control.default_value);
                    else
                        control_widget->setCurrentIndex(val);
                    
                    controlTable->addWidget(ctrl_ent);
                    //Connect signal handler:
                    
                    if(CombosignalMapper == NULL) CombosignalMapper = new QSignalMapper(this);
                    CombosignalMapper->setMapping(control_widget, control_widget);

                    QObject::connect(control_widget, SIGNAL(currentIndexChanged(int)), 
                        CombosignalMapper, SLOT(map()));
                    QObject::connect(CombosignalMapper, SIGNAL(mapped(QWidget*)),
                        this, SLOT(on_combo_changed(QWidget*)));

                }
                break;
                
            case V4L2_CTRL_TYPE_BOOLEAN:
                {
                    GVCheckBox *control_widget = new GVCheckBox(i);
                    ctrl_ent->addWidget(control_widget);
                    widgets_list.push_back(ctrl_ent);
                    
                    if(dev->get_control_val (i) != 0)
                        control_widget->setChecked((dev->listControls[i].control.default_value > 0));
                    else
                        control_widget->setChecked(dev->listControls[i].value > 0);

                    controlTable->addWidget(ctrl_ent);
                    //Connect signal handler:
                    if(ChecksignalMapper == NULL) ChecksignalMapper = new QSignalMapper(this);
                    ChecksignalMapper->setMapping(control_widget, control_widget);

                    QObject::connect(control_widget, SIGNAL(stateChanged (int)), 
                        ChecksignalMapper, SLOT(map()));
                    QObject::connect(ChecksignalMapper, SIGNAL(mapped(QWidget*)),
                        this, SLOT(on_check_button_clicked(QWidget*)));
                }
                break;
                
            case V4L2_CTRL_TYPE_INTEGER:
                {
                    if(dev->listControls[i].control.id == V4L2_CID_PAN_RELATIVE)
                    {
                        GVPanTiltWidget *control_widget = new GVPanTiltWidget( dev, i, true);
                        ctrl_ent->addWidget(control_widget);
                        widgets_list.push_back(ctrl_ent);
                        controlTable->addWidget(ctrl_ent);
                    }
                    else if (dev->listControls[i].control.id == V4L2_CID_TILT_RELATIVE)
                    {
                        GVPanTiltWidget *control_widget = new GVPanTiltWidget( dev, i, false);
                        ctrl_ent->addWidget(control_widget);
                        widgets_list.push_back(ctrl_ent);
                        controlTable->addWidget(ctrl_ent);
                    }
                    else
                    {
                        GVSlider *control_widget = new GVSlider( i );
                        
                        control_widget->setMinimum(dev->listControls[i].control.minimum);
                        control_widget->setMaximum(dev->listControls[i].control.maximum);
                        control_widget->setPageStep(dev->listControls[i].control.step);
                        if(dev->get_control_val (i) != 0)
                            control_widget->setValue(dev->listControls[i].control.default_value);
                        else
                            control_widget->setValue(dev->listControls[i].value);
                        
                        ctrl_ent->addWidget(control_widget);
                        widgets_list.push_back(ctrl_ent);
                        
                        controlTable->addWidget(ctrl_ent);
                        //Connect signal handler:
                        if(SlidersignalMapper == NULL) SlidersignalMapper = new QSignalMapper(this);
                        SlidersignalMapper->setMapping(control_widget, control_widget);

                        QObject::connect(control_widget, SIGNAL(valueChanged (int) ), 
                            SlidersignalMapper, SLOT(map()));
                        QObject::connect(SlidersignalMapper, SIGNAL(mapped(QWidget*)),
                            this, SLOT(on_slider_changed(QWidget*)));
                    }
                }
                break;
                
            case V4L2_CTRL_TYPE_BUTTON:
                {
                    GVButton *control_widget = new GVButton(i);
                    ctrl_ent->addWidget(control_widget);
                    widgets_list.push_back(ctrl_ent);
                    
                    controlTable->addWidget(ctrl_ent);
                    //Connect signal handler:
                    if(ButtonsignalMapper == NULL) ButtonsignalMapper = new QSignalMapper(this);
                        ButtonsignalMapper->setMapping(control_widget, control_widget);

                    QObject::connect(control_widget, SIGNAL(clicked () ), 
                        ButtonsignalMapper, SLOT(map()));
                    QObject::connect(ButtonsignalMapper, SIGNAL(mapped(QWidget*)),
                        this, SLOT(on_button_clicked(QWidget*)));
                }
                break;
            
            default:
                std::cerr << "Control tpye: " << std::hex << std::showbase 
                    << dev->listControls[i].control.type << " not supported\n";
        }
    }
    this->setLayout(controlTable);
}

void GVControlsWidget::on_check_button_clicked(QWidget* control)
{
    GVCheckBox* checkbox = (GVCheckBox*) control;
    int val = checkbox->isChecked() ? 1 : 0;
    int index = checkbox->get_index();
    dev->listControls[index].value = val;
    if(!(dev->set_control_val (index)))
    {
        std::cout << dev->listControls[index].control.name << " = " << 
            dev->listControls[index].value << std::endl;
    }
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[index].control.name 
            << " = " << val << std::endl;
        checkbox->blockSignals(true);
        if(dev->listControls[index].value) checkbox->setChecked(true);
        else checkbox->setChecked(false);
        checkbox->blockSignals(false);
    }
    
    update_widgets();
}

void GVControlsWidget::on_combo_changed(QWidget* control)
{
    GVComboBox* combobox = (GVComboBox*) control;
    int val = combobox->currentIndex();
    int index = combobox->get_index();
    dev->listControls[index].value = val;
    if(!(dev->set_control_val (index)))
    {
        std::cout << dev->listControls[index].control.name 
            << " = " << dev->listControls[index].value << std::endl;
    }
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[index].control.name 
            << " = " << val << std::endl;
        combobox->blockSignals(true);
        combobox->setCurrentIndex(dev->listControls[index].value);
        combobox->blockSignals(false);
    }
    
    update_widgets();
}

void GVControlsWidget::on_slider_changed(QWidget* control)
{
    GVSlider* slider = (GVSlider*) control;
    int val = slider->value();
    int index = slider->get_index();
    dev->listControls[index].value = val;
    if(!(dev->set_control_val (index)))
    {
        //std::cout << dev->listControls[index].name << " = " << val << std::endl;
    }
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[index].control.name 
            << " = " << val << std::endl;
        slider->blockSignals(true);
        slider->setValue(dev->listControls[index].value);
        slider->blockSignals(false);
    }
}

void GVControlsWidget::on_button_clicked(QWidget* control)
{
    GVButton* button = (GVButton*) control;
    
    int index = button->get_index();
    
    dev->listControls[index].value = 1;
    if(!(dev->set_control_val (index)))
    {
        //std::cout << dev->listControls[index].name << " = " << val << std::endl;
    }
    else
    {
        std::cerr << "ERROR:couldn't set " << dev->listControls[index].control.name << std::endl;
    }
}

END_GVIDEOQT_NAMESPACE
