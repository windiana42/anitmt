/*
 * core/a_animation.cc
 * 
 * Main animation core. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "animation.h"
#include <ani-parser/treereg.h>

#include "execthread.h"
#include <calccore/ccif.h>
#include <calccore/scc/ccif_simple.h>

#include "aniinstance.h"

// For the POV attach/detach: 
#include <ani-parser/treereg.h>
#include <ani-parser/exprtf.h>


using namespace ANI;


int AniAnimation::ExecAnimation(const RefString &ani_name)
{
	AniUserFunction *ufunc=FindAnimationFunction(ani_name);
	if(!ufunc)
	{  return(-2);  }
	
	// Execute the animation block: 
	// This is special: the thread has an ID=0 and 
	// schedule_status is SSDisabled as set by the constructor. 
	ExecThread *main_ethread=new ExecThread(/*context=*/NULL,/*thread_id=*/0);
	
	do {
		// Okay, initialize non-auto static variables: 
		fprintf(stdout,"VM: Initializing static variables\n");
		int rv=this->InitStaticVariables(main_ethread,+1);
		fprintf(stdout,"VM:   Initializing static variables: %d errors\n",rv);
		
		if(rv)  break;
		
		TNOperatorFunction::EvalFunc *main_evalfunc=ufunc->GetAniEvalFunc();
		assert(main_evalfunc);
		
		fprintf(stdout,"VM: Running animation \"%s\" @%s\n",
			ufunc->GetName().str(),
			ufunc->NameLocation().PosRangeString().str());
		
		ExprValue dummy;
		do {
			// Note: As for the animation point of view the next interesting 
			//       function to be called is the setting init function: 
			//       AniSetting::EvalFunc_FCallSetting::Eval() in a_setting.cc. 
			rv=main_evalfunc->Eval(dummy,/*tn=*/NULL,main_ethread);
			//assert(rv==0);  // rv=1 only in multithreading environment!
			// ...yes but the context switch hack bombards us for test reasons. 
		} while(rv);
		
		fprintf(stdout,"VM: Animation \"%s\" now over\n",
			ufunc->GetName().str());
		
		//fprintf(stderr,"\n");
		//fprintf(stderr,"%s exited (status=%s)\n",
		//	ufunc->GetName().str(),
		//	dummy.ToString().str());
		
		DELETE(main_evalfunc);
		
		// Finally, clean up static data: 
		// (Important -- static vals can reference objects, etc). 
		fprintf(stdout,"VM: Cleaning up static variables\n");
		rv=this->InitStaticVariables(main_ethread,-1);
		fprintf(stdout,"VM:   Cleaning up static variables: %d errors\n",rv);
	} while(0);
	
	DELETE(main_ethread);
	
	// IMPORTANT: Cleanup all contexts: 
	_CleanupAllContexts();
	
	return(0);
}


void AniAnimation::_CleanupAllContexts()
{
	// No context may be active here. 
	//assert(!active_context);
	
	// Delete all contexts: 
	while(!context_list.is_empty())
	{  delete context_list.popfirst();  }
}


Animation_Context *AniAnimation::GetContextByName(const String &name,
	AniSetting *setting)
{
	// Linear lookup: 
	// This is not speed critical because called seldom. 
	for(Animation_Context *c=context_list.first(); c; c=c->next)
	{
		if(c->setting!=setting)  continue;
		if(c->name!=name)  continue;
		return(c);
	}
	// Allocate a new one: 
	// We can use ID 1 for the eval thread since it is the first 
	// thread for this ani context. 
	Animation_Context *c=new Animation_Context(name,setting,
		/*eval_thread_id=*/1);
	context_list.append(c);
	return(c);
}


int AniAnimation::_GetUniqueThreadID(Animation_Context *ctx)
{
	// NOTE: thread_id's are only unique per context, 
	//       NOT accross all contexts. 
	
	// The first thread gets the ID 1 :)
	if(ctx->thread_list.is_empty())
	{
		thread_id_counter=1;
		return(1);
	}
	
	int old=thread_id_counter;
	do {
		++thread_id_counter;
		if(thread_id_counter<=1)
		{  thread_id_counter=2;  }
		// FIXME: Linear lookup could be speeded up. 
		for(ExecThread *i=ctx->thread_list.first(); i; i=i->next)
		{  if(i->thread_id==thread_id_counter)  goto retry;  }
		return(thread_id_counter);
		retry:;
	} while(old!=thread_id_counter);
	
	// This will never happen on a >=32bit box. 
	fprintf(stderr,"VM: OOPS: thread_id's exhausted.\n");
	abort();
}


