/*
 * pov-core/povwriter.cc
 * 
 * POV frame writer. 
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

#include "povwriter.h"
#include <core/animation.h>
#include <core/aniinstance.h>

#include <calccore/ccif.h>
#include <objdesc/omatrix.h>

#include <../config.h>

#include <stdarg.h>


namespace POV
{

void FrameWriter::_EvalExpr(String *dest,ExecThread *info,PTNExpression *expr,
	bool pass_null,bool remove_quotes)
{
	assert(expr->PNType()==PTN_Expression);
	
	ANI::TNExpression *e=(ANI::TNExpression*)expr->down.first();
	assert(e->NType()==ANI::TN_Expression);
	
	// Jzst to be sure: These vars already should have these values: 
	info->state=ExecThread::Running;
	info->schedule_status=ExecThread::SSDisabled;
	
	ANI::ExprValue tmp;
	for(int iter=0;;iter++)
	{
		int rv=e->Eval(tmp,info);
		if(!rv)  break;
		if(info->schedule_status!=ExecThread::SSDisabled)
		{
			fprintf(stderr,"What'chew doin' there, Willis?\n");
			abort();
		}
		if(iter>=64)
		{
			// NOTE: We should not have any context switches at all here 
			// (only regular ones caused by the context switch test hack). 
			Error(expr->GetLocationRange(),
				"more than %d context switches while evaluating "
				"POV-expression\n",
				iter);
			abort();
		}
	}
	
	if(pass_null && tmp.is_null())
	{  dest->set(NULL);   }
	else
	{
		*dest=tmp.ToString();
		
		while(remove_quotes)
		{
			if(!dest->str())  break;
			if(*dest->str()=='\"')
			{  dest->skip(1);  }
			size_t len=dest->len();
			if(len<1)  break;
			if(dest->str()[len-1]=='\"')
			{  dest->trunc(len-1);  }
			break;
		}
	}
}


int FrameWriter::_WriteObject(ExecThread *eval_thread,
	Animation_Context::ObjectInstanceNode *oin,
	Animation_Context::POVAttachmentNode *pan)
{
	CC::CCIF_Object *active_ccif=oin->obj_instance->GetActiveCCIF();
	assert(active_ccif);   // We may not be here otherwise. 
	
	{
		double time_tmp;
		int rv=active_ccif->GetVal(CC::CCIF_Object::ValTime,&time_tmp,
			/*CCIF_ExecThread=*/eval_thread);
		assert(rv==0);
		ofile.Printf(
			"// Object %s: cc=%s, time=%g\n",
			oin->obj_instance->GetASB()->CompleteName().str(),
			active_ccif->CoreFactory()->Name(),
			time_tmp);
	}
	
	ofile.Printf(
		"// POV attachment \"%s\" @%s:\n",
		pan->osdef->oname.str(),
		pan->osdef->GetLocationRange().PosRangeString().str());
	
	// Memvers of *pan: 
	//   POV::ObjectSpec *osdef
	//   POV::PTNObjectSpec *ospec;  (clone of osdef->ospec)
	
	PTNObjectEntry *oe_params=NULL;
	PTNObjectEntry *oe_defs=NULL;
	PTNObjectEntry *oe_include=NULL;
	PTNObjectEntry *oe_type=NULL;
	PTNObjectEntry *oe_append_raw=NULL;
	for(ANI::TreeNode *tn=pan->ospec->down.first(); tn; tn=tn->next)
	{
		assert(((POVTreeNode*)tn)->PNType()==PTN_ObjectEntry);
		switch(((PTNObjectEntry*)tn)->asymbol)
		{
			// Ignore these: 
			case PTNObjectEntry::AS_None:
			case PTNObjectEntry::AS_Macro:
			case PTNObjectEntry::AS_Declare:  break;
			// These are interesting: 
			case PTNObjectEntry::AS_Params:
				assert(!oe_params);
				oe_params=(PTNObjectEntry*)tn;
				break;
			case PTNObjectEntry::AS_Defs:
				assert(!oe_defs);
				oe_defs=(PTNObjectEntry*)tn;
				break;
			case PTNObjectEntry::AS_Include:
				assert(!oe_include);
				oe_include=(PTNObjectEntry*)tn;
				break;
			case PTNObjectEntry::AS_Type:
				assert(!oe_type);
				oe_type=(PTNObjectEntry*)tn;
				break;
			case PTNObjectEntry::AS_AppendRaw:
				assert(!oe_append_raw);
				oe_append_raw=(PTNObjectEntry*)tn;
				break;
			default: assert(0);
		}
	}
	
	// First, output the required includes in case we did not yet 
	// emit them. 
	if(oe_include)
		for(ANI::TreeNode *tn=oe_include->down.first(); tn; tn=tn->next)
		{
			String valstr;
			_EvalExpr(&valstr,eval_thread, (PTNExpression*)tn );
			bool already_included=included_files.find(valstr.str());
			if(!already_included)
			{  included_files.append(valstr.str());  }
			ofile.Printf("%s#include %s\n",
				already_included ? "// " : "",
				valstr.str());
		}
	
	// Then, include the file from which we got the object in 
	// case we did not yet: 
	do {
		// Get file with path: 
		RefString fileP(
			pan->osdef->GetLocationRange().pos0.GetFile().GetPath());
		
		// Cut away the path for now. Don't know if that should be kept. 
		// User can manually specify a file in the include={} entry of the 
		// object spec, so this feature could also be disabled. 
		const char *fstr=rindex(fileP.str(),'/');
		if(fstr) ++fstr; else fstr=fileP.str();
		// File without path but with `"' at beginning and end: 
		RefString fileNP;
		fileNP.sprintf(0,"\"%s\"",fstr);
		
		bool already_included=included_files.find(fileNP);
		if(already_included)
		{  included_files.append(fileNP);  }
		
		ofile.Printf("%s#include %s  // (primary)\n",
			already_included ? "// " : "",
			fileNP.str());
	} while(0);
	
	// So that we know where we are from reading the POV output :)
	ofile.Printf("#debug \"[%s]\"\n",pan->osdef->oname.str());
	
	// Next, output the defs: 
	if(oe_defs) for(ANI::TreeNode *tn=oe_defs->down.first(); tn; tn=tn->next)
	{
		PTNAssignment *ass=(PTNAssignment*)tn;
		assert(ass->PNType()==PTN_Assignment);
		String valstr;
		_EvalExpr(&valstr,eval_thread, (PTNExpression*)ass->down.last() );
		ofile.Printf("#declare %s=%s;\n",
			((ANI::TNIdentifier*)ass->down.first())->CompleteStr().str(),
			valstr.str());
	}
	
	// The object compound: 
	bool no_scope_around=0;
	if(oe_type)
	{
		String valstr;
		_EvalExpr(&valstr,eval_thread, (PTNExpression*)oe_type->down.first(),
			/*pass_null=*/1,/*remove_quotes=*/1 );
		const char *str=valstr.str();
		if(str)
		{  ofile.Printf("%s",str);  }
		else
		{  no_scope_around=1;  }
	}
	else
	{  ofile.Printf("object");  }
	
	if(!no_scope_around)
	{  ofile.Printf(" { ");  }
	
	switch(pan->osdef->otype)
	{
		case ObjectSpec::OT_Macro:
		{
			ofile.Printf("%s(",pan->osdef->oname.str());
			String valstr;
			if(oe_params) for(ANI::TreeNode *tn=oe_params->down.first(); 
				tn; tn=tn->next)
			{
				_EvalExpr(&valstr,eval_thread, (PTNExpression*)tn );
				ofile.Printf("%s%s",
					valstr.str(),
					tn->next ? "," : "");
			}
			ofile.Printf(")\n");
		}	break;
		case ObjectSpec::OT_Declare:
			ofile.Printf("%s\n",pan->osdef->oname.str());
			break;
		default: assert(0);
	}
	
	// Write transformation: 
	NUM::ObjectMatrix omat(NUM::ObjectMatrix::NoInit);
	int rv=active_ccif->GetVal(CC::CCIF_Object::ValMatrix,&omat,
		/*CCIF_ExecThread=*/eval_thread);
	assert(rv==0);
	ofile.Printf("\tmatrix <%g,%g,%g, %g,%g,%g, %g,%g,%g, %g,%g,%g>",
		omat[0][0],omat[1][0],omat[2][0],
		omat[0][1],omat[1][1],omat[2][1],
		omat[0][2],omat[1][2],omat[2][2],
		omat[0][3],omat[1][3],omat[2][3] );
	
	if(oe_append_raw)
	{
		String valstr;
		_EvalExpr(&valstr,eval_thread,
			(PTNExpression*)oe_append_raw->down.first(),
			/*pass_null=*/0,/*remove_quotes=*/1 );
		if(valstr.str() && *valstr.str())
		{  ofile.Printf("\n\t%s",valstr.str());  }
	}
	
	if(!no_scope_around)
	{  ofile.Printf("\n}\n");  }   // object scope
	
	ofile.Printf("\n");
	
	return(0);
}


