#error waiting to get implemented

// A section parameter handler is a handler which is able to deal with all 
// parameters belonging to a section without necessarily knowing about them. 
// It has funcktions called for all parameters (which have names that 
// indicate that it is) in some specified section and you can do your 
// own parameter handling if you like. E.g. it allows you to deal with 
// previously unknown parameters or handle unrecognized parameters 
// differently. 
// A section parameter handler 
class SectionParameterHandler
{
	public:
		struct SHInfo
		{
			ParamArg *arg;
			Section *s;
			ParamInfo *pi;  // or NULL if not recognized
			const char *nend;
		};
	private:
		ParameterManager *manager;
		
	protected:
		// This is called
		#error fixme
		virtual int Unknown(UnknownInfo *info)
			{  return(0);  }
		
	public:  _CPP_OPERATORS_FF
		SectionParameterHandler(ParameterManager *manager,int *failflag=NULL);
		~SectionParameterHandler();
		
		// Get pointer to parameter manager: 
		ParameterManager *parmanager()
			{  return(manager);  }
		
		Subscribe(Section *s);
		Subscribe(const char *section,Section *top=NULL);
		
		Unsubscribe(Section *s);
};


SectionParameterHandler::SectionParameterHandler(
	ParameterManager *_manager,int *failflag)
{
	int failed=0;
	
	manager=_manager;
	
	if(parmanager()->RegisterSectionParameterHandler(this))
	{  --failed;  }
	
	if(failflag)
	{  *failflag+=failed;  }
	else if(failed)
	{  ConstructorFailedExit();  }
}

SectionParameterHandler::~SectionParameterHandler()
{
	parmanager()->UnregisterSectionParameterHandler(this);
}
