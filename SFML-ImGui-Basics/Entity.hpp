#pragma once

#include "Components.hpp"
#include <string>
#include <tuple>

class EntityManager;

using ComponentTuple = std::tuple<
	CTransform,
	CCollision,
	CShape,
	CScore,
	CLifespan,
	CInput
>;

class Entity
{
	friend class EntityManager;

	ComponentTuple mComponents;
	bool mActive = true;
	std::string mTag = "default";
	size_t mId = 0;

	Entity(const size_t& id, const std::string& tag) : mTag(tag), mId(id)
	{

	}

public:

	bool isActive() const
	{
		return mActive;
	}

	void destroy()
	{
		mActive = false;
	}

	size_t id() const
	{
		return mId;
	}

	const std::string& tag() const
	{
		return mTag;
	}

	template<typename T>

	bool has() const
	{
		return get<T>().exists;
	}

	template <typename T, typename... TArgs>

	T& add(TArgs&&... mArgs)
	{
		auto& component = get<T>();
		component = T(std::forward<TArgs>(mArgs)...);
		component.exists = true;
		return component;
	}

	template<typename T>
	T& get()
	{
		return std::get<T>(mComponents);
	}

	template<typename T>
	const T& get() const
	{
		return std::get<T>(mComponents);
	}

	template<typename T>
	void remove()
	{
		get<T>() = T();
	}
};