/*
 * ani-parser/treedump.cc
 * 
 * Recursive complete programming language tree dump. 
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

#include "tree.h"

#include <stdio.h>


namespace ANI
{


void TreeNode::DumpTree(StringTreeDump *d)
{
	assert(ntype==TN_None);
	// TN_None type. Dump all nodes in down and put them into "NONE[...]". 
	
	d->Append("NONE[");
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		i->DumpTree(d);
		if(i->next)  d->Append(",");
	}
	d->Append("]");
	
	// And then abort because the TN_None node may not 
	// have children. 
	assert(0);
}


void TNIdentifierSimple::DumpTree(StringTreeDump *d)
{
	// !name is the case for the first node in "::a::b". 
	d->Append(!name ? "" : name.str());
	assert(down.is_empty());
}


void TNIdentifier::DumpTree(StringTreeDump *d)
{
	assert(!down.is_empty());
	for(TreeNode *i=down.last(); i; i=i->prev)
	{
		i->DumpTree(d);
		if(i->prev)  d->Append("::");
	}
}

RefString TNIdentifier::CompleteStr()
{
	if(down.first()==down.last())
	{
		TNIdentifierSimple *chld=(TNIdentifierSimple*)down.first();
		assert(chld->NType()==TN_IdentifierSimple);
		return(chld->name);
	}
	else  // This should be faster for more than 1 child. 
	{
		size_t len=0;
		for(TreeNode *i=down.last(); i; i=i->prev)
		{
			assert(i->NType()==TN_IdentifierSimple);
			len+=((TNIdentifierSimple*)i)->name.len();
			if(i->prev) len+=2;
		}
		String tmp(len+1);
		for(TreeNode *i=down.last(); i; i=i->prev)
		{
			TNIdentifierSimple *id=(TNIdentifierSimple*)i;
			if(id->name.str())
			{  tmp+=id->name.str();  }
			if(i->prev)  tmp+="::";
		}
		RefString rv;
		rv.set(tmp.str());
		return(rv);
	}
}


void TNExpression::DumpTree(StringTreeDump *d)
{
	// In case there's a derived class, we may not be here. 
	assert(expr_type==TNE_Expression);
	// May only have one child. 
	assert(down.first()==down.last());
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->DumpTree(d);  }
}


void TNExpressionList::DumpTree(StringTreeDump *d)
{
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		i->DumpTree(d);
		if(i->next)  d->Append(", ");
	}
}


void TNValue::DumpTree(StringTreeDump *d)
{
	String tmp=val.ToString();
	d->Append(tmp.str());
	assert(down.is_empty());
}


void TNArray::DumpTree(StringTreeDump *d)
{
	d->Append("{");
	if(extra_comma_start)  d->Append(",");
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		i->DumpTree(d);
		if(i->next)  d->Append(",");
	}
	if(extra_comma_end)  d->Append(",");
	d->Append("}");
}


void TNTypeSpecifier::DumpTree(StringTreeDump *d)
{
	switch(ev_type_id)
	{
		case EVT_Unknown:
			d->Append("[unknown type] ");
			break;
		case EVT_Void:
			d->Append("[void] ");
			break;
		case EVT_POD:
			switch(vtype)
			{
				case Value::VTInteger:
					assert(down.is_empty());
					d->Append("int ");
					break;
				case Value::VTScalar:
					assert(down.is_empty());
					d->Append("scalar ");
					break;
				case Value::VTRange:
					assert(down.is_empty());
					d->Append("range ");
					break;
				case Value::VTVector:
					assert(down.is_empty() || down.count()==1);
					if(down.is_empty())
					{  d->Append("vector ");  }
					else
					{
						d->Append("vector<");
						down.first()->DumpTree(d);
						d->Append("> ");
					}
					break;
				case Value::VTMatrix:
					assert(down.is_empty() || down.count()==2);
					if(down.is_empty())
					{  d->Append("matrix ");  }
					else
					{
						d->Append("matrix<");
						down.first()->DumpTree(d);
						d->Append(",");
						down.last()->DumpTree(d);
						d->Append("> ");
					}
					break;
				case Value::VTString:
					assert(down.is_empty());
					d->Append("string ");
					break;
				default: assert(0);
			}
			break;
		case EVT_AniScope:
			assert(down.count()==1);
			down.first()->DumpTree(d);
			d->Append(" ");
			break;
		case EVT_Array:
			// EVT_Array can happen here for function call return type 
			// and array operator new (OF_NewArray, "new[][] int"). 
			assert(down.count()==1);
			down.first()->DumpTree(d);
			d->Append("[]");
			if(parent()->NType()!=TN_TypeSpecifier)
			{  d->Append(" ");  }
			break;
		default: assert(0);
	}
}


const char *OperatorFuncIDString(OperatorFuncID ofid)
{
	switch(ofid)
	{
		case OF_None:  return("[none]");
		
		case OF_Subscript:  return("[]");
		case OF_FCall:  return("()");
		case OF_PODCast:  return("(cast)");
		case OF_MembSel:  return(".");
		case OF_Mapping:  return("->");
		
		case OF_PostIncrement:  return("++");
		case OF_PostDecrement:  return("--");
		case OF_PreIncrement:  return("++");
		case OF_PreDecrement:  return("--");
		
		case OF_UnPlus:  return("+");
		case OF_UnMinus:  return("-");
		case OF_UnNot:  return("!");
		
		case OF_New:  return("new");
		case OF_NewArray:  return("new[]");
		case OF_Delete:  return("delete");
		
		case OF_Pow:  return("^");
		case OF_Mult:  return("*");
		case OF_Div:  return("/");
		case OF_Modulo:  return("%");
		case OF_Add:  return("+");
		case OF_Subtract:  return("-");
		case OF_Lesser:  return("<");
		case OF_Greater:  return(">");
		case OF_LesserEq:  return("<=");
		case OF_GreaterEq:  return(">=");
		case OF_Equal:  return("==");
		case OF_NotEqual:  return("!=");
		case OF_LogicalAnd:  return("&&");
		case OF_LogicalOr:  return("||");
		
		case OF_IfElse:  return("? :");
		case OF_IfElse2:  return("?:");
		
		case OF_Range:  return("*..*");
		case OF_RangeNoA:  return("..*");
		case OF_RangeNoB:  return("*..");
		case OF_Vector:  return("<,,>");
	}
	return("???");
}

static const struct 
{
	// Type: 1 -> binary op *,/,...
	//       2 -> unary pre +A
	//       3 -> unary post A--
	//       4 -> special
	int type;
	const char *opstr,*opstr2;
} _OperatorFunc_IDDesc[_OF_LAST]=
{
	{ 4, "NONE?![", "]" },  // OF_None=0,
	
	{ 4, "[", "]" },  // OF_Subscript,   // A[B]
	{ 4, "", NULL   },  // OF_FCall,       // A(B,C,D,...)
	{ 4, "(", ")" },  // OF_PODCast,     // A(B)
	{ 1, ".", NULL },  // OF_MembSel,     // A.B
	{ 1, "->", NULL },  // OF_Mapping,     // A->B
	
	{ 3, "++", NULL },  // OF_PostIncrement,  // A++
	{ 3, "--", NULL },  // OF_PostDecrement,  // A--
	{ 2, "++", NULL },  // OF_PreIncrement,   // ++A
	{ 2, "--", NULL },  // OF_PreDecrement,   // --A
	
	{ 2, "+", NULL },  // OF_UnPlus,	  // +A
	{ 2, "-", NULL },  // OF_UnMinus,	  // -A
	{ 2, "!", NULL },  // OF_UnNot, 	  // !A
	
	{ 4, "new ", NULL   },  // OF_New,         // new A(B,C,D,...)
	{ 4, "new ", NULL   },  // OF_NewArray,    // new A[B][C]...
	{ 2, "delete ", NULL },  // OF_Delete,      // delete A
	
	{ 1, "^", NULL },  // OF_Pow,         // A^B
	{ 1, "*", NULL },  // OF_Mult,        // A*B
	{ 1, "/", NULL },  // OF_Div,		  // A/B
	{ 1, "%", NULL },  // OF_Modulo,	  // A%B
	{ 1, "+", NULL },  // OF_Add,		  // A+B
	{ 1, "-", NULL },  // OF_Subtract,    // A-B
	{ 1, "<", NULL },  // OF_Lesser,	  // A<B
	{ 1, ">", NULL },  // OF_Greater,	  // A>B
	{ 1, "<=", NULL },  // OF_LesserEq,    // A<=B
	{ 1, ">=", NULL },  // OF_GreaterEq,   // A>=B
	{ 1, "==", NULL },  // OF_Equal,       // A==B
	{ 1, "!=", NULL },  // OF_NotEqual,    // A!=B
	{ 1, "&&", NULL },  // OF_LogicalAnd,  // A&&B
	{ 1, "||", NULL },  // OF_LogicalOr,   // A||B
	
	{ 4, NULL, NULL },  // OF_IfElse,      // A?B:C
	{ 4, NULL, NULL },  // OF_IfElse2,     // A?:B
	
	{ 1, "..", NULL },  // OF_Range,       // A..B
	{ 2, "..", NULL },  // OF_RangeNoA,    // ..A
	{ 3, "..", NULL },  // OF_RangeNoB,    // A..
	{ 4, "<", ">" },  // OF_Vector,      // <A,B,C,...>
};


void TNOperatorFunction::DumpTree(StringTreeDump *d)
{
	assert(func_id>=0 && func_id<_OF_LAST);
	
	int tp=_OperatorFunc_IDDesc[func_id].type;
	switch(tp)
	{
		case 1:
			assert(down.count()==2);
			d->Append("(");
			down.first()->DumpTree(d);
			d->Append(_OperatorFunc_IDDesc[func_id].opstr);
			down.last()->DumpTree(d);
			d->Append(")");
			break;
		case 2:
		case 3:
			assert(down.count()==1);
			d->Append("(");
			if(tp==2)
			{  d->Append(_OperatorFunc_IDDesc[func_id].opstr);  }
			down.first()->DumpTree(d);
			if(tp==3)
			{  d->Append(_OperatorFunc_IDDesc[func_id].opstr);  }
			d->Append(")");
			break;
		case 4:
			switch(func_id)
			{
				case OF_Subscript:   // A[B]
				case OF_PODCast:     // A(B)
					assert(down.count()==2);
					down.first()->DumpTree(d);
					d->Append(_OperatorFunc_IDDesc[func_id].opstr);
					down.last()->DumpTree(d);
					d->Append(_OperatorFunc_IDDesc[func_id].opstr2);
					break;
				case OF_None:
				case OF_Vector:
					d->Append(_OperatorFunc_IDDesc[func_id].opstr);
					for(TreeNode *i=down.first(); i; i=i->next)
					{
						i->DumpTree(d);
						if(i->next)  d->Append(",");
					}
					d->Append(_OperatorFunc_IDDesc[func_id].opstr2);
					break;
				case OF_FCall:
				case OF_New:
					assert(down.first());
					d->Append(_OperatorFunc_IDDesc[func_id].opstr);
					down.first()->DumpTree(d);
					d->Append("(");
					for(TreeNode *i=down.first()->next; i; i=i->next)
					{
						i->DumpTree(d);
						if(i->next)  d->Append(",");
					}
					d->Append(")");
					break;
				case OF_NewArray:
					assert(down.first());
					d->Append(_OperatorFunc_IDDesc[func_id].opstr);
					down.first()->DumpTree(d);
					// Children are of type TNNewArraySpecifier. 
					for(TreeNode *i=down.first()->next; i; i=i->next)
					{  i->DumpTree(d);  }
					break;
				case OF_IfElse:
					assert(down.count()==3);
					d->Append("(");
					down.first()->DumpTree(d);
					d->Append(" ? ");
					down.first()->next->DumpTree(d);
					d->Append(" : ");
					down.last()->DumpTree(d);
					d->Append(")");
					break;
				case OF_IfElse2:
					assert(down.count()==2);
					d->Append("(");
					down.first()->DumpTree(d);
					d->Append(" ?: ");
					down.last()->DumpTree(d);
					d->Append(")");
					break;
				default: assert(0);
			}
			break;
		default: assert(0);
	}
}


void TNFCallArgument::DumpTree(StringTreeDump *d)
{
	assert(down.first() && down.count()<=2);
	down.first()->DumpTree(d);
	if(down.last()!=down.first())
	{  d->Append("=");  down.last()->DumpTree(d);  }
}


static const char *AssignmentOp_str[_AO_LAST]=
	{"[??]","=","+=","-=","*=","/="};
const char *AssignmentOpIDString(AssignmentOpID ass_op)
{
	if(ass_op<0 || ass_op>=_AO_LAST)  return("???");
	return(AssignmentOp_str[ass_op]);
}

void TNAssignment::DumpTree(StringTreeDump *d)
{
	assert(ass_op>=0 && ass_op<_AO_LAST);
	assert(down.count()==2);
	down.first()->DumpTree(d);
	d->Append(AssignmentOpIDString(ass_op));
	down.last()->DumpTree(d);
}


void TNStatement::DumpTree(StringTreeDump *d)
{
	// We may not be here...
	assert(0);
}


void TNExpressionStmt::DumpTree(StringTreeDump *d)
{
	if(down.is_empty())
	{  d->Append("/*empty*/;\n");  }
	else
	{
		assert(down.count()==1);
		down.first()->DumpTree(d);
		d->Append(";\n");
	}
}