int FrameWriter::WriteFrame(Animation_Context *ctx,ExecThread *eval_thread)
{
	int errors=0;
	
	// Allocate some special stack frames: 
	eval_thread->stack->AllocateEmptyFrames2( ANI::ScopeInstance()/*null ref*/ );
	
	// Make sure to clear the state before writing this file. 
	included_files.clear();   // No files yet included. 
	
	bool openfail=0;
	RefString fname;
	fname.sprintf(0,"f%07d.pov",frame_no);
	do {
		// First, open the file: 
		if(ofile.Open(fname))
		{  ++errors;  openfail=1;  break;  }
		
		// Write head: 
		AniScopeBase *_ani=ctx->setting;
		for(;_ani->Parent();_ani=_ani->Parent());
		assert(_ani->ASType()==AniScopeBase::AST_Animation);
		ofile.Printf(
			"/*** POVRay frame file generated by AniVision-%s ***/\n"
			"// AniRoot: %s\n"
			"// Setting: %s  @%s\n"
			"// Context: \"%s\"\n"
			"// Frame:   %d\n\n",
			VERSION,
			_ani->GetTreeNode()->GetLocationRange().pos0.GetFile()//...
				.GetPath().str(),
			ctx->setting->CompleteName().str(),
				ctx->setting->GetTreeNode()->GetLocationRange().//...
					PosRangeString().str(),
			ctx->name.str(),
			frame_no);
		
		// Advertise ourselves :)
		// This is done so that POV code can see if it is parsed from 
		// within an animation or in plain POV context by using 
		//    #ifdef(AniVision) ... #end
		// or
		//    #ifndef(AniVision) ... #end
		ofile.Printf(
			"#declare AniVision=1;\n\n");
		
		int n_obj_active=0,n_obj_total=0,n_att_total=0;
		// Loop through object instance nodes of this context: 
		for(size_t oidx=0; oidx<ctx->pov_instlist.NElem(); oidx++)
		{
			++n_obj_total;
			Animation_Context::ObjectInstanceNode *oin=ctx->pov_instlist[oidx];
			
			// If there are no attachments, continue right now. 
			if(oin->attach_list.is_empty()) continue;
			++n_obj_active;
			
			CC::CCIF_Object *active_ccif=oin->obj_instance->GetActiveCCIF();
			if(!active_ccif)
			{
				ofile.Printf(
					"// Object %s: [no active calc core]\n",
					oin->obj_instance->GetASB()->CompleteName().str());
				continue;
			}
			
			int active_val;
			int rv=active_ccif->GetVal(CC::CCIF_Object::ValActive,&active_val,
				/*CCIF_ExecThread=*/eval_thread);
			assert(rv==0);
			if(!active_val)
			{
				ofile.Printf(
					"// Object %s: inactive with %d attachments\n",
					oin->obj_instance->GetASB()->CompleteName().str(),
					oin->attach_list.count());
				continue;
			}
			
			// Adjust "this" pointers on the stack pages: 
			ANI::ScopeInstance inst(ANI::ScopeInstance::InternalCreate,
				oin->obj_instance);
			eval_thread->stack->GetFrame(0)->this_ptr=inst;
			eval_thread->stack->GetFrame(1)->this_ptr=inst;
			
			// Loop through the attachments: 
			for(Animation_Context::POVAttachmentNode *pan=
					oin->attach_list.first(); 
				pan; pan=pan->next)
			{
				_WriteObject(eval_thread,oin,pan);
				++n_att_total;
			}
		}
		
		ofile.Printf(
			"// EOF: Objects: %d/%d active; POV objects in scene: %d\n",
			n_obj_active,n_obj_total,n_att_total);
	} while(0);
	
	if(ofile.Close()) ++errors;
	
	// Important: Set empty instances again (for de-ref): 
	ANI::ScopeInstance inst;
	eval_thread->stack->GetFrame(0)->this_ptr=inst;
	eval_thread->stack->GetFrame(1)->this_ptr=inst;
	
	if(!openfail)
	{
		fprintf(stderr,"POV: Wrote frame %d -> \"%s\": %u bytes (%d errors)\n",
			frame_no,
			fname.str(),
			ofile.bytes_written,errors);
		++total_files_written;
		total_bytes_written+=ofile.bytes_written;
	}
	
	// Free the special stack frames again: 
	eval_thread->stack->FreeEmptyFrames2();
	
	++frame_no;
	return(errors);
}


