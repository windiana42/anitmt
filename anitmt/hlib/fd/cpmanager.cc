class FDCopyBase;

class FDCopyManager : FDBase
{
	public:
		static FDCopyManager *manager;
		
		struct _CopyID {};
		typedef _CopyID (*CopyID);
		typedef off_t copylen_t;
		
		struct CopyRequest
		{
			//*** Source and destination: ***
			// Note: You must specify ONE data source and ONE data 
			//       destination. Thus, you may not specify srcfd AND 
			//       srcbuf / destfd AND destbuf. (fds are -1 for 
			//       unset, pointers NULL). Note also that you MUST 
			//       supply at least one file descriptor (thus, the 
			//       FDCopyManager cannot be used to copy large 
			//       buffers in memory).
			int srcfd;       // fd to write data to
			int destfd;      // fd to read data from 
			char *srcbuf;    // where to read data from 
			char *destbuf;   // where to store read in data
			copylen_t len;   // How many bytes to copy max; 0 -> unlimited
			
			//*** Time restrictions: ***
			long timeout;  // Timeout for the complete request in msec; 
			               // -1 -> no timeout
			
			//*** More tuning: ***
			size_t iobufsize;  // Size of IO buffer allocated for the copy job 
			// This needs some explanation: The filedescriptor is 
			// polled for reading, if less than low_read_thresh bytes are 
			// in the buffer and is no longer polled for reading if more 
			// than high_read_thresh is in the buffer. 
			// The dest fd is polled for writing if more than 
			// high_write_thresh bytes are in the buffer and not polled 
			// for writing if less than low_write_thresh in the buffer. 
			// You may count on reasonable defaults; 
			// values of -1 mean -> set defaults according to iobufsize. 
			ssize_t low_read_thresh;
			ssize_t high_read_thresh;
			ssize_t low_write_thresh;
			ssize_t high_write_thresh;
			// Max. number of bytes passed to a call to read / write. 
			// (One call is done per fdnotify()). 
			// Set to 0 for `unlimited'. 
			ssize_t max_read_len;
			ssize_t max_write_len;
			
			// Error code in case CopyFD() returns NULL. 
			// Values: 
			//   0 -> success (no error)
			//  -1 -> allocation failure
			//  -2 -> call to PollFD() failed. 
			//  -3 -> illegal src/dest spec 
			int errcode;
			
			_CPP_OPERATORS_FF
			CopyRequest(int *failflag=NULL);
			~CopyRequest();
		};
		
		struct MCopyNode
		{
			FDCopyBase *client;  // client back pointer
			
			// The complete copy request:
			CopyRequest req;
			
			// If FDBase pointer was set: 
			FDBase *fdb;  // FDBase pointer
			enum { OFSrcSet=0x1,OFDestSet=0x2 };
			int orig_flags;  // OR of OFSrcSet, OFDestSet if following is set: 
			short orig_src_events;   // save original fd events 
			short orig_dest_events;  // used by FDBase
			
			// PollID of the source and dest fds. If src or dest is 
			// a buffer, the corresponding PollID is NULL. 
			PollID psrcid,pdestid;
			
			// Actual copy buffer: 
			char *buf;   // size: req.iobufsize
			// buf end: 
			// buf!=NULL -> end of buf
			// buf==NULL -> end of req.srcbuf or req.destbuf
			char *bufend;
			// Head of buffer: 
			// buf!=NULL -> current head of cyclic io buffer
			// buf==NULL -> current read/write position in src/dest buf
			char *bufhead;
			// Number of valid bytes after bufhead (possibly wrapping 
			// around at bufend), or 0 if buf=NULL. 
			size_t bufuse;
			
			_CPP_OPERATORS_FF
			MCopyNode(int *failflag=NULL);
			~MCopyNode();
		};
	private:
		// List of all FDCopyBase classes: 
		LinkedList<FDCopyBase> clients:
		
		int _SavePollEvents(MCopyNode *cpn);
		int _RestorePollEvents(MCopyNode *cpn);
		
