#include "valuehandler.h"
#include "cmdline.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


namespace par
{

static inline int is_trim(char c)
{
	if(!c)  return(0);
	return(isspace(int(c)));
}


// The value cannot be stored in the next arg when this is false. 
static inline bool _ValueInNextArg(ParamArg *arg)
{
	if(arg->atype!=ParamArg::Option ||  // NOT ParamArg::Assignment
	   arg->origin.otype!=ParamArg::FromCmdLine ||
	   !arg->next)
	{  return(false);  }
	return(arg->next->arg.str() ? true : false);
}


static inline void _val2str_ensure_len(char **dest,size_t *len,size_t minlen)
{
	if((*len)>=minlen)  return;
	*dest=ValueHandler::stralloc(minlen);
	*len=minlen;
}


// NON-inline!!
static PAR::ParParseState _check_trim_val(const char **a,ParamArg *arg)
{
	*a=arg->value;
	if(_ValueInNextArg(arg))
	{
		ParamArg *nx=arg->next;  // not NULL here 
		bool okay=nx->atype!=ParamArg::Assignment && 
		          nx->atype!=ParamArg::Filename;
		if(okay)
		{
			const char *v=nx->arg;  // not NULL here 
			if(*v=='-')  ++v;
			if(*v=='.')  ++v;
			if(!isdigit(*v))  okay=0;
		}
		if(okay)
		{
			*a=nx->arg;
			nx->pdone=1;
		}
	}
	if(!(*a))  return(PAR::PPSValOmitted);
	while(is_trim(**a))  ++(*a);
	if(!(**a))  return(PAR::PPSValOmitted);
	return(PAR::PPSSuccess);
}

// NON-inline!!
static PAR::ParParseState _check_arg_end(PAR::ParParseState rv,
	const char *endptr,ParamArg *arg)
{
	// MAY NOT RETURN >0 UNLESS YOU CHANGE _simple_parse<>(). 
	if(rv)  return(rv);
	
	while(is_trim(*endptr))  ++endptr;
	if(arg->origin.otype==ParamArg::FromFile)
	{
		if(*endptr && *endptr!='#')
		{  rv=PAR::PPSGarbageEnd;  }
	}
	else if(*endptr)
	{  rv=PAR::PPSGarbageEnd;  }
	
	return(rv);
}


// These are inline because they get inserted only once. 
static inline PAR::ParParseState templ_str2val(int *val,
	const char *arg,char **endptr)
{
	errno=0;
	*val=strtol(arg,endptr,/*base=*/0);
	if(errno==ERANGE || *val<long(INT_MIN) || *val>long(INT_MAX))
	{  return(PAR::PPSValOORange);  }
	return(PAR::PPSSuccess);
}

static inline PAR::ParParseState templ_str2val(unsigned int *val,
	const char *arg,char **endptr)
{
	errno=0;
	*val=strtoul(arg,endptr,/*base=*/0);
	if(errno==ERANGE || *val>ulong(UINT_MAX))
	{  return(PAR::PPSValOORange);  }
	return(PAR::PPSSuccess);
}

static inline PAR::ParParseState templ_str2val(long *val,
	const char *arg,char **endptr)
{
	errno=0;
	*val=strtol(arg,endptr,/*base=*/0);
	if(errno==ERANGE)
	{  return(PAR::PPSValOORange);  }
	return(PAR::PPSSuccess);
}

static inline PAR::ParParseState templ_str2val(unsigned long *val,
	const char *arg,char **endptr)
{
	errno=0;
	*val=strtoul(arg,endptr,/*base=*/0);
	if(errno==ERANGE)
	{  return(PAR::PPSValOORange);  }
	return(PAR::PPSSuccess);
}

static inline PAR::ParParseState templ_str2val(double *val,
	const char *arg,char **endptr)
{
	errno=0;
	*val=strtod(arg,endptr);
	if(errno==ERANGE)
	{  return(PAR::PPSValOORange);  }
	return(PAR::PPSSuccess);
}


namespace internal
{

char *templ_val2str(long val,char *dest,size_t len)
{
	_val2str_ensure_len(&dest,&len,(sizeof(val)*5+1)/2 + 2);
	if(dest)
	{  snprintf(dest,len,"%ld",val);  }
	return(dest);
}

char *templ_val2str(int val,char *dest,size_t len)
{  return(templ_val2str(long(val),dest,len));  }

char *templ_val2str(unsigned long val,char *dest,size_t len)
{
	_val2str_ensure_len(&dest,&len,(sizeof(val)*5+1)/2 + 1);
	if(dest)
	{  snprintf(dest,len,"%lu",val);  }
	return(dest);
}

char *templ_val2str(unsigned int val,char *dest,size_t len)
{  return(templ_val2str((unsigned long)(val),dest,len));  }

char *templ_val2str(double val,char *dest,size_t len)
{
	_val2str_ensure_len(&dest,&len,24);
	if(dest)
	{  snprintf(dest,len,"%g",val);  }
	return(dest);
}

char *templ_val2str(bool val,char *dest,size_t len)
{
	_val2str_ensure_len(&dest,&len,4);  // 4 bytes: "yes\0"
	if(dest)
	{  strcpy(dest,val ? "yes" : "no");  }
	return(dest);
}


// Specialisation of the function above for type bool (used for switch args): 
PAR::ParParseState simple_parse(PAR::ParamInfo *,bool *valptr,ParamArg *arg)
{
	int val=-1;
	PAR::ParParseState rv=PAR::PPSSuccess;
	
	const char *value=arg->value;
	if(_ValueInNextArg(arg))
	{
		ParamArg *nx=arg->next;  // not NULL here 
		if(nx->atype==ParamArg::Unknown)
		{
			value=arg->next->arg;
			arg->next->pdone=1;   // Parser will set that to -1 on error. 
		}
	}
	
	if(value)
	{
		// skip whitespace
		const char *p=value;
		while(is_trim(*p))  ++p;
		if(!(*p) || (arg->origin.otype==ParamArg::FromFile && *p=='#'))
		{  return(PAR::PPSValOmitted);  }
		
		const char *end=p;
		while(isalpha(*end) || isdigit(*end))  ++end;
		if(end-p>5)  return(PAR::PPSIllegalArg);
		char tmpp[6];   // `false\0'
		strncpy(tmpp,p,end-p);
		tmpp[end-p]='\0';
		
		if(!strcasecmp(tmpp,"yes")  ||
		   !strcasecmp(tmpp,"on")   ||
		   !strcasecmp(tmpp,"true") ||
		   !strcasecmp(tmpp,"1")	 )
		{  val=1;  }
		else if(!strcasecmp(tmpp,"no") ||
		   !strcasecmp(tmpp,"off")   ||
		   !strcasecmp(tmpp,"false") ||
		   !strcasecmp(tmpp,"0")	 )
		{  val=0;  }
		else 
		{  return(PAR::PPSIllegalArg);  }
		
		rv=_check_arg_end(rv,end,arg);
		// rv<=0 here. 
	}
	
	if(val<0)  val=1;
	
	if(!strncmp(arg->name,"no-",3))
	{  val=!val;  }
	
	*(bool*)(valptr)=(val ? true : false);
	return(rv);
}

// This is inline because it gets called once per type: 
template<class T> PAR::ParParseState simple_parse(
	PAR::ParamInfo *,T *valptr,ParamArg *arg)
{
	const char *a;
	PAR::ParParseState rv=_check_trim_val(&a,arg);
	if(rv>0)  return(rv);
	
	T val;
	char *endptr;
	rv=templ_str2val(&val,a,&endptr);
	if(endptr==a)  rv=PAR::PPSIllegalArg;
	if(rv>0)  return(rv);   // error
	
	rv=_check_arg_end(rv,endptr,arg);
	// rv<=0 here. 
	
	// other modes than `=' currently unsupported. 
	if(arg->assmode!='\0' && rv>=0)
	{  rv=PAR::PPSIllegalAssMode;  }
	
	// Store it: 
	*(T*)(valptr)=val;
	return(rv);
}

}  // end of namespace internal 


// Make them available: 
static SimpleValueHandler<int> static_int_handler;
static SimpleValueHandler<unsigned int> static_uint_handler;
static SimpleValueHandler<long> static_long_handler;
static SimpleValueHandler<unsigned long> static_ulong_handler;
static SimpleValueHandler<double> static_double_handler;

ValueHandler *int_handler=&static_int_handler;
ValueHandler *uint_handler=&static_uint_handler;
ValueHandler *long_handler=&static_long_handler;
ValueHandler *ulong_handler=&static_ulong_handler;
ValueHandler *double_handler=&static_double_handler;

static SimpleValueHandler<bool> static_switch_handler;
ValueHandler *switch_handler=&static_switch_handler;

static OptionValueHandler static_option_handler;
ValueHandler *option_handler=&static_option_handler;


char *OptionValueHandler::print(PAR::ParamInfo *,void *_val,char *dest,size_t len)
{
	int val=*(int*)_val;
	_val2str_ensure_len(&dest,&len,24);  // " times specified\0" -> 17 bytes
	if(dest)
	{
		static const char *spec="specified";
		switch(val)
		{
			case 0:  snprintf(dest,len,"not %s",spec);  break;
			case 1:  snprintf(dest,len,"%s once",spec);  break;
			default:  snprintf(dest,len,"%d times %s",val,spec);  break;
		}
	}
	return(dest);
}


}  // namespace end 