FrameWriter::FrameWriter() : 
	ofile(),
	included_files()
{
	frame_no=0;
	
	total_files_written=0;
	total_bytes_written=0;
}

FrameWriter::~FrameWriter()
{
	fprintf(stderr,"POV: Total: Wrote %d frame files (%u bytes).\n",
		total_files_written,total_bytes_written);
}


//------------------------------------------------------------------------------

int FrameWriter::OFile::Printf(const char *fmt,...)
{
	assert(fp);
	
	va_list ap;
	va_start(ap,fmt);
	int rv=vfprintf(fp,fmt,ap);
	va_end(ap);
	
	if(rv<0)
	{  fprintf(stderr,"POV: While writing \"%s\": %s\n",
		fname.str(),strerror(errno));  abort();  }
	else
	{  bytes_written+=rv;  }
	
	return(0);
}


int FrameWriter::OFile::Open(RefString file)
{
	assert(!fp);
	
	fname=file;
	bytes_written=0;
	
	fp=fopen(fname.str(),"w");
	if(!fp)
	{  fprintf(stderr,"POV: Failed to open frame file \"%s\": %s\n",
		fname.str(),strerror(errno));  return(-1);  }
	
	return(0);
}


int FrameWriter::OFile::Close()
{
	if(!fp) return(0);
	
	FILE *tmp=fp;
	fp=NULL;
	fname.deref();
	
	if(fclose(tmp))
	{  fprintf(stderr,"POV: Failed to close frame file \"%s\": %s\n",
		fname.str(),strerror(errno));  return(-1);  }
	
	return(0);
}


FrameWriter::OFile::OFile() : 
	fname()
{
	fp=NULL;
	bytes_written=0;
}

FrameWriter::OFile::~OFile()
{
	Close();
}

}  // end of namespace POV
