#include "pch.h"
#include "CppUnitTest.h"
#include <ppltasks.h>
#include "..\ppltasksex.h"
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestsWindowsStore
{
	TEST_CLASS(ForAsyncTest)
	{
	public:
		
		TEST_METHOD(TestLoop)
		{
			int result = 0;
			ppltasksex::forAsync<int>(0, 5, [&result](const int& v){
				return concurrency::create_task([v, &result]{
					result+=v;
				});
			});

			WaitForSingleObjectEx(GetCurrentThread(), 500, FALSE);
			Assert::AreEqual(0+1+2+3+4, result);
		}

		TEST_METHOD(TestForeach){
			std::vector<int> v;
			v.push_back(0);
			v.push_back(1);
			v.push_back(2);
			v.push_back(3);
			v.push_back(4);

			int result = 0;
			
			ppltasksex::foreachAsync(v.begin(), v.end(),
				[&result](const int& val){
					return concurrency::create_task([val, &result]{
						result+=val;
					});
			});
			WaitForSingleObjectEx(GetCurrentThread(), 500, FALSE);
			Assert::AreEqual(0+1+2+3+4, result);
			
		}

	};
}