void TNIterationStmt::DumpTree(StringTreeDump *d)
{
	assert(loop_stmt);
	switch(itertype)
	{
		case IT_None:  assert(0);  break;
		case IT_For:
			assert(for_init_stmt && for_inc_stmt);
			d->Append("for(");
			for_init_stmt->DumpTree(d);  d->RemoveEnd('\n');  d->Append(" ");
			if(loop_cond)
			{  loop_cond->DumpTree(d);  }
			else
			{  d->Append("/*empty*/");  }
			d->Append("; ");
			for_inc_stmt->DumpTree(d);  d->RemoveEnd('\n');  d->RemoveEnd(';');
			d->Append(")\n");
			d->AddIndent();
			loop_stmt->DumpTree(d);
			d->SubIndent();
			break;
		case IT_While:
			assert(!for_init_stmt && !for_inc_stmt);
			d->Append("while(");
			if(loop_cond)
			{  loop_cond->DumpTree(d);  }
			else
			{  d->Append("/*error*/");  }
			d->Append(")\n");
			d->AddIndent();
			loop_stmt->DumpTree(d);
			d->SubIndent();
			break;
		case IT_DoWhile:
			assert(!for_init_stmt && !for_inc_stmt);
			d->Append("do\n");
			d->AddIndent();
			loop_stmt->DumpTree(d);
			d->SubIndent();
			d->Append("while(");
			if(loop_cond)
			{  loop_cond->DumpTree(d);  }
			else
			{  d->Append("/*error*/");  }
			d->Append(");\n");
			break;
		default:  assert(0);
	}
	if(next)
	{  d->Append("\n");  }
}