		// Overriding virtuals: 
		int fdnotify(FDInfo *);
		
	public:  _CPP_OERPATORS_FF
		FDCopyManager(int *failflag=NULL);
		~FDCopyManager();
		
		// Default values: 
		size_t default_iobufsize;   // default for CopyRequest::iobufsize
		
		// Called by FDCopyBase when constructed:
		// Returns 0 on sucess; !=0 on error. 
		int Register(FDCopyBase *cb);
		
		// Called by FDCopyBase when it gets destroyed: 
		void Unregister(FDCopyBase *cb);
		
		// This is actally the routine for which all the 
		// FDCopyBase / FDCopyManager exist...
		// Tell FDCopyManager to copy something:
		// Returns CopyID of this copy request or NULL. 
		// Check req->errcode for errors.
		// (CopyRequest is copied; just pass pointer to object 
		// on stack.) 
		// Refer to FDCopyBase::CopyFD() for more information. 
		// (fdb may be NULL if not derived from FDBase.)
		CopyID CopyFD(FDCopyBase *client,CopyRequest *req,FDBase *fdb);
};

----------------------<snip>---------------------------

#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on. 
#warning Using assert()
#include <assert.h>
#else
#define assert(x)
#endif


// Static global manager: 
static FDCopyManager *FDCopyManager::manager=NULL;


static ssize_t fdwrite(int fd,const char *buf,size_t len)
{
	ssize_t rv;
	do
	{  rv=write(fd,buf,len);  }
	while(rv<0 && errno==EINTR);
	return(rv);
}

static ssize_t fdread(int fd,char *buf,size_t len)
{
	ssize_t rv;
	do
	{  rv=read(fd,buf,len);  }
	while(rv<0 && errno==EINTR);
	return(rv);
}


int FDCopyManager::fdnotify(FDInfo *fdi)
{
	// Get associated copy node: 
	MCopyNode *cpn=(MCopyNode *)fdi->dptr;
	assert(cpn);
	
	if(fdi->fd==cpn->req.srcfd)
	{
		assert(fdi->pollid==cpn->psrcid);
		// Can read data...
		
		if(cpn->req.destbuf)
		{
			// Read data from fd and write to buffer. Easy case. 
			size_t need=cpn->bufend-cpn->bufhead;
			if(need>cpn->req.max_read_len)
			{  need=cpn->req.max_read_len;  }
			
			ssize_t rd=fdread(cpn->req.srcfd,cpn->bufhead,need);
			if(rd>0)
			{
				cpn->bufhead+=rd;
				if(cpn->bufhead>=cpn->bufend)
				{  done_calldown();  }
			}
			else if(rd==0)
			{
				eof_calldown();
			}
			else
			{
				#error wr==0, wr<0 ??
				error_calldown();
			}
		}
		
	}
	else if(fdi->fd==cpn->req.destfd)
	{
		assert(fdi->pollid==cpn->pdestid);
		// Can write data...
		
		if(cpn->req.srcbuf)
		{
			// Read data from buffer and write to fd. Easy case. 
			size_t avail=cpn->bufend-cpn->bufhead;
			if(avail>cpn->req.max_write_len)
			{  avail=cpn->req.max_write_len;  }
			
			ssize_t wr=fdwrite(cpn->req.destfd,cpn->bufhead,avail);
			if(wr>0)
			{
				cpn->bufhead+=wr;
				if(cpn->bufhead>=cpn->bufend)
				{  done_calldown();  }
			}
			else
			{
				#error wr==0, wr<0 ??
			}
		}
		
	}
	else
	{  assert(0);  }
	
	return(0);
}


