#include <stdio.h>

#include <hlib/refstring.h>

#include "ldrproto.hpp"
#include "prototypes.hpp"

char *prg_name="test_ldrproto";

int main()
{
	fprintf(stderr,"--------------<Running LDR packet size tests>------------\n");
	
	char *args[]={NULL,"--nocolor",NULL};
	int argc=2;
	InitRVOutputParams(argc,args,NULL);
	
	int rv=LDR::LDRCheckCorrectLDRPaketSizes(1);
	fprintf(stderr,"----------------------<TESTS %s>---------------------\n",
		rv ? "FAILED" : "PASSED");
	
	return(rv ? 1 : 0);
}