int AniAnimation::_DoRunThread(Animation_Context *ctx,ExecThread *run_thread)
{
	ExprValue dummy;
	fprintf(stderr,"VM: Running [%d]\n",run_thread->thread_id);
	int eval_rv=ctx->setting->GetSettingInitEvalFunc()->Eval(
		/*retval=*/dummy,
		/*TNOperatorFunction *tn=*/NULL/*special*/,
		/*info=*/run_thread);
	// This must be done here to allow to set Waiting state below. 
	run_thread->state=ExecThread::Inactive;
	// Check for context switch: 
	if(eval_rv)
	{
		switch(run_thread->schedule_status)
		{
			case ExecThread::SSRequest:
			{
				// Process clone request. 
				ExecThread *new_thread=run_thread->Clone(
					_GetUniqueThreadID(ctx));
				// New thread is in SSDoneClone status (done by Clone())
				//new_thread->schedule_status=ExecThread::SSDoneClone;
				// Current thread has the new thread_id in schedule_status. 
				// (Done by Clone())
				//run_thread->schedule_status=new_thread->thread_id;
				// Queue new thread after current one: 
				ctx->thread_list.queueafter(new_thread,run_thread);
			} break;
			case ExecThread::SSReady:
				//run_thread->schedule_status=<unmodified>
				// nothing to do
				break;
			case ExecThread::SSYield:
				// Simply re-schedule
				run_thread->schedule_status=ExecThread::SSReady;
				break;
			case ExecThread::SSWait:
				// Enter waiting state: 
				run_thread->state=ExecThread::Waiting;
				run_thread->schedule_status=ExecThread::SSReady;
				break;
			case ExecThread::SSDelay:
				// Enter delayed state: 
				run_thread->state=ExecThread::Delayed;
				run_thread->schedule_status=ExecThread::SSReady;
				break;
			case ExecThread::SSKillMe:
				// Kill thread. 
				run_thread->schedule_status=ExecThread::SSReady;
				// Simply done by returning 0: 
				return(0);
				break;
			default:
				// All other states are illegal. 
				// No thread may have SSDisabled (cloning is enabled here), 
				// SSDoneClone and /*anything>0*/ must be processed by 
				// the clone() call which re-sets the status to SSReady. 
				assert(0);
		}
	}
	
	return(eval_rv);
}


