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

#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <libgvideo/GVid_options.h>

using namespace std;

GVOptions::GVOptions(vector<GV_options> _options)
{
    //opts = new opt_data;
    verbose = true;
    help_called = false;
    int i = 0, j = 0;
    long_options = new option [ _options.size() + 2];
    //adds -h
    short_options ="h";
    // add --help option
    long_options[i].name = "help";
    long_options[i].has_arg = no_argument;
    long_options[i].flag = NULL;
    long_options[i].val = 'h';
    //add help line for --help
    help_msg.push_back("-h, --help             : prints help message");
    // add user options
    for (i = 0; i < _options.size(); i++)
    {
        string tmpstr;
        // short options
        short_options.push_back(_options[i].short_opt);
        // long options
        long_options[i+1].name = _options[i].long_opt.c_str();
        long_options[i+1].flag = NULL;
        long_options[i+1].val = (int) _options[i].short_opt;
        //help message
        tmpstr = "-";
        tmpstr.push_back(_options[i].short_opt);
        tmpstr += ", --";
        tmpstr += _options[i].long_opt;
        // argument
        if(_options[i].required_arg)
        {
            short_options.push_back(':');
            long_options[i+1].has_arg = required_argument;
            tmpstr += "=";
            tmpstr += _options[i].arg_name;
        }
        else
        {
            long_options[i+1].has_arg = no_argument;
        }
        //add tabulation to help msg
        int spc = 0;
        if ((spc = 23 - tmpstr.size()) > 0)
            for (j=0; j < spc; j++)
                tmpstr += " ";
        tmpstr += ": ";
        tmpstr += _options[i].help_msg;
        
        help_msg.push_back(tmpstr);
    }
    i = _options.size(); //zero last entry
    long_options[i+1].name = NULL;
    long_options[i+1].has_arg = 0;
    long_options[i+1].flag = NULL;
    long_options[i+1].val = 0;
}

GVOptions::~GVOptions()
{
    //delete opts; 
}

void GVOptions::onHelp()
{
    int i = 0;
    cout << "USAGE: \n";
    cout << app_name << " options\n";
    cout << "OPTIONS:\n";
    for (i = 0; i < help_msg.size(); i++)
        cout << help_msg[i] << endl;
        
    help_called = true;
}

bool GVOptions::isHelp()
{
    return help_called;
}

int GVOptions::onOption(int option, char* optarg)
{
    //true virtual - do nothing
}

int GVOptions::parse(int argc, char *argv[])
{
    int next_option = 0;
    app_name = string(argv[0]);
    
    do 
    {
        next_option = getopt_long (argc, argv, short_options.c_str(),
                               long_options, NULL);
        switch (next_option)
        {
            case 'h':   /* -h or --help */
                onHelp();
                break;
            case -1:
                break;
            default:
                onOption(next_option, optarg);
                break;
        }
    }
    while (next_option != -1);
        
    /* Done with options.  OPTIND points to first non-option argument.
    For demonstration purposes, print them if the verbose option was
    specified.  */
    if (verbose) 
    {
        int i;
        for (i = optind; i < argc; ++i) 
            cout << "Argument: " << argv[i] << endl;
    }
    
    return(0);
    
}
