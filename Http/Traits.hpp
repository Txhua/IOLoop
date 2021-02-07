#ifndef _IOEVENT_HTTP_TRAITS_H
#define _IOEVENT_HTTP_TRAITS_H


#include <type_traits>


namespace IOEvent
{
namespace detail
{

template <typename _T>
struct warp
{
	using type = _T;
};
template <typename _T>
_T value(warp<_T>);

template <typename ...>
using void_t = void;


template<typename ...>
struct conjunction;

template<>
struct conjunction<> : std::true_type
{
};

template<typename T1>
struct conjunction<T1> : T1
{
};

template<typename T1, typename T2>
struct conjunction<T1, T2>
	: std::conditional<T1::value, T2, T1>::type
{
};

template<typename T1, typename T2, typename T3, typename... Tn>
struct conjunction<T1, T2, T3, Tn...>
	: std::conditional<T1::value, conjunction<T2, T3, Tn...>, T1>::type
{
};

template <typename, typename = void_t<>>
struct invoke_result
{

};

template <typename _Func, typename ...Args>
struct invoke_result<_Func(Args...), void_t<decltype(std::declval<_Func>()(std::declval<Args>()...))>>
{
	using type = decltype(std::declval<_Func>()(std::declval<Args>()...));
};

template <typename _Ret, typename _Func, typename ...Args>
std::false_type is_invocable_impl(_Func&&, long, Args &&...args);


template <typename _Ret, typename _Func, typename ...Args>
auto is_invocable_impl(_Func&& func, int, Args &&...args)
	-> typename std::enable_if<std::is_convertible<typename invoke_result<decltype(func)(decltype(args)...)>::type, _Ret>::value || std::is_same<_Ret, void>::value, std::true_type>::type;

template <typename, typename>
struct is_invocable : std::false_type
{

};

template <typename _Func, typename _Ret, typename ...Args>
struct is_invocable<_Func, _Ret(Args...)>
	:decltype(is_invocable_impl<_Ret>(std::declval<_Func>(), 1, std::declval<Args>()...))
{

};

template <typename _Ret, typename _Func>
struct validity_checker
{
	template <typename ...Args>
	constexpr auto operator()(Args &&...args)
	{
		return is_invocable<_Func, _Ret(decltype(args)...)>{};
	}
};

template <typename _Ret, typename _Func>
constexpr auto isValid(_Func &&)
{
	return validity_checker<_Ret, _Func>{};
}

template <typename _Func, typename _Ret, typename ...Args>
struct invoke :
	is_invocable<_Func, _Ret(Args...)>
{

};

template <typename _Ret, typename _Func, typename ...Args>
constexpr auto tryInvoke(_Func&&, Args &&...) -> decltype(invoke<_Func, _Ret, Args...>{});

template <typename _Ret, typename _Type>
constexpr auto HasStorageType(warp<_Type> x)
{
	return isValid<_Ret>([](auto x)->typename decltype(value(x))::storage_type {})(x);
}


template <typename _Ret, typename _Type>
constexpr auto HasResourceMapType(warp<_Type> x)
{
	return isValid<_Ret>([](auto x)->typename decltype(value(x))::resource_map_type{})(x);
}
	
}

template <typename, typename>
struct TryInvoke;

template <typename _Ret, typename _Func, typename ...Args>
struct TryInvoke<_Func, _Ret(Args...)>
	:decltype(detail::tryInvoke<_Ret>(std::declval<_Func>(), std::declval<Args>()...))
{

};

template<unsigned FnCount, typename Func1, typename Func2, typename Function, typename... Args>
struct TryInvokeConjunction
	: TryInvokeConjunction<FnCount - 1, Func1, Func2, Args...>
{
	static constexpr bool value = TryInvoke<Function, Func1>::value && TryInvokeConjunction<FnCount - 1, Func1, Func2, Args...>::value;
};

template<class Func1, class Func2, class Function>
struct TryInvokeConjunction<0, Func1, Func2, Function>
{
	static constexpr bool value = TryInvoke<Function, Func2>::value;
};

template <typename _Type, typename _Ret>
using HasStorageType = decltype(detail::HasStorageType<_Ret>(detail::warp<_Type>()));

template <typename _Type, typename _Ret>
using HasResourceMapType = decltype(detail::HasResourceMapType<_Ret>(detail::warp<_Type>()));



}








#endif // !_IOEVENT_HTTP_TRAITS_H