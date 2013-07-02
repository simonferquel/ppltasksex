#include "pch.h"
#include "CppUnitTest.h"
#include <ppltasks.h>
#include "..\ppltasksex.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestsWindowsStore
{
	TEST_CLASS(WhileAsyncTest)
	{
	public:
		
		TEST_METHOD(TestWhile)
		{
			int testValue = 0;
			ppltasksex::whileAsync([&testValue](){
				return testValue < 42;
			}, [&testValue](){
				return concurrency::create_task([&testValue](){
					++testValue;
				});
			});
			WaitForSingleObjectEx(GetCurrentThread(), 500, FALSE);
			Assert::AreEqual(42, testValue);

		}

	};
}