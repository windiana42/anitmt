	#if 1
	fprintf(stderr,"DUMP(length=%u)>>",sizeof(LDRHeader));
	for(char *c=(char*)hdr,*cend=c+sizeof(LDRHeader); c<cend; c++)
	{
		if(*(unsigned char*)c>=32 && *(unsigned char*)c!=127)
		{  write(2,c,1);  }
		else
		{  write(2,".",1);  }
	}
	fprintf(stderr,"<<\n");
	fprintf(stderr,"HEX>>");
	int ii=0;
	for(char *c=(char*)hdr,*cend=c+sizeof(LDRHeader); c<cend; c++)
	{
		int x=*(unsigned char*)c;
		fprintf(stderr,"%02x",x);
		if(!(ii%4))  fprintf(stderr," ");
	}
	fprintf(stderr,"<<\n");
	char buf[256];
	errno=0;
	ssize_t rd=read(sock_fd,buf,256);
	fprintf(stderr,"Reading further bytes: rd=%d; errno=%s\n",rd,strerror(errno));
	fprintf(stderr,"DUMP(length=%d)>>",rd);
	for(char *c=buf,*end=buf+rd; c<end; c++)
	{
		if(*(unsigned char*)c>=32 && *(unsigned char*)c!=127)
		{  write(2,c,1);  }
		else
		{  write(2,".",1);  }
	}
	fprintf(stderr,"<<\n");
	fprintf(stderr,"HEX>>");
	ii=0;
	for(char *c=buf,*end=buf+rd; c<end; c++)
	{
		int x=*(unsigned char*)c;
		fprintf(stderr,"%02x",x);
		if(!(ii%4))  fprintf(stderr," ");
	}
	fprintf(stderr,"<<\n");
	#endif
