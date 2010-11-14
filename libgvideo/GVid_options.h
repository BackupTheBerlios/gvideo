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
#  GVideo V4L2 video grabber and control panel                                  #
#                                                                               #
********************************************************************************/

#ifndef GVID_OPTIONS_H
#define GVID_OPTIONS_H

#include <unistd.h>
#include <getopt.h>
#include <string>
#include <iostream>
#include <vector>
#include "gvcommon.h"

START_LIBGVIDEO_NAMESPACE

struct GV_options
{
    std::string long_opt; //long option
    char short_opt;       //short option
    bool required_arg;    //required arg
    std::string arg_name; //argument name
    std::string help_msg;  //help message for option
};

class GVOptions
{
    struct option *long_options;
    std::string short_options;
    std::string app_name;
    std::vector<std::string> help_msg;
    bool verbose;
    bool help_called;
    
  public:
    //struct opt_data *opts;
    GVOptions(std::vector<GV_options> _options); //adds -h --help automatically
    ~GVOptions();
    int parse(int argc, char *argv[]);
    virtual void onHelp();
    virtual int onOption(int option, char* optarg);
    bool isHelp();

};

END_LIBGVIDEO_NAMESPACE

#endif