void AniAnimation::_DoCheckAllWaitConditions(Animation_Context *ctx)
{
	ExprValue dummy;
	
	// Check the wait conditions if we need: 
	// Of course, this is O(n); we could also introduce a separate 
	// list for those ExecThreads which need re-eval...
	ExecThreadInfo::currently_evaluating_any_wcond=1;
	while(ExecThread::wait_cond_re_eval_cs)
	{
		ExecThread::wait_cond_re_eval_cs=0;
		for(ExecThread *i=ctx->thread_list.first(); i; i=i->next)
		{
			// Only those which have a wait condition which has to 
			// be evaluated. 
			if(!i->wait_cond.must_eval)  continue;
			
			i->wait_cond.must_eval=0;
			assert(i->state==ExecThread::Waiting);
			if(i->wait_cond.GetWCState()!=
				ExecThread::WaitCondition::WC_Waiting)
			{  assert(0);  }
			
			// Okay, actually do the re-eval of the condition: 
			fprintf(stderr,"VM: Wait cond test [%d]\n",i->thread_id);
			//i->state=ExecThread::Running; <-- stays ExecThread::Waiting. 
			i->currently_evaluating_this_wcond=1;
			int erv=ctx->setting->GetSettingInitEvalFunc()->Eval(
				/*retval=*/dummy,
				/*TNOperatorFunction *tn=*/NULL/*special*/,
				/*info=*/i);
			i->currently_evaluating_this_wcond=0;
			assert(i->state==ExecThread::Waiting);
			// The wait eval func must return 1 in _any_ case when 
			// in waiting state (even when it is now over). 
			assert(erv);
			switch(i->schedule_status)
			{
				case ExecThread::SSWait:
					if(i->wait_cond.GetWCState()==
						ExecThread::WaitCondition::WC_NotInUse)
					{
						// Waiting is over now. 
						i->state=ExecThread::Inactive;
						assert(!i->wait_cond.must_eval);
					}
					else if(i->wait_cond.GetWCState()==
						ExecThread::WaitCondition::WC_Waiting)
					{
						// Continue waiting. 
						i->state=ExecThread::Waiting;
					}
					else assert(0);
					i->schedule_status=ExecThread::SSReady;
					break;
				case ExecThread::SSYield:
					Warning(TNLocation(),
						"using yield() from within waiting condition\n");
					// Continue waiting but do not set must_eval flag. 
					i->state=ExecThread::Waiting;
					i->schedule_status=ExecThread::SSReady;
					break;
				case ExecThread::SSReady:
					// Regular context switch from within wait cond. 
					// Continue waiting and re-eval again. 
					i->state=ExecThread::Waiting;
					i->schedule_status=ExecThread::SSReady;
					++i->wait_cond.must_eval;
					ExecThread::wait_cond_re_eval_cs=1;
					break;
				case ExecThread::SSRequest:
					// This is illegal and should have given 
					// a user error and abort reported in the wait 
					// eval func. 
					assert(0);  break;
				default:
					// All other states are illegal. 
					// No thread may have SSDisabled (cloning is enabled here), 
					// SSDoneClone and /*anything>0*/ must be processed by 
					// the clone() call which re-sets the status to SSReady. 
					// SSDelay may not be called from wait...
					assert(0);
			}
		}
	}
	ExecThreadInfo::currently_evaluating_any_wcond=0;
}


ExecThread *AniAnimation::_DoContextSwitch(Animation_Context *ctx,
	ExecThread *run_thread,int eval_rv)
{
	ExecThread *old_thread=run_thread;
	for(;;)
	{
		run_thread = run_thread->next ? 
			run_thread->next : ctx->thread_list.first();
		if(run_thread==old_thread)  break;
		if(run_thread->state==ExecThread::Inactive)  break;
	}
	if(run_thread->state==ExecThread::Inactive)
	{
		// This is the thread we'll run next. 
		run_thread->state=ExecThread::Running;
	}
	else
	{  run_thread=NULL;  }
	
	if(!eval_rv)
	{
		// Thread now exiting. 
		ctx->thread_list.dequeue(old_thread);
		delete old_thread;
		if(run_thread==old_thread)
		{  run_thread=NULL;  }
	}
	
	return(run_thread);
}


void AniAnimation::_DoCheckThreadDelays(Animation_Context *ctx,
	ExecThread **run_thread)
{
	for(ExecThread *i=ctx->thread_list.first(); i; i=i->next)
	{
		if(i->state!=ExecThread::Delayed)  continue;
		if(!i->tdelay.TestEndDelay())  continue;
		// Okay, delay stopped. 
		i->state=ExecThread::Inactive;
		// If there is no thread to be run next, use this one: 
		if(!*run_thread)
		{  *run_thread=i;  }
	}
}


