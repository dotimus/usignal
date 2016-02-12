/*
mikro signal v 1.0

copyright Pedro Simoes
Licensed as mit or apache or lgpl v2+ or gpl v2+
choose your poison
what, no bsd, not at the moment, ill think about it.
*/
#pragma once

#ifndef USIGNAL_HPP
#define USIGNAL_HPP


#include <memory>
#include <functional>
#include <list>
#include <utility>
#include <thread>
#include <future>
#include <mutex>

// A signal may call multiple slots with the same signature.  connect functions to the signal
// which will be called when the () method on the signal object is invoked. argument passed to ()
// will be passed to the given functions.
namespace usignal {

	template <typename ...Args>
	class signal : public signal<void(Args...)> {};

	template <typename R, typename ...Args>
	class signal<R(Args...)> {
		typedef std::pair<bool, std::function<R(Args...)>> slot;
		typedef std::shared_ptr<slot> slot_ptr;
		mutable std::list< slot_ptr> slots;
		mutable bool enabled = true;
		mutable bool asynchronous = true;
		mutable std::mutex m;

		template <typename F, typename I>
		std::function<R(Args...)> member_func_to_lambda(F&& f, I&&  i) const {
			auto func = [&](Args... a) ->R {
				return  (i.*f)(a...);
			};
			return func;
		}

	public:
		// connects a std::function to the signal. The returned
		// value can be used to disconnect the function again
		template <typename F>
		slot_ptr connect(F &&f) const {
			std::lock_guard<std::mutex>{m};
			slot_ptr ptr = std::make_shared<slot>(std::make_pair(true, f));
			slots.push_back(ptr);
			return ptr;
		}
		//connects a member function F of a given I instance obj to this signal
		template <typename F, typename I>
		slot_ptr connect(F&& f, I&&  i) const {
			std::lock_guard<std::mutex>{m};
			slot_ptr ptr = std::make_shared<slot>(std::make_pair(true, member_func_to_lambda(f, i)));
			slots.push_back(ptr);
			return ptr;
		}
		// disconnects  connected function or all slots if no args
		void disconnect() const {
			std::lock_guard<std::mutex>{m};
			slots.clear();
		}
		void disconnect(const slot_ptr &ptr) const {
			std::lock_guard<std::mutex>{m};
			slots.remove(ptr);
		}
		//check state of specific slot or if empty of signal
		bool state() const { return state; }
		bool state(const slot_ptr &ptr) const {
			std::lock_guard<std::mutex>{m};
			for (auto it : slots) { if (it == ptr) return (*it).first; }
			return false;
		}
		//change state on/off of specific slot or if empty of signal
		bool toggle() const { return enabled = !enabled; }
		bool toggle(const slot_ptr &ptr) const {
			std::lock_guard<std::mutex>{m};
			for (auto it : slots) { if (it == ptr) return (*it).first = !(*it).first; }
			return false;
		}
		// async by default, call without params to get state, call with false/true to set desired state 
		bool async() const { return asynchronous; }
		bool async(bool state) const {
			return asynchronous = state;
		}
		// calls all connected functions with return R and puts result in a list<shared_future<R>>>
		std::list<std::shared_future<R>> operator()(Args... a) const {
			std::lock_guard<std::mutex>{m};
			std::list<std::shared_future<R>> ret{};
			if (enabled)
				for (auto it : slots)
					if ((*it).first)
					{
						std::packaged_task<R(Args...)> task((*it).second); // wrap the function
						std::shared_future<R> f = task.get_future();
						ret.push_back(f);  // get a future
						std::thread(std::move(task), a...).detach();  // launch on a thread
						if (!asynchronous)
							f.wait();
					}
			return ret;
		}
	};
}
#endif /* USIGNAL_HPP */