// Save ("original") poll events (client side) on passed fds and 
// set them to 0. 
int FDCopyManager::_SavePollEvents(MCopyNode *cpn)
{
	FDBase *fdb=cpn->fdb;
	if(!fdb)  retturn(0);
	
	PollID pollid=fdb->FDPollID(cpn->req.srcfd);
	if(pollid)
	{
		cpn->orig_src_events=fdb->FDEvents(pollid);
		cpn->orig_flags|=MCopyNode::OFSrcSet;
		fdb->PollFD(pollid,0);
	}
	PollID pollid=fdb->FDPollID(cpn->req.destfd);
	if(pollid)
	{
		cpn->orig_dest_events=fdb->FDEvents(pollid);
		cpn->orig_flags|=MCopyNode::OFDestSet;
		fdb->PollFD(pollid,0);
	}
	return(0);
}

// Restore the saved poll events:
int FDCopyManager::_RestorePollEvents(MCopyNode *cpn)
{
	FDBase *fdb=cpn->fdb;
	if(!fdb)  retturn(0);
	
	if(cpn->orig_flags & MCopyNode::OFSrcSet)
	{
		// First check for the PollID because the poll node might 
		// no longer exist; in this case we should not allocate it. 
		PollID pollid=fdb->FDPollID(cpn->req.srcfd);
		if(pollid)
		{  fdb->PollFD(pollid,cpn->orig_src_events);  }
	}
	if(cpn->orig_flags & MCopyNode::OFDestSet)
	{
		PollID pollid=fdb->FDPollID(cpn->req.destfd);
		if(pollid)
		{  fdb->PollFD(pollid,cpn->orig_dest_events);  }
	}
	
	return(0);
}


FDCopyManager::CopyID FDCopyManager::CopyFD(FDCopyBase *client,
	CopyRequest *req,FDBase *fdb)
{
	req->errcode=0;
	
	#if TESTING
	assert(clients.find(client));
	#endif
	
	// Check input: 
	if(((req->srcfd<0)==(!req->srcbuf)) || 
	   ((req->destfd<0)==(!req->destbuf)) || 
	   (req->srcbuf && req->destbuf))
	{
		req->errcode=-3;
		return(NULL);
	}
	
	// This may give a warning if req->len is an unsigned type: 
	if(req->len<0)
	{  req->len=0;  }
	
	if(!req->iobufsize)
	{
		// Set default io buf size: 
		req->iobufsize=default_ioufsize;
	}
	
	#warning A more clever algorithm should be invented here: 
	ssize_t iobs=req->iobufsize;
	if(req->low_read_thresh<0 || low_read_thresh>=iobs)
	{  req->low_read_thresh=iobs/8;  }
	if(req->high_read_thresh<0 || req->high_read_thresh>=iobs)
	{  req->low_read_thresh=iobs-iobs/8;  }
	if(req->high_read_thresh<=req->low_read_thresh)
	{
		#error ...
	}
	
	if(req->low_write_thresh<0 || low_write_thresh>=iobs)
	{  req->low_write_thresh=iobs/8;  }
	if(req->high_write_thresh<0 || req->high_write_thresh>=iobs)
	{  req->low_write_thresh=iobs-iobs/8;  }
	if(req->high_write_thresh<=req->low_write_thresh)
	{
		#error ...
	}
	
	// Last chance to quit without much effort...
	if(req->errcode)
	{  return(NULL);  }
	
	// Okay, allocate a copy node: 
	MCopyNode *cpn=NEW<MCopyNode>();
	if(!cpn)  // allocation failure
	{  req->errcode=-1;  return(NULL);  }
	
	// Basic setup: 
	cpn->client=client;  // set back pointer
	cpn->fdb=fdb;
	cpn->req=*req;  // implicit copy 
	
	// Allocate our FDNodes at the FDManager: 
	if(req->srcfd>=0)
	{
		int rv=PollFD(req->srcfd,0,cpn,&cpn->psrcid);
		if(rv!=0)
		{  req->errcode=-2;  goto retunpoll;  }
	}
	if(req->destfd>=0)
	{
		int rv=PollFD(req->destfd,0,cpn,&cpn->pdestid);
		if(rv!=0)
		{  req->errcode=-2;  goto retunpoll;  }
	}
	
	// Allocate copy buffer if needed: 
	if(cpn->psrcid && cpn->pdestid)
	{
		// If iobufsize is 0, set up default: 
		if(!cpn->req.iobufsize)
		{  cpn->req.iobufsize=default_iobufsize;  }
		// If we shall copy less bytes then iobufsize, make buffer smaller: 
		if(cpn->req.len && cpn->req.len<copylen_t(cpn->req.iobufsize))
		{  cpn->req.iobufsize=size_t(cpn->req.len);  }
		// Actually allocate buffer: 
		cpn->buf=(char*)LMalloc(cpn->req.iobufsize);
		if(!cpn->buf)
		{  req->errcode=-1;  goto retfreebuf;  }
		cpn->bufend=cpn->buf+cpn->req.iobufsize;
		cpn->bufhead=cpn->buf;
		cpn->bufuse=0;
	}
	else
	{
		// Set buffer vars (src or dest (not both) is passd as argument 
		// to CopyFD()). 
		cpn->bufhead = (cpn->pdestbuf) ? cpn->destbuf : cpn->srcbuf;
		cpn->bufend = cpn->bufhead + cpn->bufuse;
		// bufuse must stay 0 as buf=NULL. 
	}
	
	// Set poll events of FDs to 0 if FDBase pointer is set: 
	_SavePollEvents(cpn);
	
	// Actually start copy process: 
	#error missing.
	
	return((CopyID)cpn);
	
retfreebuf:;
	cpn->buf=(char*)LFree(cpn->buf);
retunpoll:;
	if(cpn->psrcid)   UnpollFD(cpn->psrcid);
	if(cpn->pdestid)  UnpollFD(cpn->pdestid);
retfree:;
	if(cpn)  delete cpn;
	return(NULL);
}


