#include <hlib/notifierlist.h>

char *prg_name="test-cpp";

class A : public NotifyHandler<A>
{
	void handlenotify(int ntype,A *ptr) {}
	
	public:
		A() : NotifyHandler<A>() {}
		~A() {}
};

int main()
{
	// Currently, this will only test for clean compilation. 
	NotifierList<A> nlist;
	A a[10];
	for(int i=0; i<10; i++)
		nlist.Append(&a[i]);
	nlist.CallNotifiers(0,&a[0]);
	nlist.Remove(&a[2]);
	nlist.is_empty();
	nlist.Clear();
	
	return(0);
}