void TNSelectionStmt::DumpTree(StringTreeDump *d)
{
	d->Append("if(");
	if(cond_expr)
	{  cond_expr->DumpTree(d);  }
	else
	{  d->Append("/*error*/");  }
	d->Append(")\n");
	assert(if_stmt);
	d->AddIndent();
	if_stmt->DumpTree(d);
	d->SubIndent();
	if(else_stmt)
	{
		// Dump else-if - chains as: 
		//   if()
		//   else if()
		//   else if()
		//   else
		// instead of
		//   if()
		//   else
		//       if()
		//       else
		//           if()
		//           else
		if(else_stmt->StmtType()==TNS_SelectionStmt)
		{
			// "else-if" - chain. 
			d->Append("else ");
			else_stmt->DumpTree(d); 
		}
		else
		{
			d->Append("else\n");
			d->AddIndent();
			else_stmt->DumpTree(d); 
			d->SubIndent();
		}
	}
	if(next)
	{  d->Append("\n");  }
}


void TNJumpStmt::DumpTree(StringTreeDump *d)
{
	switch(jumptype)
	{
		case JT_None:  assert(0);  break;
		case JT_Break:
			assert(down.is_empty());
			d->Append("break;\n");
			break;
		case JT_Return:
			assert(down.count()<=1);
			d->Append("return ");
			if(down.first())
			{  down.first()->DumpTree(d);  }
			d->Append(";\n");
			break;
		default:  assert(0);
	}
}


