/*
 * logger.cpp
 * 
 * Message logger class implementation. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "logger.hpp"

#define UNSET_NEG_MAGIG (-31969)


// Init static data: 
MessageLogger *MessageLogger::logger=NULL;


void MessageLogger::_DoStore(MyRecorder *rec,
	const HTime &time,const RefString &msg,MType mtype)
{
	MEntry *ent=rec->rec->StorePtr();
	
	// NOTE: Messages in both buffers get counted twice even 
	//       if they are shared among several RefStrings. 
	size_t ent_bytes=ent->msg.len()+sizeof(int)+1;
	rec->tot_bytes-=ent_bytes;
	
	ent->time=time;
	ent->msg=msg;
	ent->mtype=mtype;
	
	ent_bytes=ent->msg.len()+sizeof(int)+1;
	rec->tot_bytes+=ent_bytes;
	
	if(rec->max_bytes<rec->tot_bytes)
	{  rec->max_bytes=rec->tot_bytes;  }
	
	if(rec->max_msgs<rec->rec->NUsed())
	{  rec->max_msgs=rec->rec->NUsed();  }
}


int MessageLogger::_LogMessage(MType mtype,const char *fmt,va_list ap)
{
	int errors=0;
	
	// First check, if we do log this message at all: 
	bool store_in_erec=(erec.rec && (mtype & (MTSVerbose|MTWarning|MTError)));
	bool store_in_mrec=(mrec.rec && (mtype & hist_mode));
	
	if(!store_in_mrec && !store_in_erec)
	{  return(errors);  }
	
	// First, format message into string: 
	RefString msg;
	msg.vsprintf(/*maxlen=*/1024/*...which must be enough for a line*/,
		fmt,ap);
	if(!msg)
	{  msg=allocfail_str;  ++errors;  }
	
	// Get current time; we can use the "most current" time because the 
	// FDManager will update it frequently enough. 
	HTime curr(HTime::MostCurr);
	
	if(store_in_mrec)
	{  _DoStore(&mrec,curr,msg,mtype);  }
	if(store_in_erec)
	{  _DoStore(&erec,curr,msg,mtype);  }
	
	return(errors);
}


int MessageLogger::_vaMessage(MType mtype,const char *fmt,va_list ap)
{
	bool write_to_tty=1;
	if(logger && !(logger->tty_mode & mtype))  write_to_tty=0;
	
	int rv=0;
	
	// First, write to tty if needed: 
	if(write_to_tty)
	{
		FILE *out = (mtype & (MTWarning|MTError)) ? stderr : stdout;
		const char *col_start=NULL;
		const char *col_end=NULL;
		bool append_nl=0;
		const char *prefix_str=NULL;
		switch(mtype)
		{
			case MTDVerbose:
				//prefix_str="D: ";
				// Plain (non-colored) output. 
				break;
			case MTVerbose:
				//prefix_str="V: ";
				if(rv_oparams.enable_color_stdout)
				{
					col_start=rv_oparams.console_blue_start;
					col_end=rv_oparams.console_blue_end;
				}
				break;
			case MTSVerbose:
				prefix_str="S: ";
				// NOTE: VerboseSpecial() adds a newline by itself 
				//       because of the green beackground...
				// fprintf(stdout,"\33[0;42m");   // <-- green background
				if(rv_oparams.enable_color_stdout)
				{
					col_start=rv_oparams.console_Blue_start;
					col_end=rv_oparams.console_Blue_end;
				}
				append_nl=1;
				break;
			case MTWarning:
				prefix_str="W: ";
				if(rv_oparams.enable_color_stderr)
				{
					col_start=rv_oparams.console_red_start;
					col_end=rv_oparams.console_red_end;
				}
				break;
			case MTError:
				prefix_str="E: ";
				if(rv_oparams.enable_color_stderr)
				{
					col_start=rv_oparams.console_Red_start;
					col_end=rv_oparams.console_Red_end;
				}
				break;
			default:
				prefix_str="??: ";
				break;  // Should not be here...
		}
		
		// Actually write it: 
		if(col_start)
		{  fprintf(out,"%s",col_start);  }
		if(prefix_str && logger && (logger->tty_mode & MT_Prefix))
		{  fprintf(out,"%s",prefix_str);  }
		rv=vfprintf(out,fmt,ap);
		if(col_end)
		{  fprintf(out,append_nl ? "%s\n" : "%s",col_end);  }
		else if(append_nl)
		{  fprintf(out,"\n");  }
		fflush(out);
	}
	
	// Then, note down in logger: 
	if(logger)
	{  logger->_LogMessage(mtype,fmt,ap);  }
	
	return(rv);
}


