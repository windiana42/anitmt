#error UNUSED. 

// (Base) Class as passed to ProcessVals(). 
// The actual processing function must be provided by a derived class. 
struct ProcessValsInfo
{
	// The current exec thread info must be passed because 
	// most of the values are stored on the stack. 
	ExecThreadInfo *info;
	
	// The actual processing function called for every variable. 
	virtual void process(TNIdentifier *name,const ExprValue &ev);
	
	_CPP_OPERATORS
	ProcessValsInfo()
	~ProcessValsIndo()
};


void TreeNode::ProcessVals(ProcessValsInfo *)
{
	// Must be overridden if ever called. 
	assert(0);
}


void TNIdentifier::ProcessVals(ProcessValsInfo *pvi)
{
	assert(evalfunc);
	ExprValue ev(evalfunc->Eval(this,pvi->info));
	pvi->process(this,ev);
}


void TNExpression::ProcessVals(ProcessValsInfo *pvi)
{
	// Special case as usual :)
	// I mean: TNExpression is a base class for different expressions 
	// the only special case of no derived class is an expression 
	// holding an identifier. 
	if(!down.first() || down.first()->NType()!=TN_Identifier || 
		down.first()!=down.last())
	{  assert(0);  }
	
	down.first()->ProcessVals(pvi);
}


void TNExpressionList::ProcessVals(ProcessValsInfo *pvi)
{
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->ProcessVals(pvi);  }
}


void TNValue::ProcessVals(ProcessValsInfo *)
{
	return;  // nothing to do
}


void TNArray::ProcessVals(ProcessValsInfo *pvi)
{
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->ProcessVals(pvi);  }
}


void TNOperatorFunction::ProcessVals(ProcessValsInfo *pvi)
{
	switch(func_id)
	{
		case OF_None:
			break;
		case OF_FCall:       // A(B,C,D,...)          B,C,D,...=TNFCallArgument
		case OF_IfElse:      // A?B:C
		case OF_Vector:      // <A,B,C,...>
			for(TreeNode *i=down.first()->next; i; i=i->next)
			{  i->ProcessVals(pvi);  }
			break;
		case OF_PODCast:     // A(B)                  A=TNTypeSpecifier
			down.last()->ProcessVals(pvi);
			break;
		case OF_PostIncrement:  // A++
		case OF_PostDecrement:  // A--
		case OF_PreIncrement:   // ++A
		case OF_PreDecrement:   // --A
			
		case OF_UnPlus:      // +A
		case OF_UnMinus:     // -A
		case OF_UnNot:       // !A
		case OF_RangeNoA:    // ..A
		case OF_RangeNoB:    // A..
			down.first()->ProcessVals(pvi);
			break;
		case OF_Pow:         // A^B
		case OF_Mult:        // A*B
		case OF_Div:         // A/B
		case OF_Modulo:      // A%B
		case OF_Add:         // A+B
		case OF_Subtract:    // A-B
		case OF_Lesser:      // A<B
		case OF_Greater:     // A>B
		case OF_LesserEq:    // A<=B
		case OF_GreaterEq:   // A>=B
		case OF_Equal:       // A==B
		case OF_NotEqual:    // A!=B
		case OF_LogicalAnd:  // A&&B
		case OF_LogicalOr:   // A||B
		case OF_Range:       // A..B
		case OF_IfElse2:     // A?:B
			down.first()->ProcessVals(pvi);
			down.last()->ProcessVals(pvi);
			break;
		case OF_New:         // new A(B,C,D,...)   A=TNIdentifier: B,C,D,...=TNFCallArgument
		case OF_NewArray:    // new A[B][C]        A=TNTypeSpecifier: B,C,..=TNNewArraySpecifier
		case OF_Delete:      // delete A           A=TNExpression
			Warning("%s: ignoring operator %s\n",
				pvi->purpose_str,OperatorFuncIDString(func_id));
			break;
		case OF_Subscript:   // A[B]
			// Hmmm... is that what we expect?
			down.first()->ProcessVals(pvi);
			down.last()->ProcessVals(pvi);
			break;
		case OF_MembSel:     // A.B  (member select)  B=TNIdentifier
		case OF_Mapping:     // A->B                  B=TNIdentifier
		{
			// Hmmm... is that what we expect?
			// The base is probably an indetifier for the object. 
			down.first()->ProcessVals(pvi); ???
			// The member element is always an identifier. But in order 
			// to address it properly, we need the base VALUE. And that 
			// normally means evaluation. But that means context switches, 
			// blah blah...
			down.last()->ProcessVals(pvi);
			break;
		default:
			assert(0);
	}
}


void TNFCallArgument::ProcessVals(ProcessValsInfo *pvi)
{
	down.last()->ProcessVals(pvi);
}


void TNAssignment::ProcessVals(ProcessValsInfo *pvi)
{
	down.last()->ProcessVals(pvi);
	down.first()->ProcessVals(pvi);
}


void TNStatement::ProcessVals(ProcessValsInfo *)
{
	// May currently not be called for any sort of statement. 
	assert(0);
}


void TNDeclarator::ProcessVals(ProcessValsInfo *)
{
	// May currently not be called for any sort of declarator. 
	assert(0);
}


void TNNewArraySpecifier::ProcessVals(ProcessValsInfo *pvi)
{
	// May currently not be called for TNNewArraySpecifier. 
	assert(0);
}


void TNFuncDefArgDecl::ProcessVals(ProcessValsInfo *)
{
	// May currently not be called for function defs. 
	assert(0);
}


void TNFunctionDef::ProcessVals(ProcessValsInfo *)
{
	// May currently not be called for function defs. 
	assert(0);
}


void TNObject::ProcessVals(ProcessValsInfo *)
{
	// May not be called on object. 
	assert(0);
}


void TNSetting::ProcessVals(ProcessValsInfo *)
{
	// May not be called on setting. 
	assert(0);
}


void TNAnimation::ProcessVals(ProcessValsInfo *)
{
	// May not be called on animation. 
	assert(0);
}
