// Copyright 2015 Peter Most, PERA Software Solutions GmbH
//
// This file is part of the CppAidKit library.
//
// CppAidKit is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CppAidKit is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with CppAidKit. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <mutex>
#include <functional>

namespace aidkit {

// Replacing accessor/const_accessor with unique_ptr is possible but has the undesirable sideeffect
// that 'if (data.access())' would compile because a unique_ptr is convertible to bool!

template <typename T, typename Mutex = std::mutex>
class thread_shared {
	public:
		class accessor {
			public:
				accessor(T *d, Mutex *m) noexcept
					: m_lock(*m), m_data(d)
				{ }

				accessor(const accessor &) = delete;
				accessor &operator=(const accessor &) = delete;

				T *operator->() noexcept
				{
					return m_data;
				}

				T &operator*() noexcept
				{
					return *m_data;
				}

			private:
				std::scoped_lock<Mutex> m_lock;
				T *m_data;
		};

		class const_accessor {
			public:
				const_accessor(const T *d, Mutex *m) noexcept
					: m_lock(*m), m_data(d)
				{ }

				const_accessor(const const_accessor &) = delete;
				const_accessor &operator=(const const_accessor &) = delete;

				const T *operator->() const noexcept
				{
					return m_data;
				}

				const T &operator*() const noexcept
				{
					return *m_data;
				}

			private:
				std::scoped_lock<Mutex> m_lock;
				const T *m_data;
		};

		template <typename... Args>
		thread_shared(Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
			: m_data(std::forward<Args>(args)...)
		{
		}

		thread_shared(const thread_shared &) = delete;
		thread_shared &operator=(const thread_shared &) = delete;

		[[nodiscard]]
		accessor access() noexcept
		{
			return accessor(&m_data, &m_mutex);
		}

		[[nodiscard]]
		const_accessor access() const noexcept
		{
			return const_accessor(&m_data, &m_mutex);
		}

		template <typename Functor, typename U, typename... Types>
		friend decltype(auto) access(Functor &&, thread_shared<U> &, thread_shared<Types> &...);

		template <typename Functor, typename U, typename... Types>
		friend decltype(auto) access(Functor &&, const thread_shared<U> &, const thread_shared<Types> &...);

	private:
		T m_data;
		mutable Mutex m_mutex;
};

template <typename Functor, typename T, typename... Types>
inline decltype(auto) access(Functor &&functor, thread_shared<T> &data, thread_shared<Types> &...datas)
{
	std::scoped_lock lock(data.m_mutex, datas.m_mutex...);

	return std::invoke(std::forward<Functor>(functor), data.m_data, datas.m_data...);
}

template <typename Functor, typename T, typename... Types>
inline decltype(auto) access(Functor &&functor, const thread_shared<T> &data, const thread_shared<Types> &...datas)
{
	std::scoped_lock lock(data.m_mutex, datas.m_mutex...);

	return std::invoke(std::forward<Functor>(functor), data.m_data, datas.m_data...);
}

}