void AniAnimation::RunContext(Animation_Context *ctx)
{
	//assert(!active_context);
	
	// This is the new active context. 
	//active_context=ctx;
	
	// If there are no threads, we need to create one. 
	ExecThread *run_thread=NULL;
	if(ctx->thread_list.is_empty())
	{
		run_thread=new ExecThread(ctx,_GetUniqueThreadID(ctx));
		run_thread->schedule_status=ExecThread::SSReady;  // Cloning allowed. 
		ctx->thread_list.append(run_thread);
		run_thread->state=ExecThread::Running;
	}
	else
	{
		// Otherwise, we need to resume. 
		// Find thread which is running or inactive. 
		ExecThread *inact_thread=NULL;
		for(ExecThread *i=ctx->thread_list.first(); i; i=i->next)
		{
			if(i->state==ExecThread::Running)
			{  assert(!run_thread);  run_thread=i;  }
			else if(i->state==ExecThread::Inactive && !inact_thread)
			{  inact_thread=i;  }
		}
		if(!run_thread && inact_thread)
		{
			// Wake up inact_thread: 
			run_thread=inact_thread;
			run_thread->state=ExecThread::Running;
		}
		if(!run_thread)
		{
			fprintf(stderr,"Cannot resume: No running and no inactive thread.\n");
			assert(0);
		}
	}
	assert(run_thread);
	
	if(!ctx->setting->GetSettingInitEvalFunc()) assert(0);
	
	// THE MAIN LOOP of the setting. 
	fprintf(stderr,"VM: Running preliminary main loop...\n");
	for(;;)
	{
		// Before we run the threads again: Check the delays: 
		_DoCheckThreadDelays(ctx,&run_thread);
		
		// Okay, run setting init function: 
		// Execute threads in round-robin-like scheduling until no 
		// thread can run any longer (all exited, delayed or waiting). 
		if(!run_thread)
		{  fprintf(stderr,"VM: No threads to run for time=???.\n"
			/*ctx->ccif->CurrentSettingTime()*/);  }
		else
		{
			fprintf(stderr,"VM: Running threads for time=???.\n"
				/*ctx->ccif->CurrentSettingTime()*/);
			
			for(;;)
			{
				// Regularly execute thread: 
				int eval_rv=_DoRunThread(ctx,run_thread);
				// Check all waiting conditions: 
				_DoCheckAllWaitConditions(ctx);
				
				// Do a regular context switch: 
				run_thread=_DoContextSwitch(ctx,run_thread,eval_rv);
				
				if(!run_thread)
				{
					// All threads either exited or are in waiting/delayed state. 
					// This means that we now compute the internal ani 
					// variables and then emit the frame. 
					fprintf(stderr,"VM: All threads done.\n");
					break;
				}
			}
		}
		
		// All threads done; call calc core to do its work: 
		for(int i=0; i<CC::CCIF_Factory::NFactories(); i++)
		{
			ctx->ccif[i]->ComputeAll();
		}
		
		// Finally, write frame: 
		// Need to temporarily disable the ani context. 
		// I'm not sure if this is 100% needed but it is to be sure 
		// (e.g. that no objects are created from within POV expressions). 
		//FIXME: the eval thread has a context set and we do not need 
		//       active_context anywhere any longer. TEST ME!!
		//active_context=NULL;
		pov_frame_writer.WriteFrame(ctx,ctx->eval_thread);
		//active_context=ctx;
		
		//...and go on in time. 
		// FIXME: NOTE: this must be changed. Need to compute time step by 
		// minimum of next frame, wait conditions and all calc cores. 
		// Need proper way to decide on the end of the ani. 
		// See also: a_setting.cc -> SetTimeRange(). 
		// And search for "FIXME: ccif[0]". 
		int n_done=0;
		for(int i=0; i<CC::CCIF_Factory::NFactories(); i++)
		{
			if(ctx->ccif[i]->TimeStep())  ++n_done;
		}
		if(n_done)  break;
	}
	fprintf(stderr,"VM: Left main setting loop.\n");
	
	// Deactivate context again. 
	//active_context=NULL;
}


AniUserFunction *AniAnimation::FindAnimationFunction(const RefString &name)
{
	for(AniFunction *func=functionlist.first(); func; func=func->next)
	{
		if(func->ASType()!=AST_UserFunc)  continue;
		AniUserFunction *ufunc=(AniUserFunction*)func;
		if(!(ufunc->GetAttrib() & AniUserFunction::IS_Animation))  continue;
		if(!!name && name!=ufunc->GetName())  continue;
		return(ufunc);
	}
	return(NULL);
}


ExprValue &AniAnimation::GetInternalVarRef(int bvi)
{
	assert(bvi>=0 && bvi<_BVI_LAST);
	return(ivar_ev[bvi]);
}


void AniAnimation::vn_change(void *hook)
{
	int bvi=((ExprValue*)hook)-ivar_ev;
	assert(bvi>=0 && bvi<_BVI_LAST);
	
	switch(bvi)
	{
		case BVI_Time:
		{
			double new_val=ivar_ev[bvi].GetPODDouble();
			if(new_val==ani_time)  return;
			
			// ani_time_ev changed. 
			assert(!"!implemented");
			
		}	break;
		default: assert(0);
	}
}

