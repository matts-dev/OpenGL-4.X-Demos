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

template<typename T, typename... Args>
sp<T> new_sp(Args&&... args)
{
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
