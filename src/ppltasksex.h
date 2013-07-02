#include <ppltasks.h>
#include <memory>
#include <mutex>
#include <functional>
#include <iterator>

namespace ppltasksex{


	class movable_lockguard{
	private:
		std::mutex* _pMutex;
		// uncopyable:
		movable_lockguard(const movable_lockguard& noncopied){}
	public:
		movable_lockguard(std::mutex* pMutex):_pMutex(pMutex){
			if(_pMutex){
				_pMutex->lock();
			}
		}
		movable_lockguard(movable_lockguard&& moved):
			_pMutex(nullptr)
		{
			std::swap(moved._pMutex, this->_pMutex);
		}
		~movable_lockguard(){
			if(_pMutex){
				_pMutex->unlock();
				_pMutex = nullptr;
			}
		}
	};

	class lifetime_checker{
	private:
		std::shared_ptr<std::mutex> _mutex;
		concurrency::cancellation_token_source _cts;

		// non movable
		lifetime_checker(lifetime_checker&& nonmoved)
		{

		}

	public:
		lifetime_checker() : 
			_mutex(new std::mutex()), 
			_cts(){
		}
		lifetime_checker(const lifetime_checker& copied):
			_mutex(copied._mutex), _cts(copied._cts)
		{

		}
		concurrency::cancellation_token getCancelToken() const{
			return _cts.get_token();
		}
		void onDestructing(){
			movable_lockguard lg(_mutex.get());
			_cts.cancel();
		}
		movable_lockguard acquire() const{
			return movable_lockguard(_mutex.get());
		}
		movable_lockguard acquireOrCancel() const {
			movable_lockguard lg(_mutex.get());
			if(getCancelToken().is_canceled()){
				concurrency::cancel_current_task();
			}
			return std::move(lg);
		}
	};

	inline concurrency::task<void> whileAsync(
		std::function<bool(void)> condition, 
		std::function<concurrency::task<void>(void)> body){
			static concurrency::task<void> s_voidTaskCached ([]{});
			if(condition()){
				return body().then([condition, body](){
					return whileAsync(condition, body);
				});
			} else {
				return s_voidTaskCached;
			}
	}
	template<typename TCounter>
	concurrency::task<void> forAsync(
		TCounter startInclusive,
		TCounter endExclusive,
		std::function<concurrency::task<void>(const TCounter&)> body){
			static concurrency::task<void> s_voidTaskCached ([]{});
			
			if(startInclusive<endExclusive){
				return body(startInclusive).then([startInclusive, endExclusive, body](){
					TCounter nextValue(startInclusive);
					return forAsync(++nextValue, endExclusive, body);
				});
			} else {
				return s_voidTaskCached;
			}
	}

	template<typename TIterator>
	concurrency::task<void> foreachAsync(TIterator begin, TIterator end, 
		std::function<concurrency::task<void>(const typename std::iterator_traits<TIterator>::value_type&)> body){
			
			static concurrency::task<void> s_voidTaskCached ([]{});

			if(begin != end){
				return body(*begin).then([begin, end, body](){
					TIterator next(begin);
					return foreachAsync(++next, end, body);
				});
			} else {
				return s_voidTaskCached;
			}
	}
	

}