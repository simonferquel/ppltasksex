ppltasksex
==========

Helper collection to help correct usage of ppl tasks

1. LifetimeChecker

When you create a task continuation in a member function that captures the "this" pointer, you need to check if the object is still living before accessing its state.
Additionaly, as the destruction of the object could happen concurrently with the task continuation, you need to do a bit of synchronization between your task continuations and the destructor.

The LifetimeChecker class help you do this in a simple way. The following code sample illustrate how to use it:

class AsyncComponent{
private:
	ppltasksex::lifetime_checker _checker;
	AsyncComponent(const LifetimeCheckerTestComponent& noncopied){
	}
public:
	AsyncComponent(){}
	~AsyncComponent(){
	    // this acquires the lifetime_checker mutex and cancels the underlying cancel_token
		_checker.onDestructing();
	}
	task<int> doSomethingAsynchronously(){
		// we need to capture the lifetime_checker by value (and not accessing it via the
		// member variable) so we introduce a local copy
		ppltasksex::lifetime_checker localChecker(_checker);
		return task<void>([](){
			// in a real case, this would be an asynchronous call like an async IO
			WaitForSingleObjectEx(GetCurrentThread(), 500, FALSE);
		}).then([localChecker, this](){
		    // this acquires the lifetime_checker mutex, and check the cancellation token.
			// if it has been cancelled (by the destructor), it will call cancel_current_task() (that throws a task_cancelled exception)
			auto guard = localChecker.acquireOrCancel();

			// as long as the "guard" variable is in scope, member variables can be safely accessed
			return 42;
		}, localChecker.getCancelToken());
	}
};


2. whileAsync

whileAsync reproduces the "while" loop flow with the differnce that the body is asynchronous.
whileAsync takes 2 arguments and returns a task<void> completing when the condition returns false:
- condition : a function returning a boolean
- body : a function returning a task<void> that will be executed while the condition returns true

If you use the lifetime_checker with whileAsync, you need to pay attention that the first execution of "condition" will not occur in the context of a task (if whileAsync is called outside of task), and so you should not call "acquireOrCancel" in it.
However, this condition works well:

[this, localChecker](){
	auto guard = localChecker.acquire();
	if(localChecker.getCancelToken().is_canceled()){
		return false;
	}
	// real condition
}

Additionaly, only checking the lifetime_checker in the condition is not enough. You should check it inside the body as well (because living the condition releases the lifetime_checker mutex)

3. forAsync

forAsync reproduces the "for" loop flow with an asynchronous body:

ppltasksex::forAsync<int>(0, 5, [&result](const int& v){
				return concurrency::create_task([v]{
					...
				});
			});

it returns a task<void> that completes when the async loop has ended

remark: each occurence of the body is executed after the previous one completes

4. foreachAsync

foreachAsync works like std::foreach, except that the body is asynchronous (each occurence of the body is executed after the previous one completes)

			std::vector<int> v;
			v.push_back(0);
			v.push_back(1);
			v.push_back(2);
			v.push_back(3);
			v.push_back(4);

			int result = 0;
			
			ppltasksex::foreachAsync(v.begin(), v.end(),
				[](const int& val){
					return concurrency::create_task([val]{
						...
					});
			});