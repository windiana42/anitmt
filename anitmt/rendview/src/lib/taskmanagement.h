/* simple include wrapper for various files: */

#ifndef _LIB_TASKMANAGEMENT_H_
#define _LIB_TASKMANAGEMENT_H_ 1

// Our misc stuff and config and HLIB config: 
#include "prototypes.hpp"

// Basic hlib stuff: 
#include <hlib/refstring.h>

// Process management & related: 
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
class TaskDriverInterface;
class TaskDriverFactory;

class TaskFile;
class CompleteTask;

class TaskDriver;
class LDRClient;

enum TaskDriverType
{
	DTNone=-1,
	DTRender,
	DTFilter,
	_DTLast
};

#endif  /* _LIB_TASKMANAGEMENT_H_ */

