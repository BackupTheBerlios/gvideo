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
#  gvideo threads definitions                                                   #
#                                                                               #
********************************************************************************/

#ifndef GVTHREADS_H
#define GVTHREADS_H

#include <pthread.h>
#include "gvcommon.h"

START_LIBGVTHREADS_NAMESPACE

// this class is used to make generic calls to the run function
// in any object which have been inherited from object class
class Object
{
  public:
    virtual void run(){};
};

//now we will create the threading framework
class GVThread : public Object
{
    pthread_t thid;
    int ret;
  public:
    int get_err();
    // this member function will call the run function of the derived class
    void start();
    // this is to join the object threads
    void join();
};

END_LIBGVTHREADS_NAMESPACE

#endif
