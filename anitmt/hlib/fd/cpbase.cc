class FDCopyBase : LinkedListBase<FDCopyBase>
{
	friend class FDCopyManager;
	public:
		typedef FDCopyManager::CopyInfo CopyInfo;
		typedef FDCopyManager::StatusCode StatusCode;
		typedef FDCopyManager::CopyID CopyID;
		typedef FDCopyManager::CopyRequest CopyRequest;
		typedef FDCopyManager::copylen_t copylen_t;
	private:
		typedef FDCopyManager::MCopyNode MCopyNode;
		// List of copy requests, managed by FDCopyManager: 
		LinkedList<struct MCopyNode> rlist;
	protected:
		// Called upon variuos conditions (completion of copy 
		// request / error condition): 
		// See struct CopyInfo (cpmanager.h) for details. 
		// Return value: currently ignored; use 0. 
		virtual int cpnotify(CopyInfo *ci)  {  return(0);  }
		
		// Called to notify client on progress made: 
		// Return value: currently ignored; use 0. 
		virtual int cpnotify(ProgressInfo *pi)  {  return(0);  }
	public:  _CPP_OPERATORS_FF
		FDCopyBase(int *failflag);
		~FDCopyBase();
		
		// Use this to get the FDCopyManager: 
		inline FDCopyManager *cpmanager()  {  return(FDCopyManager::manager);  }
		
		#error need \
			QueryProgress(CopyInfo *save_here)  \
			CopyControl(stop/cont/cancel)
		#error Was macht poll() bei SIGPIPE / EPIPE? - Kommt POLLERR??
		
		// Central routine: Give FDCopyManager a copy job: 
		// CopyRequest: stores all the needed information and tuning 
		//    parameters for the copy request. 
		//    NOTE: *req is modified: The iobufsize/thresholds are 
		//          set to proper values if you did not and the errval 
		//          is set. If all that went throuh without an error, 
		//          the CopyRequest structure is copied and stored in 
		//          a list maintained by FDCopyManager. 
		// FDBase: Pass a pointer to *this if your class is derived 
		//    from FDBase. This simply does a PollFD(srcfd/destfd,0) 
		//    to make sure that the fd is not polled by the calling 
		//    FDBase-derived class. Make sure that you do NOT poll 
		//    for the fd you are currently copying!
		CopyID CopyFD(CopyRequest *req,FDBase *fdb)
			{  return(cpmanager()->CopyFD(this,req,fdb));  }
};


FDCopyBase::FDCopyBase(int *failflag) :
	LinkedListBase<FDCopyBase>(),
	rlist(failflag)
{
	int failed=0;
	
	if(cpmanager()->Register(this))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  return;  }
	else if(failed)
	{  ConstructorFailedExit("CPC");  }
}

FDCopyBase::~FDCopyBase()
{
	#warning make sure rlist is empty.
	
	manager()->Unregister(this);
}


/******************************************************************************/

CopyRequest();
{
	srcfd=destfd=-1;
	srcbuf=destbuf=NULL;
	len=0;
	
	timeout=-1;
	
	dptr=NULL;
	
	iobufsize=FDCopyManager::default_iobufsize;
	low_read_thresh=-1;
	high_read_thresh=-1;
	low_write_thresh=-1;
	high_write_thresh=-1;
	
	max_read_len=0;
	max_write_len=0;
	
	errcode=0;
}

~CopyRquest()
{
	// Only cancel the most important fields...
	srcbuf=destbuf=NULL;
	len=0;
	dptr=NULL;
}
