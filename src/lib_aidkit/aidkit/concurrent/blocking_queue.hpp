// Copyright 2023 Peter Most, PERA Software Solutions GmbH
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
#include <optional>
#include <queue>
#include <condition_variable>

namespace aidkit::concurrent {

template <typename T>
class blocking_queue
{
	public:
		blocking_queue() = default;

		blocking_queue(const blocking_queue &) = delete;
		blocking_queue &operator = (const blocking_queue &) = delete;

		void enqueue(const T &item)
		{
			std::unique_lock lock(m_mutex);

			m_items.push(item);
			m_items_available.notify_one();
		}

		void enqueue(T &&item)
		{
			std::unique_lock lock(m_mutex);

			m_items.push(std::move(item));
			m_items_available.notify_one();
		}

		T dequeue()
		{
			std::unique_lock lock(m_mutex);

			m_items_available.wait(lock, [this] { return !m_items.empty(); });

			T item(std::move(m_items.front()));
			m_items.pop();

			return item;
		}

		std::optional<T> dequeue(const std::chrono::milliseconds &timeout)
		{
			std::unique_lock lock(m_mutex);

			if (m_items_available.wait_for(lock, timeout, [this] { return !m_items.empty(); }))
			{
				std::optional<T> item(std::move(m_items.front()));
				m_items.pop();

				return item;
			}
			else
			{
				return std::nullopt;
			}
		}

	private:
		std::mutex m_mutex;
		std::queue<T> m_items;
		std::condition_variable m_items_available;
};

}
