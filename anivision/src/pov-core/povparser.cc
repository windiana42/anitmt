/*
 * pov-core/povparser.cc
 * 
 * POV command comment parser. 
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

#include "povparser.h"
#include <ani-parser/ani_parser.h>


namespace POV
{

CommandParser::CCommand::CCommand(const RefString &_text,
	const SourcePosition &_startpos,const SourcePosition &_endpos,int _fs) : 
	startpos(_startpos),
	endpos(_endpos),
	text(_text),
	file_seq(_fs)
{
	ptn_root=NULL;
}

CommandParser::CCommand::~CCommand()
{
	if(ptn_root)
	{
		ANI::TNLocation loc(startpos,endpos);
		fprintf(stderr,"POV:   Deleting tree @%s\n",
			loc.PosRangeString().str());
		ptn_root->DeleteTree();
		ptn_root=NULL;
	}
}


/******************************************************************************/

int CommandParser::_Register_ObjectSpec(PTNObjectSpec *ospec)
{
	int errors=0;
	
	ObjectSpec *os=new ObjectSpec(ospec);
	
	// Check components: 
	int seen_component[PTNObjectEntry::_AS_LAST];
	for(int i=0; i<PTNObjectEntry::_AS_LAST; i++)
	{  seen_component[i]=0;  }
	for(ANI::TreeNode *_oent=ospec->down.first(); _oent; _oent=_oent->next)
	{
		PTNObjectEntry *oent=(PTNObjectEntry*)_oent;
		assert(oent->PNType()==PTN_ObjectEntry);
		
		// Check for repetitions: 
		PTNObjectEntry::ASymbol symb=oent->GetSymbol();
		++seen_component[symb];
		if(seen_component[symb]>1)
		{
			if(seen_component[symb]==2)
			{
				Error(oent->GetLocationRange(),
					"more than one \"%s\" component in object spec\n",
					PTNObjectEntry::ASymbolStr(symb));
				++errors;
			}
			continue;
		}
		
		// Only first occurance. 
		switch(symb)
		{
			case PTNObjectEntry::AS_None:  // may not happen
				assert(0);
				break;
			case PTNObjectEntry::AS_Macro:
			case PTNObjectEntry::AS_Declare:
			{
				ObjectSpec::ObjectType ot=(symb==PTNObjectEntry::AS_Macro ? 
					ObjectSpec::OT_Macro : ObjectSpec::OT_Declare);
				// Okay... a macro. 
				if(os->otype!=ObjectSpec::OT_None)
				{
					Error(oent->GetLocationRange(),
						"cannot use \"%s\" together with \"%s\"\n",
						ObjectSpec::ObjectTypeStr(ot),
						ObjectSpec::ObjectTypeStr(os->otype));
					++errors;  break;
				}
				os->otype=ot;
				ANI::TNIdentifier *idf=(ANI::TNIdentifier*)oent->down.first();
				assert(idf->NType()==ANI::TN_Identifier);
				os->oname=idf->CompleteStr();
			}	break;
			case PTNObjectEntry::AS_Params:     break;
			case PTNObjectEntry::AS_Defs:       break;
			case PTNObjectEntry::AS_Include:    break;
			case PTNObjectEntry::AS_Type:       break;
			case PTNObjectEntry::AS_AppendRaw:  break;
			default:  assert(0);  // Forgot to implement?
		}
	}
	
	do {
		if(errors)  break;
		
		if(os->otype==ObjectSpec::OT_None || !os->oname)
		{
			Error(ospec->GetLocationRange(),
				"object spec lacks a name "
				"(i.e. \"macro\" or \"declare\" component)\n");
			++errors;  break;
		}
		if(os->otype==ObjectSpec::OT_Declare && 
			seen_component[PTNObjectEntry::AS_Params])
		{
			Error(ospec->GetLocationRange(),
				"object \"%s\": cannot pass \"params\" to "
				"\"declare\"d object\n",
				os->oname.str());
			++errors;  break;
		}
		
		// See if we already have that in the list: 
		ObjectSpec *i=FindObject(os->oname);
		if(i)
		{
			ANI::TNLocation loc=os->ospec->GetLocationRange();
			Error(loc,
				"object named \"%s\" previously defined @%s\n",
				os->oname.str(),
				i->ospec->GetLocationRange().PosRangeStringRelative(loc).str());
			++errors;  break;
		}
	} while(0);
	
	if(errors)
	{  DELETE(os);  }
	else
	{
		oslist.append(os);
		
		fprintf(stderr,"POV:   Object spec \"%s\" (%s) registered.\n",
			os->oname.str(),ObjectSpec::ObjectTypeStr(os->otype));
	}
	
	return(errors);
}


int CommandParser::_Register_GeneralCmd(PTNGeneralCmd *tn)
{
	int errors=0;
	
	switch(tn->pcmd)
	{
		case PTNGeneralCmd::PC_None:  assert(0);  break;  // may not happen...
		case PTNGeneralCmd::PC_FileID:
			// This should have been dealt with before. 
			// Ignore it (should not happen anyways). 
			break;
		default:  assert(0);  // Forgot to implement?!
	}
	
	return(errors);
}


int CommandParser::_RegisterCCommand(CommandParser::CCommand *cmd)
{
	int errors=0;
	
	// Loop over the top nodes: 
	assert(cmd->ptn_root);
	for(ANI::TreeNode *tn=cmd->ptn_root->down.first(); tn; tn=tn->next)
	{
		switch(((POVTreeNode*)tn)->PNType())
		{
			case PTN_ObjectSpec:
				errors+=_Register_ObjectSpec((PTNObjectSpec*)tn);
				break;
			case PTN_GeneralCmd:
				errors+=_Register_GeneralCmd((PTNGeneralCmd*)tn);
				break;
			default: assert(0);  // Grammar should forbid that. 
		}
	}
	
	return(errors);
}