int FDCopyManager::Register(FDCopyBase *client)
{
	if(client)
	{
		if(client.next || client==clients.last())
		{  return(1);  }   // already registered
		clients.append(client);
	}
	return(0);
}

void FDCopyManager::Unregister(FDCopyBase *client)
{
	if(!client)  return;
	
	// Client registered?
	if(client.next || client==clients.last())
	{
		// Remove client from list: 
		clients.dequeue(client);
	}
}


FDCopyManager::FDCopyManger(int *failflag=NULL) : 
	FDBase(failflag),
	clients(failflag)
{
	int failed=0;
	
	// Tell FDManager that we're a manager: 
	if(SetManager(1))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  return;  }
	else if(failed)
	{  ConstructorFailedExit("CP");  }
	
	/*--- DO NOT USE >int failed< BELOW HERE. ---*/
	
	// Set defaults: 
	default_iobufsize=16384;
	
	// Init global manager: 
	#if TESTING
	if(manager)
	{  fprintf(stderr,"%s: more than one FDCopyManager.\n",
		prg_name);  exit(1);  }
	#endif
	
	manager=this;
}

FDCopyManager::~FDCopyManger()
{
	#if TESTING
	if(!clients.is_empty())
	{  fprintf(stderr,"CPMan: Oops: client list not empty: %d clients left\n",
		clients.count());  }
	#endif
	
	manager=NULL;
}


/******************************************************************************/

FDCopyManager::MCopyNode::MCopyNode(int *failflag) : 
	req(failflag)
{
	client=NULL;
	
	psrcid=NULL;
	pdestid=NULL;
	
	fdb=NULL;
	orig_flags=0;
	orig_src_events=0;
	orig_dest_events=0;
	
	buf=NULL;
	bufend=NULL;
	bufhead=NULL;
	bufuse=0;
}

FDCopyManager::MCopyNode::~MCopyNode()
{
	// Clear only the most important thigs 
	// (make bug tracking easier...)
	client=NULL;
	fdb=NULL;
	psrcid=NULL;
	pdestid=NULL;
	
	buf=(char*)LFree(buf);
	bufend=NULL;
	bufhead=NULL;
	bufuse=0;
}