int MessageLogger::_Error(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv=_vaMessage(MTError,fmt,ap);
	va_end(ap);
	return(rv);
}

int MessageLogger::_Warning(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv=_vaMessage(MTWarning,fmt,ap);
	va_end(ap);
	return(rv);
}

int MessageLogger::_Verbose(int vspec,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	bool is_debug=(vspec & (VERBOSE_DBG|VERBOSE_DBGV));
	int rv=_vaMessage(is_debug ? MTDVerbose : MTVerbose,fmt,ap);
	va_end(ap);
	return(rv);
}

int MessageLogger::_VerboseSpecial(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv=_vaMessage(MTSVerbose,fmt,ap);
	va_end(ap);
	return(rv);
}


int MessageLogger::_vaWarning(const char *fmt,va_list ap)
{
	return(_vaMessage(MTWarning,fmt,ap));
}

int MessageLogger::_vaError(const char *fmt,va_list ap)
{
	return(_vaMessage(MTError,fmt,ap));
}


int MessageLogger::_ParseMsgSpec(const char *_str,MType *mtype,
	const char *arg_for_error,bool is_tty)
{
	int mode=+1;
	int locerr=0;
	for(const char *str=_str; *str; str++)
	{
		MType bits=MTNone;
		switch(*str)
		{
			case '+':  mode=+1;  break;
			case '-':  mode=-1;  break;
			case 'v':  bits=MTVerbose;   break;
			case 'w':  bits=MTWarning;   break;
			case 'e':  bits=MTError;     break;
			case 's':  bits=MTSVerbose;  break;
			case 'd':  bits=MTDVerbose;  break;
			case 'a':  bits=(MType)
				(MTVerbose|MTDVerbose|MTSVerbose|MTWarning|MTError);
				break;
			case 'P':
				if(is_tty)
				{  bits=MT_Prefix;  }
				else
				{  ++locerr;  }
				break;
			default:  ++locerr;  break;
		}
		if(bits)
		{
			if(mode>0)
			{  *mtype=(MessageLogger::MType)((*mtype)|bits);  }
			else
			{  *mtype=(MessageLogger::MType)((*mtype)&~bits);  }
		}
	}
	
	if(locerr)
	{  Error("LOG: Illegal %s spec \"%s\"\n",arg_for_error,_str);  }
	
	return(locerr);
}


int MessageLogger::CheckParams()
{
	int failed=0;
	
	// Interprete tty_str: 
	if(p->tty_str.str())
	{  failed+=_ParseMsgSpec(p->tty_str.str(),&tty_mode,"-tty",/*is_tty=*/1);  }
	// ...and hist_msg:
	if(p->hist_msg.str())
	{  failed+=_ParseMsgSpec(p->hist_msg.str(),&hist_mode,"-hist-msg",
		/*is_tty=*/0);  }
	
	// Check history sizes: 
	if(p->ehist_size==UNSET_NEG_MAGIG)
	{
		// Set default. This should be >0 if the remote admin port is used. 
		// FIXME. 
		p->ehist_size=0;
	}
	if(p->hist_size==UNSET_NEG_MAGIG)
	{  p->hist_size=0;  }
	
	if(p->ehist_size<0)
	{
		Error("LOG: Illegal -ehist-size spec (%d)\n",p->ehist_size);
		p->ehist_size=0;
		++failed;
	}
	if(p->hist_size<0)
	{
		Error("LOG: Illegal -hist-size spec (%d)\n",p->hist_size);
		p->hist_size=0;
		++failed;
	}
	
	// Don't need param info any longer...
	p->tty_str.deref();
	p->hist_msg.deref();
	// DELETE(p); <-- Do not do that: we did not unregister the params. 
	
	if(!failed)
	{
		if( erec.Alloc(p->ehist_size) || mrec.Alloc(p->hist_size) )
		{  Error("LOG: %s\n",cstrings.allocfail);  ++failed;  }
	}
	
	if(!failed)
	{
		Verbose(MiscInfo,"LOG: Terminal output:");
		if(tty_mode & MTError)    Verbose(MiscInfo," errors");
		if(tty_mode & MTWarning)  Verbose(MiscInfo," warnings");
		if(tty_mode & (MTSVerbose|MTVerbose|MTDVerbose))
			Verbose(MiscInfo," verbose (");
		if(tty_mode & MTSVerbose)  Verbose(MiscInfo,"special%s",
			(tty_mode & (MTVerbose|MTDVerbose)) ? " " : "");
		if(tty_mode & MTVerbose)  Verbose(MiscInfo,"normal%s",
			(tty_mode & MTDVerbose) ? " " : "");
		if(tty_mode & MTDVerbose)  Verbose(MiscInfo,"debug");
		Verbose(MiscInfo,")\n");
		
		char tmp[8]="-";
		char *d=tmp;
		if(hist_mode & MTError)     *d++='e';
		if(hist_mode & MTWarning)   *d++='w';
		if(hist_mode & MTSVerbose)  *d++='s';
		if(hist_mode & MTVerbose)   *d++='v';
		if(hist_mode & MTDVerbose)  *d++='d';
		*d++='\0';
		Verbose(MiscInfo,"LOG: Message history: general: "
			"%u msgs [%s]; errors: %u msgs\n",
			mrec.Size(),tmp,erec.Size());
	}
	
	return(failed ? 1 : 0);
}