void TNCompoundStmt::DumpTree(StringTreeDump *d)
{
	bool indent_just_added=d->IndentJustAdded();
	if(indent_just_added && parent() && parent()->NType()==TN_Statement && 
		((TNStatement*)parent())->StmtType()==TNS_CompoundStmt)
	{  indent_just_added=0;  }
	
	if(indent_just_added)  d->SubIndent();
	if(down.is_empty())
	{  d->Append("{ /*empty*/ }\n");  }
	else if(down.first()==down.last() && 
		( ((TNStatement*)down.first())->StmtType()!=TNS_IterationStmt && 
		  ((TNStatement*)down.first())->StmtType()!=TNS_SelectionStmt ) )
	{
		d->Append("{  ");
		down.first()->DumpTree(d);
		d->RemoveEnd('\n');
		d->Append("  }\n");
	}
	else
	{
		d->Append("{\n");
		d->AddIndent();
		for(TreeNode *i=down.first(); i; i=i->next)
		{  i->DumpTree(d);  }
		d->SubIndent();
		d->Append("}\n");
	}
	if(indent_just_added)  d->AddIndent();
}


void TNDeclarator::DumpTree(StringTreeDump *d)
{
	switch(decltype)
	{
		case DT_None:  assert(0);  break;
		case DT_Name:
			assert(down.count()==1);
			down.first()->DumpTree(d);
			break;
		case DT_Array:
			assert(down.count()<=2 && down.first());
			down.first()->DumpTree(d);
			d->Append("[");
			if(down.last()!=down.first())
			{  down.last()->DumpTree(d);  }
			d->Append("]");
			break;
		case DT_Initialize:
			assert(down.count()<=2 && down.first());
			down.first()->DumpTree(d);
			d->Append("=");
			down.last()->DumpTree(d);
			break;
		default: assert(0);
	}
}


