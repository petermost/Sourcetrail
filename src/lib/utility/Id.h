#ifndef ID_H
#define ID_H

#include <QMetaType>

#include <concepts>
#include <cstddef>
#include <iosfwd>
#include <string>
#include <type_traits>

class Id final {
public:
	using type = long long;

	constexpr Id() noexcept
		: m_value(0)
	{}

	constexpr Id(type value) noexcept
		: m_value(value)
	{
	}

	Id operator ++ () noexcept
	{
		return ++m_value;
	}

	Id operator ++ (int) noexcept
	{
		return m_value++;
	}

	Id operator += (const type value) noexcept
	{
		return m_value += value;
	}

	constexpr bool operator < (const Id other) const noexcept
	{
		return m_value < other.m_value;
	}

	constexpr bool operator > (const Id other) const noexcept
	{
		return m_value > other.m_value;
	}

	constexpr bool operator == (const Id other) const noexcept
	{
		return m_value == other.m_value;
	}

	constexpr bool operator != (const Id other) const noexcept
	{
		return m_value != other.m_value;
	}

	constexpr explicit operator bool () const noexcept
	{
		return m_value != 0;
	}

	template <std::integral T>
	constexpr explicit operator T() const noexcept
	{
		static_assert(sizeof(T) >= sizeof(type));

		return m_value;
	}

private:
	type m_value;
};

Q_DECLARE_METATYPE(Id)

// Collision prevention:

enum class CollisionGuardBits : Id::type
{
	ONE   = ~(~Id::type(0) >> 1),
	TWO   = ~(~Id::type(0) >> 2),
	THREE = ~(~Id::type(0) >> 3)
};

constexpr Id addCollisionGuardBits(Id id, CollisionGuardBits bits)
{
    return static_cast<Id::type>(id) | static_cast<Id::type>(bits);
}

constexpr Id addCollisionGuardOffset(Id id)
{
	return static_cast<Id::type>(id) * 10'000;
}

std::string to_string(const Id id);

std::ostream &operator << (std::ostream &os, const Id id);

// Support for unordered_map:
namespace std
{
	template<>
		struct hash<Id> {
			public:
				std::size_t operator()(const Id id) const noexcept;

			private:
				std::hash<Id::type> m_hash;
		};
}

#endif