void AniAnimation::vn_delete(void *hook)
{
	int bvi=((ExprValue*)hook)-ivar_ev;
	assert(bvi>=0 && bvi<_BVI_LAST);
	
	// This may not happen. 
	assert(0);
}


POV::ObjectSpec *AniAnimation::FindPOVObject(const RefString &objname)
{
	return(pov_cmd_parser.FindObject(objname));
}


int AniAnimation::ScanPOVFile(const RefString &filename)
{
	int errors=0;
	
	fprintf(stderr,"POV: Scanning file \"%s\": [started]\n",
		filename.str());
	errors=pov_cmd_parser.ParseFile(filename,/*AniParser=*/NULL);
	fprintf(stderr,"POV: Scanning file \"%s\": %d errors (%s)\n",
		filename.str(),errors,strerror(errno));
	
	return(errors);
}


int AniAnimation::POVAttach(Animation_Context *ctx,AniObjectInstance *obj_inst,
	const RefString &objname)
{
	POV::ObjectSpec *ospec=FindPOVObject(objname);
	if(!ospec)  return(1);
	
	if(ctx)
	{
		// Find associated object instance node or create new one: 
		ssize_t idx;
		Animation_Context::ObjectInstanceNode *oin=
			ctx->_FindAllocObjectInstanceNode(obj_inst,&idx,/*alloc_new=*/1);
		
		int rv=oin->DoPOVAttach(ospec);
		
		// If there was an error, we may not need the node. Remove in 
		// such a case. 
		ctx->_CheckDeleteObjectInstanceNode(idx);
		
		return(rv);
	}
	else
	{
		//assert(!active_context);
		
		assert(0); /* implement me: pov attach without context */
		
		//int rv=oin->DoPOVAttach(ospec);
		//return(rv);
	}
	
	assert(0);  // never reached
	return(0);
}


int AniAnimation::POVDetach(Animation_Context *ctx,AniObjectInstance *obj_inst,
	const RefString &objname)
{
	if(ctx)
	{
		//assert(ctx==active_context);
		ssize_t idx;
		Animation_Context::ObjectInstanceNode *oin=
			ctx->_FindAllocObjectInstanceNode(obj_inst,&idx,/*alloc_new=*/0);
		if(!oin)  return(1);  // not attached
		assert(idx>=0);
		
		// Find passed object in the attachment list: 
		Animation_Context::POVAttachmentNode *an=
			oin->FindAttachmentNodeByName(objname);
		if(!an)  return(1);  // not attached
		
		int rv=oin->DoPOVDetach(an);
		ctx->_CheckDeleteObjectInstanceNode(idx);
		return(rv);
	}
	else
	{
		//assert(!active_context);
		
		assert(0); /* implement me: pov detach without context */
		// NOTE: also implement a _ForceDeleteInstanceNode() analogon 
		// for case where we're without context...
	}
	
	assert(0);  // never reached
	return(0);
}


AniAnimation::AniAnimation(const RefString &name,ANI::TreeNode *_treenode) : 
	AniAniSetObj(AST_Animation,name,_treenode,/*parent=*/NULL),
	ValueNotifyHandler(),
	pov_cmd_parser(),
	pov_frame_writer(),
	context_list()
{
	//active_context=NULL;
	
	thread_id_counter=0;
	
	// Set up initial values for the internal vars: 
	ani_time=0.0;
	
	int _count=0;
	ivar_ev[BVI_Time].assign(ani_time);  ++_count;
	assert(_count==_BVI_LAST);
	
	// Register value change notifier: 
	for(int i=0; i<_BVI_LAST; i++)
	{
		int rv=ValueNotifyHandler::VNInstall(ivar_ev[i],&ivar_ev[i]);
		assert(!rv);
	}
	
	// Register internal functions (atan(),sin(),$pov(),...) 
	// and variables ($time,...): 
	AniInternalFunction::RegisterInternalFunctions(this);
	AniInternalVariable::RegisterInternalVariables(this);
}

AniAnimation::~AniAnimation()
{
	for(int i=0; i<_BVI_LAST; i++)
	{
		int rv=ValueNotifyHandler::VNUninstall(ivar_ev[i]);
		assert(rv==0);
	}
	
	// Actually, this should not be necessary here: 
	_CleanupAllContexts();
}

