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

#include "libgvthreads/GVThreads.h"

START_LIBGVTHREADS_NAMESPACE

// now we will create an executer non member function
//which can call the run function of any Object class instance
void* executer(void* param)
{
    Object *obj=(Object*) param;
    obj->run();
    // this will definitely call the run function associated with the object obj
    // from this general function--the rest i guess u can figure out!
};

int GVThread::get_err()
{
    return ret;
};

// this member function will call the run function of the derived class
void GVThread::start()
{
    ret=pthread_create(&thid, NULL, executer, (void*)this);
};
// this is to join the object threads
void GVThread::join()
{
    pthread_join(thid, NULL);
};


END_LIBGVTHREADS_NAMESPACE

