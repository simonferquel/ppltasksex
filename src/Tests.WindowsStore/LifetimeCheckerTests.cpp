#include "pch.h"
#include "CppUnitTest.h"
#include <ppltasks.h>
#include "..\ppltasksex.h"

using namespace concurrency;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestsWindowsStore
{
	TEST_CLASS(LifetimeCheckerTests)
	{
	public:
		
		TEST_METHOD(TestCancellationSourceAssumptions)
		{
			cancellation_token_source src1;
			cancellation_token_source srcCopied(src1);
			auto token = src1.get_token();
			srcCopied.cancel();
			Assert::IsTrue(token.is_canceled());


			cancellation_token_source src2;
			cancellation_token_source srcAssigned = src2;
			auto token2 = src2.get_token();
			srcAssigned.cancel();
			Assert::IsTrue(token2.is_canceled());
		}
		class LifetimeCheckerTestComponent{
		private:
			ppltasksex::lifetime_checker _checker;
			LifetimeCheckerTestComponent(const LifetimeCheckerTestComponent& noncopied){

			}
		public:
			LifetimeCheckerTestComponent(){}
			~LifetimeCheckerTestComponent(){
				_checker.onDestructing();
			}
			task<int> doSomethingAsynchronously(){
				ppltasksex::lifetime_checker localChecker(_checker);
				return task<void>([](){
					WaitForSingleObjectEx(GetCurrentThread(), 500, FALSE);
				}).then([localChecker, this](){
					auto guard = localChecker.acquireOrCancel();
					return 42;
				}, localChecker.getCancelToken());
			}
			void foo(){

			}
		};
		TEST_METHOD(TestLifetimeCheckerDone){
			LifetimeCheckerTestComponent component;
			{
				auto t = component.doSomethingAsynchronously();
				WaitForSingleObjectEx(GetCurrentThread(), 1000, FALSE);
				auto result = t.get();
				Assert::AreEqual(42,result);
			}
		}

		TEST_METHOD(TestLifetimeCheckerCancelled){
			task<int> t;
			{
				LifetimeCheckerTestComponent component;
				t = component.doSomethingAsynchronously();
			}
			try{
				WaitForSingleObjectEx(GetCurrentThread(), 1000, FALSE);
				t.get();
				Assert::Fail(L"Should have cancelled");
			} catch(task_canceled& taskCancel){
				//
			}
		}

	};
}