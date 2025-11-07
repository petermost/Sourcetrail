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

module;

#include <mutex>

export module aidkit.shared_data;

export namespace aidkit {

template <typename T, typename Mutex = std::mutex>
	class shared_data_ptr {
		public:
			shared_data_ptr(T *d, Mutex *m) noexcept
				: m_data(d), m_mutex(m)
			{
				m_mutex->lock();
			}

			~shared_data_ptr() noexcept
			{
				m_mutex->unlock();
			}

			T *operator->() noexcept
			{
				return m_data;
			}

			T &operator*() noexcept
			{
				return *m_data;
			}

			shared_data_ptr(const shared_data_ptr &) = delete;
			shared_data_ptr &operator=(const shared_data_ptr &) = delete;

		private:
			T *m_data;
			mutable Mutex *m_mutex;
	};

template <typename T, typename Mutex = std::mutex>
	class const_shared_data_ptr {
		public:
			const_shared_data_ptr(const T *d, Mutex *m) noexcept
				: m_data(d), m_mutex(m)
			{
				m_mutex->lock();
			}

			~const_shared_data_ptr() noexcept
			{
				m_mutex->unlock();
			}

			const T *operator->() const noexcept
			{
				return m_data;
			}

			const T &operator*() const noexcept
			{
				return *m_data;
			}

			const_shared_data_ptr(const const_shared_data_ptr &) = delete;
			const_shared_data_ptr &operator=(const const_shared_data_ptr &) = delete;

		private:
			const T *m_data;
			mutable Mutex *m_mutex;
	};

template <typename T, typename Mutex = std::mutex>
	class shared_data {
		public:
			template <typename... Args>
				shared_data(Args &&... args)
					: m_data(std::forward<Args>(args)...)
				{
				}

			shared_data(const shared_data &) = delete;
			shared_data &operator=(const shared_data &) = delete;

			shared_data_ptr<T, Mutex> lock()
			{
				return shared_data_ptr(&m_data, &m_mutex);
			}

			const_shared_data_ptr<T, Mutex> lock() const
			{
				return const_shared_data_ptr(&m_data, &m_mutex);
			}

		private:
			T m_data;
			mutable Mutex m_mutex;
	};

// Provide class template argument deduction guide (CTAD) to silence the warning:
// "'shared_data_ptr' may not intend to support class template argument deduction [-Wctad-maybe-unsupported]"

// template <typename T, typename Mutex>
// 	shared_data_ptr(shared_data<T, Mutex>) -> shared_data_ptr<T, Mutex>;

// Provide class template argument deduction guide (CTAD) to silence the warning:
// "'const_shared_data_ptr' may not intend to support class template argument deduction [-Wctad-maybe-unsupported]"

// template <typename T, typename Mutex>
// 	const_shared_data_ptr(shared_data<T, Mutex>) -> const_shared_data_ptr<T, Mutex>;

}
