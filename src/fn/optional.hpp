#ifndef _adb97dcb_a440_45a7_a9d4_3b33e43100d4
#define _adb97dcb_a440_45a7_a9d4_3b33e43100d4

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif

namespace fn
{

template<typename T> class optional;

namespace fn_ {

/*
 *  remove reference from a type, similar to std::remove_reference
 */
template<class R> struct remove_reference      {typedef R T;};
template<class R> struct remove_reference<R&>  {typedef R T;};
template<class R> struct remove_reference<R&&> {typedef R T;};


/*
 *  cast to rvalue reference, similar to std::move
 */
template <class T>
typename remove_reference<T>::T&&
move(T&& a)
{
    return ((typename remove_reference<T>::T&&)a);
}

template<typename T> class optional_ref;
template<typename T> class optional_value;

template<typename T> struct return_cast;

/*
 *  This helper allows for easier usage of a functions return type
 *  in a type deduction context.
 *  Additionally it works around a strange msvc internal compiler error.
 */
template<class F, class T>
static auto return_type(F&& f,T&& value)->decltype(f(value))
{
    return f(value);
}


template<typename T>
class optional_base
{
public:
    typedef T Type;
    bool valid() const { return !!value; }

protected:
    optional_base(T* const p):
        value(p)
    {}

private:
    friend class optional<T const&>;
    friend class optional<T const>;
    friend class optional<T&>;
    friend class optional<T>;
    friend class optional_value<T>;
    friend class optional_ref<T>;

    T* value = nullptr;

public:
    template<typename F>
    T operator|(F fallback) const
    {
        return valid() ? *value : fallback;
    }

    bool operator==(T const& other_value) const
    {
        return valid() && (*value == other_value);
    }

    bool operator!=(T const& other_value) const
    {
        return !((*this) == other_value);
    }

    bool operator==(optional_base const& other) const
    {
        return (valid() && other.valid()) && (*value == *other.value);
    }

    bool operator!=(optional_base const& other) const
    {
        return !(*this == other);
    }

    template<typename EmptyF>
    auto operator||(EmptyF const& handle_no_value) const
        ->decltype(handle_no_value())
    {
        if(!valid()){
            return handle_no_value();
        }
        else {
            return return_cast<decltype(handle_no_value())>::value(*value);
        }
    }

    T operator~() const
    {
        return (*this) | T{};
    }
};

template<typename T>
class optional_value : public optional_base<T>
{
    friend class optional<T>;
    friend class optional<T const>;
    friend class optional<T const&>;
    friend class optional<T&>;

    using optional_base<T>::value;

protected:
    unsigned char value_mem[sizeof(T)];

    optional_value():
        optional_base<T>(nullptr)
    {}

    optional_value(T* v) :
        optional_base<T>(v)
    {}

    ~optional_value()
    {
        if(optional_base<T>::valid()){
            reinterpret_cast<T*>(value_mem)->~T();
        }
    }

public:
    using optional_base<T>::valid;

    template<typename ValueF>
    auto operator>>(ValueF const& handle_value) const
        ->decltype(
            return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value)
        )
    {
        if(valid()){
            return return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value);
        }
        else{
            return {};
        }
    }
};

template<typename T>
class optional_ref : public optional_base<T>
{
    friend class optional<T>;
    friend class optional<T const>;
    friend class optional<T const&>;
    friend class optional<T&>;

    using optional_base<T>::value;

protected:
    optional_ref():
        optional_base<T>(nullptr)
    {}

    optional_ref(T* v):
        optional_base<T>(v)
    {}

    optional_ref(T&& value):
        optional_base<T>(true,value)
    {}

public:
    template<typename F>
    T& operator|(F& fallback) const
    {
        if(optional_base<T>::valid()){
            return *optional_base<T>::value;
        }
        else{
            return fallback;
        }
    }

    template<typename F>
    T operator|(F const& fallback) const
    {
        if(optional_base<T>::valid()){
            return *optional_base<T>::value;
        }
        else{
            return fallback;
        }
    }

    optional_ref& operator=(optional_ref const& other)
    {
        optional_base<T>::value = other.value;
        return *this;
    }

public:
    using optional_base<T>::valid;

    template<typename ValueF>
    auto operator>>(
        ValueF const& handle_value) const
        ->decltype(
            return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value)
        )
    {
        if(valid()){
            return return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value);
        }
        else{
            return {};
        }
    }
};

}

template<typename T>
class optional final : public fn_::optional_value<T>
{
    using fn_::optional_value<T>::value_mem;

    friend class optional<T const>;
    friend class optional<T const&>;
    friend class optional<T&>;

public:
    optional():
        fn_::optional_value<T>()
    {}

    optional(T&& v):
        fn_::optional_value<T>(reinterpret_cast<T*>(value_mem))
    {
        new (value_mem) T{fn_::move(v)};
    }