int CommandParser::ParseFile(const RefString &file,ANI::Ani_Parser *ap_to_use)
{
	errors=0;
	passed_ani_parser=ap_to_use;
	int rv=POVLexerScanner_Interface::ScanFile(file);
	passed_ani_parser=NULL;
	errors+=rv;
	return(errors);
}


int CommandParser::CommandNotify(const RefString &text,
	SourcePosition startpos,SourcePosition endpos,int file_seq)
{
	CCommand *cmd=new CCommand(text,startpos,endpos,file_seq);
	
	// We need to parse it immediately to be able to analyse 
	// fileID in case there is one. 
	ANI::Ani_Parser *ap=passed_ani_parser;
	if(!ap)
	{
		if(!ani_parser)
		{  ani_parser=new ANI::Ani_Parser();  }
		ap=ani_parser;
	}
	int rv=ap->ParsePOVCommand(cmd->text,cmd->startpos,&cmd->ptn_root);
	int retval=0;
	if(rv)
	{  ++errors;  }
	else if(cmd->ptn_root)  // See if there was a fileID... 
	{
		for(ANI::TreeNode *_tn=cmd->ptn_root->down.first(); _tn; )
		{
			POVTreeNode *ptn=(POVTreeNode*)_tn;
			_tn=_tn->next;
			if(ptn->PNType()!=PTN_GeneralCmd)  continue;
			PTNGeneralCmd *gcmd=(PTNGeneralCmd*)ptn;
			if(gcmd->pcmd!=PTNGeneralCmd::PC_FileID)  continue;
			
			RefString fileid;
			fileid=((ANI::TNIdentifier*)gcmd->down.first())->CompleteStr();
			// See if we already know this file ID: 
			if(!fileid_list.find(fileid))
			{  fileid_list.append(fileid);  }
			else
			{  retval=1;  }
			
			fprintf(stderr,"POV:   Found %s @fileID \"%s\" at %s.\n",
				retval ? "known" : "new",
				fileid.str(),
				gcmd->GetLocationRange().PosRangeString().str());
			
			// Delete FileID node: 
			delete cmd->ptn_root->RemoveChild(ptn);
			
			if(retval)
			{
				// So, we found known FileID. Delete all nodes after it: 
				for(;;)
				{
					ANI::TreeNode *tmp=_tn;
					if(!tmp)  break;
					_tn=_tn->next;
					delete cmd->ptn_root->RemoveChild(tmp);
				}
			}
			
			// If no nodes are left, delete the head as well: 
			if(cmd->ptn_root->down.is_empty())
			{
				// This is a _head_ node, hence we use DeleteTree(). 
				cmd->ptn_root->DeleteTree();
				cmd->ptn_root=NULL;
				break;
			}
				
			if(retval) break;
		}
	}
	
	// Nodes without tree are uninteresting. 
	if(!cmd->ptn_root)
	{  DELETE(cmd);  cmd=NULL;  }
	
	if(cmd)
	{
		// Queue commands: 
		if(cmd->ptn_root)
		{  cmdlist.append(cmd);  }
		
		// Okay, do further processing. (Must already be queued.) 
		errors+=_RegisterCCommand(cmd);
	}
	
	// Return: 0 -> go on; 1 -> skip file; 2 -> stop reading. 
	return(retval);
}


ObjectSpec *CommandParser::FindObject(const RefString &name)
{
	#warning "Could speed up using hash..."
	// See if we already have that in the list: 
	for(ObjectSpec *i=oslist.first(); i; i=i->next)
	{
		if(i->oname==name)
		{  return(i);  }
	}
	return(NULL);
}


void CommandParser::DumpContent(FILE *out)
{
	fprintf(out,"Known file IDs: ");
	for(const RefStrList::Node *n=fileid_list.first(); n; n=n->next)
	{  fprintf(out,"%s%s",n->str(),n->next ? "," : "");  }
	fprintf(out,"\n");
	
	for(CCommand *c=cmdlist.first(); c; c=c->next)
	{
		RefString tmp;
		int rv=c->startpos.PosRangeString(c->endpos,&tmp);
		assert(!rv);
		fprintf(out,
			"----------<%s>----------\n"
			"%.*s\n",
			tmp.str(),
			c->text.len(),c->text.str());
		
		if(c->ptn_root)
		{
			fprintf(out,"---------------<TREE>---------------\n");
			c->ptn_root->DumpTree(out);
		}
	}
	
	fprintf(out,"Object list: %d entries\n",oslist.count());
	for(ObjectSpec *os=oslist.first(); os; os=os->next)
	{
		fprintf(out,
			"  %s (%s)\n",
			os->oname.str(),
			ObjectSpec::ObjectTypeStr(os->otype));
	}
}


CommandParser::CommandParser() : 
	POVLexerScanner_Interface(),
	cmdlist(),
	oslist(),
	fileid_list()
{
	ani_parser=NULL;
	passed_ani_parser=NULL;
	errors=0;
}

CommandParser::~CommandParser()
{
	while(!oslist.is_empty())
	{  delete oslist.popfirst();  }
	
	fprintf(stderr,"POV: Deleting all %d POV trees...\n",cmdlist.count());
	while(!cmdlist.is_empty())
	{  delete cmdlist.poplast();  }
	fprintf(stderr,"POV:   POV trees deleted: "
		"%u bytes for %d tree nodes still allocated.\n",
		ANI::TreeNode::tree_nodes_tot_size,ANI::TreeNode::n_alloc_tree_nodes);
	
	DELETE(ani_parser);
}

}  // end of namespace POV