void TNNewArraySpecifier::DumpTree(StringTreeDump *d)
{
	assert(down.count()<=1);
	d->Append("[");
	if(down.first())  down.first()->DumpTree(d);
	d->Append("]");
}


void TNDeclarationStmt::DumpTree(StringTreeDump *d)
{
	assert(!down.is_empty());
	if((attrib & IS_Static))
	{  d->Append("static ");  }
	if((attrib & IS_Const))
	{  d->Append("const ");  }
	down.first()->DumpTree(d);
	for(TreeNode *i=down.first()->next; i; i=i->next)
	{
		i->DumpTree(d);
		if(i->next)  d->Append(",");
	}
	d->Append(";\n");
}


void TNAniDescStmt::DumpTree(StringTreeDump *d)
{
	d->Append("%");
	if(core_name)
	{
		core_name->DumpTree(d);
		d->Append(":");
	}
	/*else if(cc_factory)
	{
		// This would require to use <calcore/ccif.h> which is not 
		// acceptable in ani-parser. 
		d->Append("[");
		d->Append(((CCIF_Factory*)cc_factory)->Name());
		d->Append("]:");
	}*/
	if(name_expr)
	{  name_expr->DumpTree(d);  }
	if(adb_child)
	{  adb_child->DumpTree(d);  }
	d->Append(";\n");
}


void TNFuncDefArgDecl::DumpTree(StringTreeDump *d)
{
	assert(down.count()==2);
	down.first()->DumpTree(d);
	down.last()->DumpTree(d);
}


void TNFunctionDef::DumpTree(StringTreeDump *d)
{
	if(!func_name || !func_body)  assert(0);
	if(func_name->NType()!=TN_Identifier)  assert(0);
	if(func_body->NType()!=TN_Statement || 
		func_body->StmtType()!=TNS_CompoundStmt)  assert(0);
	if(prev && prev->NType()==TN_Statement)
	{  d->Append("\n");  }
	if(attrib & IS_Static)
	{  d->Append("static ");  }
	if(ret_type)
	{
		assert(!(attrib & IS_Destructor));
		ret_type->DumpTree(d);
	}
	else if((attrib & IS_Destructor))
	{  d->Append("~");  }
	else if((attrib & IS_Animation))
	{  d->Append("animation ");  }
	//else -> happens for internal funcs like "$$povhook". 
	func_name->DumpTree(d);
	if(!(attrib & IS_Animation))
	{
		d->Append("(");
		for(TreeNode *arg=first_arg; arg; arg=arg->next)
		{
			assert(arg->NType()==TN_FuncDefArgDecl);
			arg->DumpTree(d);
			if(arg->next)  d->Append(",");
		}
		d->Append(")");
	}
	else assert(!first_arg);
	if(attrib & IS_Const)
	{  d->Append(" const");  }
	d->Append("\n");
	func_body->DumpTree(d);
	if(next)
	{  d->Append("\n");  }
}


void TNObject::DumpTree(StringTreeDump *d)
{
	assert(!down.is_empty());
	if(prev && prev->NType()==TN_Statement)
	{  d->Append("\n");  }
	// Print name: 
	d->Append("object ");
	down.first()->DumpTree(d);
	d->Append("\n{\n");
	d->AddIndent();
	// Dump body: 
	for(TreeNode *i=down.first()->next; i; i=i->next)
	{  i->DumpTree(d);  }
	d->SubIndent();
	d->Append("}\n");
	if(next)
	{  d->Append("\n");  }
}


void TNSetting::DumpTree(StringTreeDump *d)
{
	assert(!down.is_empty());
	if(prev && prev->NType()==TN_Statement)
	{  d->Append("\n");  }
	// Print name: 
	d->Append("setting ");
	down.first()->DumpTree(d);
	d->Append("\n{\n");
	d->AddIndent();
	// Dump body: 
	for(TreeNode *i=down.first()->next; i; i=i->next)
	{  i->DumpTree(d);  }
	d->SubIndent();
	d->Append("}\n");
	if(next)
	{  d->Append("\n");  }
}


void TNAnimation::DumpTree(StringTreeDump *d)
{
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		i->DumpTree(d);
		if(i->next) d->Append("\n");
	}
}


}  // end of namespace func