    optional(optional<T>&& original):
        fn_::optional_value<T>(
            original.valid()
                ? reinterpret_cast<T*>(value_mem)
                : nullptr
        )
    {
        if(original.valid()){
            new (value_mem) T{fn_::move(*original.value)};
            original.value = nullptr;
            reinterpret_cast<T*>(original.value_mem)->~T();
        }
    }

    optional& operator=(optional&& other)
    {
        if(other.valid()){
            if(fn_::optional_value<T>::valid()){
                *fn_::optional_value<T>::value = fn_::move(*other.value);
            }
            else{
                new (value_mem) T{fn_::move(*other.value)};
                fn_::optional_value<T>::value = reinterpret_cast<T*>(value_mem);
            }
        }
        else{
            if(fn_::optional_value<T>::valid()){
                reinterpret_cast<T*>(value_mem)->~T();
            }
            fn_::optional_value<T>::value = nullptr;
        }
        return *this;
    }

    optional& operator=(optional const& other)
    {
        if(other.valid()){
            if(fn_::optional_value<T>::valid()){
                *fn_::optional_value<T>::value = *other.value;
            }
            else{
                new (value_mem) T{*other.value};
                fn_::optional_value<T>::value = reinterpret_cast<T*>(value_mem);
            }
        }
        else{
            if(fn_::optional_value<T>::valid()){
                reinterpret_cast<T*>(value_mem)->~T();
            }
            fn_::optional_value<T>::value = nullptr;
        }
        return *this;
    }

    optional(T const& v):
        fn_::optional_value<T>(reinterpret_cast<T*>(value_mem))
    {
        new (value_mem) T{v};
    }

    optional(optional<T&> const& original):
        fn_::optional_value<T>(
            original.valid()
                ? reinterpret_cast<T*>(value_mem)
                : nullptr
        )
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T const&> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T const> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }
};

template<typename T>
class optional<T const> final : public fn_::optional_value<T const>
{
    using fn_::optional_value<T const>::value_mem;

    friend class optional<T>;

public:
    optional():
        fn_::optional_value<T const>()
    {}

    optional(T const& v):
        fn_::optional_value<T const>(reinterpret_cast<T*>(value_mem))
    {
        new (value_mem) T{v};
    }

    optional(optional<T>&& original):
        fn_::optional_value<T const>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        if(original.valid()){
            new (value_mem) T{fn_::move(*original.value)};
            original.value = nullptr;
            reinterpret_cast<T*>(value_mem)->~T();
        }
    }

    optional(optional<T> const& original):
        fn_::optional_value<T const>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T&> const& original):
        fn_::optional_value<T const>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }
};

template<typename T>
class optional<T&> final : public fn_::optional_ref<T>
{
    friend class optional<T const&>;
    friend class optional<T const>;
    friend class optional<T>;

public:
    optional():
        fn_::optional_ref<T>()
    {}

    optional(T& v):
        fn_::optional_ref<T>(&v)
    {}

    optional(optional<T> const& original):
        fn_::optional_ref<T>(original.value)
    {}
};

template<typename T>
class optional<T const&> final : public fn_::optional_ref<T const>
{
    friend class optional<T const>;
    friend class optional<T>;

public:
    optional():
        fn_::optional_ref<T const>()
    {}

    optional(T const& v):
        fn_::optional_ref<T const>(&v)
    {}

    optional(optional<T> const& original):
        fn_::optional_ref<T const>(original.value)
    {}

    optional(optional<T&> const& original):
        fn_::optional_ref<T const>(original.value)
    {}
};

template<>
class optional<void> final
{
    bool no_value;
public:
    optional(): no_value(true) {}
    optional(bool): no_value(false) {}

    template<typename EmptyF>
    void operator||(EmptyF const& handle_no_value) const
    {
        if(no_value){
            return handle_no_value();
        }
    }
};

namespace fn_ {

/*
 *  Convert a functions return value to an optional.
 *  This is needed for optional handler chaining.
 *  In particular a specialization is implemented that
 *  avoids getting optional<optional<T>>.
 */
template<typename T>
struct return_cast {
    template<typename F,typename  V>
    static optional<T> func(F&& f,V&& v) {return f(v);}

    template<typename V>
    static T value(V&& v) {return v;}
};

template<>
struct return_cast<void> {
    template<typename F,typename  V>
    static
    optional<void> func(F&& f,V&& v) {f(v); return true;}

    template<typename V>
    static void value(V&&) {}
};

template<typename T>
struct return_cast<optional<T>> {

    template<typename F,typename  V>
    static
    optional<T> func(F&& f,V&& v) {return f(v);}

    template<typename V>
    static T value(V&& v) {return v;}
};

}

}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif
