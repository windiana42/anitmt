/* simple include wrapper for various files: */

#ifndef _LIB_TASKMANAGEMENT_H_
#define _LIB_TASKMANAGEMENT_H_ 1

// Our misc stuff: 
#include "prototypes.hpp"

// Basic hlib stuff: 
#include <hlib/prototypes.h>
#include <hlib/refstring.h>

// Process management & related: 
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <hlib/htime.h>
#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>
#include <hlib/procmanager.h>
#include <hlib/procbase.h>
#include <hlib/secthdl.h>

// Parameter management: 
#include <hlib/parbase.h>

class TaskManager;
class TaskSource;

class TaskFile;
class CompleteTask;

#endif  /* _LIB_TASKMANAGEMENT_H_ */

