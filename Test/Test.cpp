// Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <boost/thread/thread.hpp>

int g_i = 0;
void Proc1()
{
	while (true)
	{
		g_i++;
		Sleep(100);
	}

}
void Proc2()
{
	while (true)
	{
		if (0 == g_i%10)
		{
			g_i = 1;
			Sleep(100);
		}
	}
}
class A
{
	int a;
};
class B :public A
{
	int b;
};
int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "please input g_i" << std::endl;
	std::cin >> g_i;
	/*boost::thread_group grp;
	boost::thread *p1 = grp.create_thread(Proc1);
	boost::thread p2(Proc2);
	int i = 0;
	
	while (i < 100)
	{
		std::cout << g_i << std::endl;
		i++;
		Sleep(100);
		if (10 == i)
		{
			
			grp.remove_thread(p1);
			//p2.try_join_for(boost::chrono::milliseconds(500));
			//i = 10;
			//delete p1;
			
		}
	}*/
	B *p1 = new B();
	A *p2 = (A*)p1;
	std::cout << "p1:" << p1 << std::endl;
	std::cout << "p2:" << p2 << std::endl;
	std::cin >> g_i;
	return 0;
}

