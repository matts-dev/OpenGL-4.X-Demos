#pragma once

#include<memory>

template<typename T>
using sp = std::shared_ptr<T>;

//weak pointer
template<typename T>
using wp = std::weak_ptr<T>;

//unique pointer
template<typename T>
using up = std::unique_ptr<T>;

class Entity
{
	template<typename T, typename... Args> 
	friend sp<T> new_sp(Args&&... args);
public:
	virtual ~Entity() {};
protected:
	virtual void postConstruct() {}
};


template<typename T, typename... Args>
sp<T> new_sp(Args&&... args)
{
	if constexpr (std::is_base_of<Entity, T>::value)
	{
		sp<T> newObj = std::make_shared<T>(std::forward<Args>(args)...);

		//safe cast because of type-trait
		Entity* newEntity = static_cast<Entity*>(newObj.get());
		newEntity->postConstruct();
		return newObj;
	}
	else
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
up<T> new_up(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

#define sp_this() shared_from_this()

//template<typename T, typename K>
//sp<T> sp_as(const sp<K>& toConvert)
//{
//	static_assert(std::is_base_of<decltype(toConvert::element_type), T>::value, "attempting to make shared this to an unrealted type -- did you inherit correctly?");
//	return std::static_pointer_cast<T>(toConvert);
//}
//#define sp_this() sp_this_as<std::remove_reference<decltype(*this)>::type>()