int MessageLogger::_SetUpParams()
{
	p=NEW<Params>();
	if(!p)  return(1);
	
	if(SetSection("m","Message logger/storage"))
	{  return(1);  }
	
	add_failed=0;
	AddParam("tty",
		"What to write on the tty. Use +/-aewsvdP (default +ewsvd-P):\n"
		"  \"v\" -> (normal) verbose messages    \"w\" -> warning messages\n"
		"  \"s\" -> special (verbose) messages   \"e\" -> error messages\n"
		"  \"d\" -> debug (verbose) messages     \"a\" -> all messages\n"
		"  \"P\" -> message prefix ('E'rror, 'W'arning, 'S'pecial verbose) \n"
		"Note that only such verbose messages enabled with -verbose will "
		"actually be generated at all.",
		&p->tty_str);
	
	#warning Default value? (Dependent on whether we use remote admin port)
	AddParam("ehist-size",
		"size (in messages) of error and warning (and special verbose) "
		"message history buffer; "
		"useful for remote admin port [not yet implemented], only",
		&p->ehist_size);
	AddParam("hist-size",
		"like ehist-size but for buffer storing all types of messages "
		"(including verbose); messages in both buffers (ehist and hist) "
		"consume memory only once",
		&p->hist_size);
	AddParam("hist-msg",
		"which messages to store in the history above buffer; spec "
		"like -tty but without 'P' flag (detault: +a)",
		&p->hist_msg);
	
	return(add_failed ? 1 : 0);
}


MessageLogger::MessageLogger(par::ParameterManager *parman,int *failflag) : 
	par::ParameterConsumer_Overloaded(parman,failflag),
	mrec(failflag),
	erec(failflag),
	allocfail_str(failflag)
{
	int failed=0;
	p=NULL;
	
	tty_mode=(MType)(MTVerbose|MTDVerbose|MTSVerbose|MTWarning|MTError);
	hist_mode=(MType)(MTVerbose|MTDVerbose|MTSVerbose|MTWarning|MTError);
	
	if(logger)  ++failed;  // Global logger already set. 
	
	if(allocfail_str.set("[allocation failure]"))  ++failed;
	
	if(_SetUpParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskMan");  }
	
	// Set up global logger: 
	logger=this;
}

MessageLogger::~MessageLogger()
{
	if(mrec.rec || erec.rec)
	{  Verbose(MiscInfo,
		"LOG: Msg hist:    size      max usage\n"
		"LOG:   all msg: %5um  %5dm/%4ukb\n"
		"LOG:   err/wrn: %5um  %5dm/%4ukb\n",
		mrec.Size(),mrec.max_msgs,mrec.max_bytes>>10,
		erec.Size(),erec.max_msgs,erec.max_bytes>>10);  }
	
	DELETE(p);
	
	// Cleanup global logger: 
	if(logger==this)
	{  logger=NULL;  }
}


//------------------------------------------------------------------------------

MessageLogger::Params::Params(int *failflag) : 
	tty_str(failflag),
	hist_msg(failflag)
{
	ehist_size=UNSET_NEG_MAGIG;
	hist_size=UNSET_NEG_MAGIG;
}

MessageLogger::Params::~Params()
{
}


//------------------------------------------------------------------------------

int MessageLogger::MyRecorder::Alloc(size_t size)
{
	if(!size)  return(0);
	rec=NEW1< CyclicDataRecorder<MEntry> >(size);
	if(!rec)  return(-1);
	tot_bytes=sizeof(MEntry)*size;
	max_bytes=tot_bytes;
	return(0);
}


MessageLogger::MyRecorder::MyRecorder(int * /*failflag*/)
{
	rec=NULL;
	
	tot_bytes=0;
	max_bytes=0;
	
	max_msgs=0;
}

MessageLogger::MyRecorder::~MyRecorder()
{
	DELETE(rec);
}
