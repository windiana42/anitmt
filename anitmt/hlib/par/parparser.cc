namespace par
{

#error screwed up. 

void ParameterParser::_ErrorHeader(const ParamArg *arg)
{
	switch(arg->otype)
	{
		case ParamArg::FromFile: 
			fprintf(stderr,"%s: %s:%d: ",prg_name,arg->origin.origin,arg->opos);
			break;
		case ParamArg::FromCmdLine:
			fprintf(stderr,"%s: argv[%d]: ",prg_name,arg->opos);
			break;
		default:
			fprintf(stderr,"%s: [unknown source %s:%d]: ",
				prg_name,arg->origin.origin,arg->opos);
			break;
	}
}

int ParameterParser::ParParseError(const ParamArg *arg,ParParseErrval errval)
{
	_ErrorHeader(arg);
	switch(errval)
	{
		case PPENameOmitted:
			fprintf(stderr,"value specified but name omitted\n");  break;
		case PPEUnknownArg:
			fprintf(stderr,"unknown argument \"%.*s\"\n",
				int(arg->namelen),arg->name);
			break;
		default:
			fprintf(stderr,"unknown error %d [this is a bug]\n",errval);
			break;
	}
	return(0);
}


int ParameterParser::WarnAlreadySet(const ParamArg *arg,
	const ParameterHandlerBase::ParDesc *pd,
	ParamArg::Origin *origin)
{
	// We do not warn for options. 
	if(pd->ptype==PTOption)
		return(0);
	
	bool same_origin = 
		(origin->otype==arg->origin.otype) && 
		((origin->origin==arg->origin.origin) ? true : 
			!strcmp(origin->origin,arg->origin.origin));
	
	#warning !!! THIS IS MESSED UP !!! (*origin <-> *arg->origin) !!!
	
	if(same_origin)
	{
		_ErrorHeader(arg);
		fprintf(stderr,"warning: %s previously set in ",_Skip_No(arg));
		switch(otype)
		{
			case ParamArg::FromFile: 
				fprintf(stderr,"%s:%d\n",origin->origin,origin->opos);  break;
			case ParamArg::FromCmdLine:
				fprintf(stderr,"argv[%d]\n",arg->opos);  break;
			default:
				fprintf(stderr,"[unknown source %s:%d]\n",
					arg->origin,arg->opos);
				break;
		}
	}
	
	return(0);
}


int ParameterParser::ValParseError(
	ParameterHandlerBase::ParDesc *,
	ParamArg *arg,ParParseState pps)
{
	if(!pps)  // PPSSuccess
	{  return(0);  }
	
	_ErrorHeader(arg);
	
	const char *argname=_Skip_No(arg);
	int argnamelen=(arg->name+arg->namelen)-argname;
	
	switch(pps)
	{
		case PPSSuccess:  break;  // against compiler warning 
		// Warnings: 
		case PPSGarbageEnd:
			fprintf(stderr,"garbage at the end of value for %.*s\n",
				argnamelen,argname);
			break;
		case PPSValOmitted:
			fprintf(stderr,"required value for %.*s omitted\n",
				argnamelen,argname);
			break;
		case PPSIllegalArg:
			fprintf(stderr,"illegal/badly formatted value for %.*s\n",
				argnamelen,argname);
			break;
		case PPSValOORange:
			fprintf(stderr,"value for %.*s out of range\n",
				argnamelen,argname);
			break;
		case PPSArgTooLong:
			fprintf(stderr,"%.*s: argument too long\n",
				argnamelen,argname);
			break;
		case PPSOptAssign:
			fprintf(stderr,"%.*s is an option, not a parameter\n",
				argnamelen,argname);
			break;
		case PPSUnknown:  // should never be reached. 
		default:
			assert(0);
			break;
	}
	return(0);
}

}  // namespace end 

