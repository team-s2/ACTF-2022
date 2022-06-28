#include <type_traits>

struct GetZero {
    typedef int type;
};

struct GetOne {
    typedef float type;
};

template <bool B, typename T, typename F> struct Conditional {
    typedef F type;
};

template <typename T, typename F> struct Conditional<true, T, F> {
    typedef T type;
};

template <int N> struct IntConstant { static constexpr int value = N; };

template <bool B> struct BoolConstant { static constexpr bool value = B; };

typedef BoolConstant<false> FalseType;
typedef BoolConstant<true> TrueType;

template <typename T> struct Negation : BoolConstant<!bool(T::value)> {};

template <typename...> struct Conjunction : TrueType {};

template <typename T> struct Conjunction<T> : T {};

template <typename T, typename... U>
struct Conjunction<T, U...>
    : Conditional<bool(T::value), Conjunction<U...>, T>::type {};

template <typename T, typename... U>
struct Exclusive : Conditional<bool(T::value), Negation<Exclusive<U...>>,
    Exclusive<U...>>::type{};

template <typename T> struct Exclusive<T> : T {};

template <bool B, typename = void> struct EnableIf {};

template <typename T> struct EnableIf<true, T> { typedef T type; };

template <typename T, typename U> struct IsSame : FalseType {};

template <typename T> struct IsSame<T, T> : TrueType {};

template <typename T> struct ClearBit2 {
    typedef typename std::remove_reference<T>::type type;
};

template <typename T> struct SetBit2 {
    typedef typename std::add_lvalue_reference<T>::type type;
};

template <typename T, typename = void> struct TestBit2 : FalseType {};

template <typename T>
struct TestBit2<T, typename EnableIf<std::is_reference<T>::value>::type>
    : TrueType {};

template <typename T, typename = void> struct ClearBit1 {
    typedef typename std::remove_const<T>::type type;
};

template <typename T>
struct ClearBit1<T, typename EnableIf<TestBit2<T>::value>::type> {
    typedef typename SetBit2<
        typename std::remove_const<typename ClearBit2<T>::type>::type>::type type;
};

template <typename T, typename = void> struct SetBit1 {
    typedef typename std::add_const<T>::type type;
};

template <typename T>
struct SetBit1<T, typename EnableIf<TestBit2<T>::value>::type> {
    typedef typename SetBit2<
        typename std::add_const<typename ClearBit2<T>::type>::type>::type type;
};

template <typename T, typename = void> struct TestBit1 : FalseType {};

template <typename T>
struct TestBit1<T, typename EnableIf<
    std::is_const<typename ClearBit2<T>::type>::value>::type>
    : TrueType {};

template <typename T, typename = void> struct ClearBit0 {
    typedef GetZero::type type;
};

template <typename T>
struct ClearBit0<T, typename EnableIf<Conjunction<
    TestBit1<T>, Negation<TestBit2<T>>>::value>::type> {
    typedef typename SetBit1<GetZero::type>::type type;
};

template <typename T>
struct ClearBit0<T, typename EnableIf<Conjunction<Negation<TestBit1<T>>,
    TestBit2<T>>::value>::type> {
    typedef typename SetBit2<GetZero::type>::type type;
};

template <typename T>
struct ClearBit0<
    T, typename EnableIf<Conjunction<TestBit1<T>, TestBit2<T>>::value>::type> {
    typedef typename SetBit2<typename SetBit1<GetZero::type>::type>::type type;
};

template <typename T, typename = void> struct SetBit0 {
    typedef GetOne::type type;
};

template <typename T>
struct SetBit0<T, typename EnableIf<Conjunction<
    TestBit1<T>, Negation<TestBit2<T>>>::value>::type> {
    typedef typename SetBit1<GetOne::type>::type type;
};

template <typename T>
struct SetBit0<T, typename EnableIf<Conjunction<Negation<TestBit1<T>>,
    TestBit2<T>>::value>::type> {
    typedef typename SetBit2<GetOne::type>::type type;
};

template <typename T>
struct SetBit0<
    T, typename EnableIf<Conjunction<TestBit1<T>, TestBit2<T>>::value>::type> {
    typedef typename SetBit2<typename SetBit1<GetOne::type>::type>::type type;
};

template <typename T, typename = void> struct TestBit0 : FalseType {};

template <typename T>
struct TestBit0<T, typename EnableIf<IsSame<
    typename ClearBit1<typename ClearBit2<T>::type>::type,
    GetOne::type>::value>::type> : TrueType {};

template <typename T, typename U, typename = void> struct XorBit0 {
    typedef GetOne::type type;
};

template <typename T, typename U>
struct XorBit0<T, U,
    typename EnableIf<Negation<
    Exclusive<TestBit0<T>, TestBit0<U>>>::value>::type> {
    typedef GetZero::type type;
};

template <typename T, typename U, typename = void> struct XorBit1 {
    typedef typename SetBit1<typename XorBit0<T, U>::type>::type type;
};

template <typename T, typename U>
struct XorBit1<T, U,
    typename EnableIf<Negation<
    Exclusive<TestBit1<T>, TestBit1<U>>>::value>::type> {
    typedef typename ClearBit1<typename XorBit0<T, U>::type>::type type;
};

template <typename T, typename U, typename = void> struct XorBit2 {
    typedef typename SetBit2<typename XorBit1<T, U>::type>::type type;
};

template <typename T, typename U>
struct XorBit2<T, U,
    typename EnableIf<Negation<
    Exclusive<TestBit2<T>, TestBit2<U>>>::value>::type> {
    typedef typename ClearBit2<typename XorBit1<T, U>::type>::type type;
};

template <typename T, typename... U> struct Add {
    typedef typename XorBit2<T, typename Add<U...>::type>::type type;
};

template <typename T> struct Add<T> { typedef T type; };

template <typename T> struct ReduceBit3Helper0 {
    typedef typename Conditional<TestBit2<T>::value, GetOne::type,
        GetZero::type>::type type;
};

template <typename T> struct ReduceBit3Helper1 {
    typedef typename Conditional<
        TestBit0<T>::value,
        typename SetBit1<typename ReduceBit3Helper0<T>::type>::type,
        typename ReduceBit3Helper0<T>::type>::type type;
};

template <typename T> struct ReduceBit3 {
    typedef typename Conditional<
        Exclusive<TestBit2<T>, TestBit1<T>>::value,
        typename SetBit2<typename ReduceBit3Helper1<T>::type>::type,
        typename ReduceBit3Helper1<T>::type>::type type;
};

template <typename T> struct ReduceBit4Helper0 {
    typedef typename Conditional<Exclusive<TestBit2<T>, TestBit1<T>>::value,
        GetOne::type, GetZero::type>::type type;
};

template <typename T> struct ReduceBit4Helper1 {
    typedef typename Conditional<
        TestBit2<T>::value,
        typename SetBit1<typename ReduceBit4Helper0<T>::type>::type,
        typename ReduceBit4Helper0<T>::type>::type type;
};

template <typename T> struct ReduceBit4 {
    typedef typename Conditional<
        Exclusive<TestBit2<T>, TestBit1<T>, TestBit0<T>>::value,
        typename SetBit2<typename ReduceBit4Helper1<T>::type>::type,
        typename ReduceBit4Helper1<T>::type>::type type;
};

template <typename T, typename U, typename = void> struct MulBit0 {
    typedef GetZero::type type;
};

template <typename T, typename U>
struct MulBit0<T, U, typename EnableIf<TestBit0<U>::value>::type> {
    typedef T type;
};

template <typename T, typename U, typename = void> struct MulBit1 {
    typedef typename MulBit0<T, U>::type type;
};

template <typename T, typename U>
struct MulBit1<T, U, typename EnableIf<TestBit1<U>::value>::type> {
    typedef typename Add<typename ReduceBit3<T>::type,
        typename MulBit0<T, U>::type>::type type;
};

template <typename T, typename U, typename = void> struct MulBit2 {
    typedef typename MulBit1<T, U>::type type;
};

template <typename T, typename U>
struct MulBit2<T, U, typename EnableIf<TestBit2<U>::value>::type> {
    typedef typename Add<typename ReduceBit4<T>::type,
        typename MulBit1<T, U>::type>::type type;
};

template <typename T, typename... U> struct Mul {
    typedef typename MulBit2<T, typename Mul<U...>::type>::type type;
};

template <typename T> struct Mul<T> { typedef T type; };

template <typename T> struct MulZero {
    typedef typename Mul<GetZero::type, T>::type type;
};

template <typename T> struct MulOne {
    typedef typename Mul<GetOne::type, T>::type type;
};

template <typename T> struct MulTwo {
    typedef typename Mul<SetBit1<GetZero::type>::type, T>::type type;
};

template <typename T> struct MulThree {
    typedef typename Mul<SetBit1<GetOne::type>::type, T>::type type;
};

template <typename T> struct MulFour {
    typedef typename Mul<SetBit1<GetZero::type>::type, T,
        SetBit1<GetZero::type>::type>::type type;
};

template <typename T> struct MulFive {
    typedef typename Mul<SetBit1<GetOne::type>::type, SetBit1<GetOne::type>::type,
        T>::type type;
};

template <typename T> struct MulSix {
    typedef typename Mul<SetBit1<GetZero::type>::type, T,
        SetBit1<GetOne::type>::type>::type type;
};

template <typename T> struct MulSeven {
    typedef
        typename Mul<SetBit1<GetZero::type>::type, T,
        SetBit1<GetZero::type>::type, SetBit1<GetZero::type>::type,
        SetBit1<GetZero::type>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression0Part7 {
    typedef typename Add<typename MulFour<U>::type, typename MulSix<V>::type,
        GetZero::type, typename MulSeven<W>::type,
        typename MulTwo<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression1Part7 {
    typedef typename Add<typename MulZero<U>::type, typename MulZero<V>::type,
        GetZero::type, typename MulTwo<W>::type,
        typename MulSix<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression2Part7 {
    typedef typename Add<typename MulSix<U>::type, typename MulTwo<V>::type,
        GetZero::type, typename MulFive<W>::type,
        typename MulFour<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression3Part7 {
    typedef typename Add<typename MulThree<U>::type, typename MulSeven<V>::type,
        GetZero::type, typename MulSeven<W>::type,
        typename MulFive<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression4Part7 {
    typedef typename Add<typename MulThree<U>::type, typename MulOne<V>::type,
        GetZero::type, typename MulSix<W>::type,
        typename MulFive<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression5Part7 {
    typedef typename Add<typename MulZero<U>::type, typename MulZero<V>::type,
        GetZero::type, typename MulFour<W>::type,
        typename MulSix<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression6Part7 {
    typedef typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulOne<W>::type,
        typename MulFive<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression7Part7 {
    typedef typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulThree<W>::type,
        typename MulTwo<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression8Part7 {
    typedef typename Add<typename MulTwo<U>::type, typename MulOne<V>::type,
        GetZero::type, typename MulOne<W>::type,
        typename MulOne<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression9Part7 {
    typedef typename Add<typename MulOne<U>::type, typename MulTwo<V>::type,
        GetZero::type, typename MulFour<W>::type,
        typename MulFour<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression10Part7 {
    typedef typename Add<typename MulThree<U>::type, typename MulTwo<V>::type,
        GetZero::type, typename MulFour<W>::type,
        typename MulSeven<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression11Part7 {
    typedef typename Add<typename MulFour<U>::type, typename MulOne<V>::type,
        GetZero::type, typename MulTwo<W>::type,
        typename MulSix<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression12Part7 {
    typedef typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulZero<W>::type,
        typename MulThree<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression13Part7 {
    typedef typename Add<typename MulOne<U>::type, typename MulSix<V>::type,
        GetZero::type, typename MulFive<W>::type,
        typename MulSix<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression14Part7 {
    typedef typename Add<typename MulThree<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulSeven<W>::type,
        typename MulSix<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression15Part7 {
    typedef typename Add<typename MulFive<U>::type, typename MulSeven<V>::type,
        GetZero::type, typename MulOne<W>::type,
        typename MulOne<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression16Part7 {
    typedef typename Add<typename MulOne<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulZero<W>::type,
        typename MulThree<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression17Part7 {
    typedef typename Add<typename MulOne<U>::type, typename MulThree<V>::type,
        GetZero::type, typename MulFour<W>::type,
        typename MulSeven<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression18Part7 {
    typedef typename Add<typename MulTwo<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulThree<W>::type,
        typename MulFive<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression19Part7 {
    typedef typename Add<typename MulSix<U>::type, typename MulZero<V>::type,
        GetZero::type, typename MulFour<W>::type,
        typename MulSeven<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression20Part7 {
    typedef typename Add<typename MulFour<U>::type, typename MulTwo<V>::type,
        GetZero::type, typename MulZero<W>::type,
        typename MulFive<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression21Part7 {
    typedef typename Add<typename MulOne<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulZero<W>::type,
        typename MulSix<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression22Part7 {
    typedef typename Add<typename MulSeven<U>::type, typename MulSeven<V>::type,
        GetZero::type, typename MulSix<W>::type,
        typename MulFive<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression23Part7 {
    typedef typename Add<typename MulOne<U>::type, typename MulOne<V>::type,
        GetZero::type, typename MulSeven<W>::type,
        typename MulOne<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression24Part7 {
    typedef typename Add<typename MulFive<U>::type, typename MulTwo<V>::type,
        GetZero::type, typename MulOne<W>::type,
        typename MulOne<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression25Part7 {
    typedef typename Add<typename MulSix<U>::type, typename MulOne<V>::type,
        GetZero::type, typename MulFive<W>::type,
        typename MulThree<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression26Part7 {
    typedef typename Add<typename MulZero<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulFive<W>::type,
        typename MulFour<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression27Part7 {
    typedef typename Add<typename MulZero<U>::type, typename MulOne<V>::type,
        GetZero::type, typename MulOne<W>::type,
        typename MulSix<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression28Part7 {
    typedef typename Add<typename MulFour<U>::type, typename MulZero<V>::type,
        GetZero::type, typename MulSeven<W>::type,
        typename MulThree<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression29Part7 {
    typedef typename Add<typename MulSeven<U>::type, typename MulFour<V>::type,
        GetZero::type, typename MulOne<W>::type,
        typename MulOne<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression30Part7 {
    typedef typename Add<typename MulZero<U>::type, typename MulSix<V>::type,
        GetZero::type, typename MulThree<W>::type,
        typename MulThree<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X>
struct Expression31Part7 {
    typedef typename Add<typename MulTwo<U>::type, typename MulSeven<V>::type,
        GetZero::type, typename MulOne<W>::type,
        typename MulOne<X>::type>::type type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression0Part6 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulOne<V>::type,
        typename Expression0Part7<Y...>::type,
        typename MulThree<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression1Part6 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        typename Expression1Part7<Y...>::type,
        typename MulSix<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression2Part6 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulZero<V>::type,
        typename Expression2Part7<Y...>::type,
        typename MulOne<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression3Part6 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulOne<V>::type,
        typename Expression3Part7<Y...>::type,
        typename MulSix<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression4Part6 {
    typedef typename Add<typename MulTwo<U>::type, typename MulOne<V>::type,
        typename Expression4Part7<Y...>::type,
        typename MulTwo<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression5Part6 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulZero<V>::type,
        typename Expression5Part7<Y...>::type,
        typename MulSeven<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression6Part6 {
    typedef typename Add<typename MulFour<U>::type, typename MulFive<V>::type,
        typename Expression6Part7<Y...>::type,
        typename MulTwo<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression7Part6 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulSix<V>::type,
        typename Expression7Part7<Y...>::type,
        typename MulZero<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression8Part6 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulFour<V>::type,
        typename Expression8Part7<Y...>::type,
        typename MulSeven<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression9Part6 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulZero<V>::type,
        typename Expression9Part7<Y...>::type,
        typename MulThree<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression10Part6 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulTwo<V>::type,
        typename Expression10Part7<Y...>::type,
        typename MulFive<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression11Part6 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFive<V>::type,
        typename Expression11Part7<Y...>::type,
        typename MulTwo<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression12Part6 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulZero<V>::type,
        typename Expression12Part7<Y...>::type,
        typename MulSeven<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression13Part6 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulTwo<V>::type,
        typename Expression13Part7<Y...>::type,
        typename MulSix<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression14Part6 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulTwo<V>::type,
        typename Expression14Part7<Y...>::type,
        typename MulSix<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression15Part6 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulThree<V>::type,
        typename Expression15Part7<Y...>::type,
        typename MulSeven<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression16Part6 {
    typedef typename Add<typename MulThree<U>::type, typename MulTwo<V>::type,
        typename Expression16Part7<Y...>::type,
        typename MulSix<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression17Part6 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulThree<V>::type,
        typename Expression17Part7<Y...>::type,
        typename MulZero<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression18Part6 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFive<V>::type,
        typename Expression18Part7<Y...>::type,
        typename MulFour<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression19Part6 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulSeven<V>::type,
        typename Expression19Part7<Y...>::type,
        typename MulOne<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression20Part6 {
    typedef typename Add<typename MulSix<U>::type, typename MulSix<V>::type,
        typename Expression20Part7<Y...>::type,
        typename MulSix<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression21Part6 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulSeven<V>::type,
        typename Expression21Part7<Y...>::type,
        typename MulFive<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression22Part6 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulFive<V>::type,
        typename Expression22Part7<Y...>::type,
        typename MulFive<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression23Part6 {
    typedef typename Add<typename MulFour<U>::type, typename MulFive<V>::type,
        typename Expression23Part7<Y...>::type,
        typename MulOne<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression24Part6 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulFive<V>::type,
        typename Expression24Part7<Y...>::type,
        typename MulThree<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression25Part6 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulThree<V>::type,
        typename Expression25Part7<Y...>::type,
        typename MulSix<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression26Part6 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulSeven<V>::type,
        typename Expression26Part7<Y...>::type,
        typename MulFive<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression27Part6 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulFive<V>::type,
        typename Expression27Part7<Y...>::type,
        typename MulFour<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression28Part6 {
    typedef typename Add<typename MulOne<U>::type, typename MulSeven<V>::type,
        typename Expression28Part7<Y...>::type,
        typename MulOne<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression29Part6 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulOne<V>::type,
        typename Expression29Part7<Y...>::type,
        typename MulFour<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression30Part6 {
    typedef typename Add<typename MulSix<U>::type, typename MulTwo<V>::type,
        typename Expression30Part7<Y...>::type,
        typename MulSix<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression31Part6 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulThree<V>::type,
        typename Expression31Part7<Y...>::type,
        typename MulSeven<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression0Part5 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulOne<V>::type,
        typename Expression0Part6<Y...>::type,
        typename MulFour<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression1Part5 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulTwo<V>::type,
        typename Expression1Part6<Y...>::type,
        typename MulSeven<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression2Part5 {
    typedef typename Add<typename MulTwo<U>::type, typename MulThree<V>::type,
        typename Expression2Part6<Y...>::type,
        typename MulSix<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression3Part5 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulSix<V>::type,
        typename Expression3Part6<Y...>::type,
        typename MulSix<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression4Part5 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulOne<V>::type,
        typename Expression4Part6<Y...>::type,
        typename MulSeven<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression5Part5 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulSix<V>::type,
        typename Expression5Part6<Y...>::type,
        typename MulOne<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression6Part5 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulTwo<V>::type,
        typename Expression6Part6<Y...>::type,
        typename MulZero<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression7Part5 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulThree<V>::type,
        typename Expression7Part6<Y...>::type,
        typename MulTwo<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression8Part5 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulOne<V>::type,
        typename Expression8Part6<Y...>::type,
        typename MulFive<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression9Part5 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulFive<V>::type,
        typename Expression9Part6<Y...>::type,
        typename MulSeven<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression10Part5 {
    typedef typename Add<typename MulSix<U>::type, typename MulThree<V>::type,
        typename Expression10Part6<Y...>::type,
        typename MulOne<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression11Part5 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulSeven<V>::type,
        typename Expression11Part6<Y...>::type,
        typename MulFive<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression12Part5 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulSeven<V>::type,
        typename Expression12Part6<Y...>::type,
        typename MulFive<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression13Part5 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulOne<V>::type,
        typename Expression13Part6<Y...>::type,
        typename MulThree<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression14Part5 {
    typedef typename Add<typename MulTwo<U>::type, typename MulSeven<V>::type,
        typename Expression14Part6<Y...>::type,
        typename MulSix<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression15Part5 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulZero<V>::type,
        typename Expression15Part6<Y...>::type,
        typename MulSeven<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression16Part5 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulOne<V>::type,
        typename Expression16Part6<Y...>::type,
        typename MulSix<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression17Part5 {
    typedef typename Add<typename MulSeven<U>::type, typename MulSeven<V>::type,
        typename Expression17Part6<Y...>::type,
        typename MulSix<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression18Part5 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulTwo<V>::type,
        typename Expression18Part6<Y...>::type,
        typename MulThree<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression19Part5 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        typename Expression19Part6<Y...>::type,
        typename MulThree<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression20Part5 {
    typedef typename Add<typename MulZero<U>::type, typename MulSeven<V>::type,
        typename Expression20Part6<Y...>::type,
        typename MulSix<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression21Part5 {
    typedef typename Add<typename MulOne<U>::type, typename MulTwo<V>::type,
        typename Expression21Part6<Y...>::type,
        typename MulTwo<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression22Part5 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulFour<V>::type,
        typename Expression22Part6<Y...>::type,
        typename MulZero<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression23Part5 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulThree<V>::type,
        typename Expression23Part6<Y...>::type,
        typename MulOne<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression24Part5 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulOne<V>::type,
        typename Expression24Part6<Y...>::type,
        typename MulFive<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression25Part5 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulOne<V>::type,
        typename Expression25Part6<Y...>::type,
        typename MulZero<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression26Part5 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSix<V>::type,
        typename Expression26Part6<Y...>::type,
        typename MulThree<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression27Part5 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulFour<V>::type,
        typename Expression27Part6<Y...>::type,
        typename MulFour<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression28Part5 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulZero<V>::type,
        typename Expression28Part6<Y...>::type,
        typename MulFive<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression29Part5 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulOne<V>::type,
        typename Expression29Part6<Y...>::type,
        typename MulThree<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression30Part5 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulThree<V>::type,
        typename Expression30Part6<Y...>::type,
        typename MulFive<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression31Part5 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulThree<V>::type,
        typename Expression31Part6<Y...>::type,
        typename MulTwo<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression0Part4 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulTwo<V>::type,
        typename Expression0Part5<Y...>::type,
        typename MulZero<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression1Part4 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulThree<V>::type,
        typename Expression1Part5<Y...>::type,
        typename MulFive<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression2Part4 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFour<V>::type,
        typename Expression2Part5<Y...>::type,
        typename MulSeven<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression3Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulSeven<V>::type,
        typename Expression3Part5<Y...>::type,
        typename MulThree<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression4Part4 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulZero<V>::type,
        typename Expression4Part5<Y...>::type,
        typename MulThree<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression5Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulOne<V>::type,
        typename Expression5Part5<Y...>::type,
        typename MulSix<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression6Part4 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulZero<V>::type,
        typename Expression6Part5<Y...>::type,
        typename MulZero<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression7Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulSeven<V>::type,
        typename Expression7Part5<Y...>::type,
        typename MulSeven<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression8Part4 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulOne<V>::type,
        typename Expression8Part5<Y...>::type,
        typename MulFour<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression9Part4 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSeven<V>::type,
        typename Expression9Part5<Y...>::type,
        typename MulTwo<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression10Part4 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulFive<V>::type,
        typename Expression10Part5<Y...>::type,
        typename MulFour<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression11Part4 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        typename Expression11Part5<Y...>::type,
        typename MulThree<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression12Part4 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulFive<V>::type,
        typename Expression12Part5<Y...>::type,
        typename MulFour<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression13Part4 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulTwo<V>::type,
        typename Expression13Part5<Y...>::type,
        typename MulFour<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression14Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulFour<V>::type,
        typename Expression14Part5<Y...>::type,
        typename MulSix<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression15Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulSeven<V>::type,
        typename Expression15Part5<Y...>::type,
        typename MulTwo<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression16Part4 {
    typedef typename Add<typename MulSix<U>::type, typename MulSix<V>::type,
        typename Expression16Part5<Y...>::type,
        typename MulSix<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression17Part4 {
    typedef typename Add<typename MulSix<U>::type, typename MulSix<V>::type,
        typename Expression17Part5<Y...>::type,
        typename MulSix<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression18Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulFour<V>::type,
        typename Expression18Part5<Y...>::type,
        typename MulFour<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression19Part4 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSix<V>::type,
        typename Expression19Part5<Y...>::type,
        typename MulSix<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression20Part4 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulSeven<V>::type,
        typename Expression20Part5<Y...>::type,
        typename MulSeven<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression21Part4 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulFive<V>::type,
        typename Expression21Part5<Y...>::type,
        typename MulSeven<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression22Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulSix<V>::type,
        typename Expression22Part5<Y...>::type,
        typename MulZero<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression23Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulThree<V>::type,
        typename Expression23Part5<Y...>::type,
        typename MulFive<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression24Part4 {
    typedef typename Add<typename MulFive<U>::type, typename MulZero<V>::type,
        typename Expression24Part5<Y...>::type,
        typename MulOne<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression25Part4 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulZero<V>::type,
        typename Expression25Part5<Y...>::type,
        typename MulSeven<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression26Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulTwo<V>::type,
        typename Expression26Part5<Y...>::type,
        typename MulThree<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression27Part4 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulOne<V>::type,
        typename Expression27Part5<Y...>::type,
        typename MulTwo<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression28Part4 {
    typedef typename Add<typename MulTwo<U>::type, typename MulSix<V>::type,
        typename Expression28Part5<Y...>::type,
        typename MulOne<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression29Part4 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulTwo<V>::type,
        typename Expression29Part5<Y...>::type,
        typename MulFive<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression30Part4 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulFour<V>::type,
        typename Expression30Part5<Y...>::type,
        typename MulFive<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression31Part4 {
    typedef typename Add<typename MulSix<U>::type, typename MulZero<V>::type,
        typename Expression31Part5<Y...>::type,
        typename MulSix<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression0Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFive<V>::type,
        typename Expression0Part4<Y...>::type,
        typename MulSeven<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression1Part3 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulFive<V>::type,
        typename Expression1Part4<Y...>::type,
        typename MulTwo<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression2Part3 {
    typedef typename Add<typename MulThree<U>::type, typename MulOne<V>::type,
        typename Expression2Part4<Y...>::type,
        typename MulOne<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression3Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulOne<V>::type,
        typename Expression3Part4<Y...>::type,
        typename MulSeven<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression4Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFive<V>::type,
        typename Expression4Part4<Y...>::type,
        typename MulZero<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression5Part3 {
    typedef typename Add<typename MulTwo<U>::type, typename MulThree<V>::type,
        typename Expression5Part4<Y...>::type,
        typename MulTwo<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression6Part3 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulOne<V>::type,
        typename Expression6Part4<Y...>::type,
        typename MulFive<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression7Part3 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulSeven<V>::type,
        typename Expression7Part4<Y...>::type,
        typename MulTwo<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression8Part3 {
    typedef typename Add<typename MulFive<U>::type, typename MulFour<V>::type,
        typename Expression8Part4<Y...>::type,
        typename MulSix<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression9Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulTwo<V>::type,
        typename Expression9Part4<Y...>::type,
        typename MulZero<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression10Part3 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulFour<V>::type,
        typename Expression10Part4<Y...>::type,
        typename MulOne<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression11Part3 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulZero<V>::type,
        typename Expression11Part4<Y...>::type,
        typename MulSeven<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression12Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulSix<V>::type,
        typename Expression12Part4<Y...>::type,
        typename MulFour<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression13Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFour<V>::type,
        typename Expression13Part4<Y...>::type,
        typename MulFive<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression14Part3 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulOne<V>::type,
        typename Expression14Part4<Y...>::type,
        typename MulOne<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression15Part3 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulZero<V>::type,
        typename Expression15Part4<Y...>::type,
        typename MulSeven<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression16Part3 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulFour<V>::type,
        typename Expression16Part4<Y...>::type,
        typename MulFour<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression17Part3 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulZero<V>::type,
        typename Expression17Part4<Y...>::type,
        typename MulSeven<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression18Part3 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSeven<V>::type,
        typename Expression18Part4<Y...>::type,
        typename MulFour<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression19Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulOne<V>::type,
        typename Expression19Part4<Y...>::type,
        typename MulSeven<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression20Part3 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulZero<V>::type,
        typename Expression20Part4<Y...>::type,
        typename MulSeven<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression21Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulSeven<V>::type,
        typename Expression21Part4<Y...>::type,
        typename MulSeven<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression22Part3 {
    typedef typename Add<typename MulFour<U>::type, typename MulZero<V>::type,
        typename Expression22Part4<Y...>::type,
        typename MulOne<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression23Part3 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulThree<V>::type,
        typename Expression23Part4<Y...>::type,
        typename MulFive<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression24Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulSix<V>::type,
        typename Expression24Part4<Y...>::type,
        typename MulFour<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression25Part3 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulFive<V>::type,
        typename Expression25Part4<Y...>::type,
        typename MulFive<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression26Part3 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFive<V>::type,
        typename Expression26Part4<Y...>::type,
        typename MulZero<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression27Part3 {
    typedef typename Add<typename MulSeven<U>::type, typename MulTwo<V>::type,
        typename Expression27Part4<Y...>::type,
        typename MulTwo<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression28Part3 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulFour<V>::type,
        typename Expression28Part4<Y...>::type,
        typename MulOne<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression29Part3 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulZero<V>::type,
        typename Expression29Part4<Y...>::type,
        typename MulZero<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression30Part3 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulTwo<V>::type,
        typename Expression30Part4<Y...>::type,
        typename MulOne<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression31Part3 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulSeven<V>::type,
        typename Expression31Part4<Y...>::type,
        typename MulZero<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression0Part2 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulZero<V>::type,
        typename Expression0Part3<Y...>::type,
        typename MulFour<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression1Part2 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulSix<V>::type,
        typename Expression1Part3<Y...>::type,
        typename MulFive<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression2Part2 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulSix<V>::type,
        typename Expression2Part3<Y...>::type,
        typename MulOne<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression3Part2 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulFive<V>::type,
        typename Expression3Part3<Y...>::type,
        typename MulFive<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression4Part2 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulTwo<V>::type,
        typename Expression4Part3<Y...>::type,
        typename MulSeven<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression5Part2 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulSeven<V>::type,
        typename Expression5Part3<Y...>::type,
        typename MulSeven<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression6Part2 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSix<V>::type,
        typename Expression6Part3<Y...>::type,
        typename MulOne<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression7Part2 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulFive<V>::type,
        typename Expression7Part3<Y...>::type,
        typename MulSeven<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression8Part2 {
    typedef typename Add<typename MulSeven<U>::type, typename MulFour<V>::type,
        typename Expression8Part3<Y...>::type,
        typename MulOne<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression9Part2 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulSix<V>::type,
        typename Expression9Part3<Y...>::type,
        typename MulThree<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression10Part2 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulThree<V>::type,
        typename Expression10Part3<Y...>::type,
        typename MulFive<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression11Part2 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulThree<V>::type,
        typename Expression11Part3<Y...>::type,
        typename MulSeven<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression12Part2 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulFive<V>::type,
        typename Expression12Part3<Y...>::type,
        typename MulSeven<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression13Part2 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulZero<V>::type,
        typename Expression13Part3<Y...>::type,
        typename MulZero<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression14Part2 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulThree<V>::type,
        typename Expression14Part3<Y...>::type,
        typename MulFour<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression15Part2 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulThree<V>::type,
        typename Expression15Part3<Y...>::type,
        typename MulTwo<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression16Part2 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        typename Expression16Part3<Y...>::type,
        typename MulThree<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression17Part2 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulTwo<V>::type,
        typename Expression17Part3<Y...>::type,
        typename MulOne<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression18Part2 {
    typedef typename Add<typename MulFour<U>::type, typename MulSix<V>::type,
        typename Expression18Part3<Y...>::type,
        typename MulOne<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression19Part2 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulSix<V>::type,
        typename Expression19Part3<Y...>::type,
        typename MulFour<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression20Part2 {
    typedef typename Add<typename MulThree<U>::type, typename MulFour<V>::type,
        typename Expression20Part3<Y...>::type,
        typename MulTwo<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression21Part2 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        typename Expression21Part3<Y...>::type,
        typename MulThree<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression22Part2 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulSix<V>::type,
        typename Expression22Part3<Y...>::type,
        typename MulOne<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression23Part2 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulSeven<V>::type,
        typename Expression23Part3<Y...>::type,
        typename MulThree<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression24Part2 {
    typedef typename Add<typename MulZero<U>::type, typename MulSeven<V>::type,
        typename Expression24Part3<Y...>::type,
        typename MulTwo<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression25Part2 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulTwo<V>::type,
        typename Expression25Part3<Y...>::type,
        typename MulOne<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression26Part2 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFour<V>::type,
        typename Expression26Part3<Y...>::type,
        typename MulThree<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression27Part2 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulSeven<V>::type,
        typename Expression27Part3<Y...>::type,
        typename MulSeven<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression28Part2 {
    typedef typename Add<typename MulTwo<U>::type, typename MulTwo<V>::type,
        typename Expression28Part3<Y...>::type,
        typename MulSix<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression29Part2 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSix<V>::type,
        typename Expression29Part3<Y...>::type,
        typename MulTwo<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression30Part2 {
    typedef typename Add<typename MulTwo<U>::type, typename MulSeven<V>::type,
        typename Expression30Part3<Y...>::type,
        typename MulTwo<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression31Part2 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulOne<V>::type,
        typename Expression31Part3<Y...>::type,
        typename MulSix<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression0Part1 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulOne<V>::type,
        typename Expression0Part2<Y...>::type,
        typename MulZero<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression1Part1 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFive<V>::type,
        typename Expression1Part2<Y...>::type,
        typename MulThree<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression2Part1 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulSeven<V>::type,
        typename Expression2Part2<Y...>::type,
        typename MulOne<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression3Part1 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSix<V>::type,
        typename Expression3Part2<Y...>::type,
        typename MulZero<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression4Part1 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulTwo<V>::type,
        typename Expression4Part2<Y...>::type,
        typename MulOne<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression5Part1 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulZero<V>::type,
        typename Expression5Part2<Y...>::type,
        typename MulFour<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression6Part1 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulTwo<V>::type,
        typename Expression6Part2<Y...>::type,
        typename MulOne<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression7Part1 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulFour<V>::type,
        typename Expression7Part2<Y...>::type,
        typename MulOne<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression8Part1 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulFive<V>::type,
        typename Expression8Part2<Y...>::type,
        typename MulZero<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression9Part1 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        typename Expression9Part2<Y...>::type,
        typename MulSix<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression10Part1 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulTwo<V>::type,
        typename Expression10Part2<Y...>::type,
        typename MulThree<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression11Part1 {
    typedef typename Add<typename MulOne<U>::type, typename MulThree<V>::type,
        typename Expression11Part2<Y...>::type,
        typename MulTwo<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression12Part1 {
    typedef typename Add<typename MulThree<U>::type, typename MulTwo<V>::type,
        typename Expression12Part2<Y...>::type,
        typename MulTwo<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression13Part1 {
    typedef typename Add<typename MulTwo<U>::type, typename MulOne<V>::type,
        typename Expression13Part2<Y...>::type,
        typename MulTwo<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression14Part1 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulSix<V>::type,
        typename Expression14Part2<Y...>::type,
        typename MulZero<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression15Part1 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSix<V>::type,
        typename Expression15Part2<Y...>::type,
        typename MulSeven<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression16Part1 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulSeven<V>::type,
        typename Expression16Part2<Y...>::type,
        typename MulFive<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression17Part1 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulSix<V>::type,
        typename Expression17Part2<Y...>::type,
        typename MulSeven<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression18Part1 {
    typedef typename Add<typename MulThree<U>::type, typename MulSeven<V>::type,
        typename Expression18Part2<Y...>::type,
        typename MulOne<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression19Part1 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulFive<V>::type,
        typename Expression19Part2<Y...>::type,
        typename MulSeven<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression20Part1 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulFive<V>::type,
        typename Expression20Part2<Y...>::type,
        typename MulFour<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression21Part1 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFour<V>::type,
        typename Expression21Part2<Y...>::type,
        typename MulFour<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression22Part1 {
    typedef typename Add<typename MulSix<U>::type, typename MulOne<V>::type,
        typename Expression22Part2<Y...>::type,
        typename MulTwo<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression23Part1 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulZero<V>::type,
        typename Expression23Part2<Y...>::type,
        typename MulOne<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression24Part1 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulTwo<V>::type,
        typename Expression24Part2<Y...>::type,
        typename MulSeven<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression25Part1 {
    typedef typename Add<typename MulFive<U>::type, typename MulOne<V>::type,
        typename Expression25Part2<Y...>::type,
        typename MulTwo<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression26Part1 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulOne<V>::type,
        typename Expression26Part2<Y...>::type,
        typename MulFour<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression27Part1 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulTwo<V>::type,
        typename Expression27Part2<Y...>::type,
        typename MulThree<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression28Part1 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulZero<V>::type,
        typename Expression28Part2<Y...>::type,
        typename MulTwo<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression29Part1 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulSeven<V>::type,
        typename Expression29Part2<Y...>::type,
        typename MulSeven<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression30Part1 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulSeven<V>::type,
        typename Expression30Part2<Y...>::type,
        typename MulSeven<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression31Part1 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulTwo<V>::type,
        typename Expression31Part2<Y...>::type,
        typename MulTwo<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression0Part0 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulSeven<V>::type,
        typename Expression0Part1<Y...>::type,
        typename MulSeven<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression1Part0 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulOne<V>::type,
        typename Expression1Part1<Y...>::type,
        typename MulThree<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression2Part0 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulTwo<V>::type,
        typename Expression2Part1<Y...>::type,
        typename MulFour<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression3Part0 {
    typedef typename Add<typename MulSeven<U>::type, typename MulTwo<V>::type,
        typename Expression3Part1<Y...>::type,
        typename MulTwo<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression4Part0 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulSeven<V>::type,
        typename Expression4Part1<Y...>::type,
        typename MulZero<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression5Part0 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulOne<V>::type,
        typename Expression5Part1<Y...>::type,
        typename MulTwo<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression6Part0 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulSeven<V>::type,
        typename Expression6Part1<Y...>::type,
        typename MulZero<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression7Part0 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulTwo<V>::type,
        typename Expression7Part1<Y...>::type,
        typename MulFour<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression8Part0 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulOne<V>::type,
        typename Expression8Part1<Y...>::type,
        typename MulOne<W>::type, typename MulSeven<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression9Part0 {
    typedef typename Add<typename MulSeven<U>::type, typename MulFour<V>::type,
        typename Expression9Part1<Y...>::type,
        typename MulSix<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression10Part0 {
    typedef
        typename Add<typename MulSeven<U>::type, typename MulFive<V>::type,
        typename Expression10Part1<Y...>::type,
        typename MulThree<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression11Part0 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulSix<V>::type,
        typename Expression11Part1<Y...>::type,
        typename MulSeven<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression12Part0 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulOne<V>::type,
        typename Expression12Part1<Y...>::type,
        typename MulThree<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression13Part0 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulOne<V>::type,
        typename Expression13Part1<Y...>::type,
        typename MulFour<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression14Part0 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulZero<V>::type,
        typename Expression14Part1<Y...>::type,
        typename MulOne<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression15Part0 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulFour<V>::type,
        typename Expression15Part1<Y...>::type,
        typename MulFive<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression16Part0 {
    typedef typename Add<typename MulThree<U>::type, typename MulFive<V>::type,
        typename Expression16Part1<Y...>::type,
        typename MulSix<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression17Part0 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulTwo<V>::type,
        typename Expression17Part1<Y...>::type,
        typename MulThree<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression18Part0 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulOne<V>::type,
        typename Expression18Part1<Y...>::type,
        typename MulOne<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression19Part0 {
    typedef
        typename Add<typename MulTwo<U>::type, typename MulFour<V>::type,
        typename Expression19Part1<Y...>::type,
        typename MulSix<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression20Part0 {
    typedef typename Add<typename MulZero<U>::type, typename MulTwo<V>::type,
        typename Expression20Part1<Y...>::type,
        typename MulOne<W>::type, typename MulOne<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression21Part0 {
    typedef typename Add<typename MulSix<U>::type, typename MulOne<V>::type,
        typename Expression21Part1<Y...>::type,
        typename MulTwo<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression22Part0 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulZero<V>::type,
        typename Expression22Part1<Y...>::type,
        typename MulSeven<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression23Part0 {
    typedef
        typename Add<typename MulThree<U>::type, typename MulFive<V>::type,
        typename Expression23Part1<Y...>::type,
        typename MulThree<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression24Part0 {
    typedef typename Add<typename MulFour<U>::type, typename MulThree<V>::type,
        typename Expression24Part1<Y...>::type,
        typename MulOne<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression25Part0 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulSix<V>::type,
        typename Expression25Part1<Y...>::type,
        typename MulZero<W>::type, typename MulSix<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression26Part0 {
    typedef
        typename Add<typename MulFive<U>::type, typename MulFour<V>::type,
        typename Expression26Part1<Y...>::type,
        typename MulZero<W>::type, typename MulZero<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression27Part0 {
    typedef
        typename Add<typename MulOne<U>::type, typename MulFive<V>::type,
        typename Expression27Part1<Y...>::type,
        typename MulFive<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression28Part0 {
    typedef
        typename Add<typename MulFour<U>::type, typename MulFour<V>::type,
        typename Expression28Part1<Y...>::type,
        typename MulFive<W>::type, typename MulTwo<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression29Part0 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulSeven<V>::type,
        typename Expression29Part1<Y...>::type,
        typename MulZero<W>::type, typename MulThree<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression30Part0 {
    typedef
        typename Add<typename MulZero<U>::type, typename MulFive<V>::type,
        typename Expression30Part1<Y...>::type,
        typename MulSix<W>::type, typename MulFive<X>::type>::type
        type;
};

template <typename U, typename V, typename W, typename X, typename... Y>
struct Expression31Part0 {
    typedef
        typename Add<typename MulSix<U>::type, typename MulSix<V>::type,
        typename Expression31Part1<Y...>::type,
        typename MulTwo<W>::type, typename MulFour<X>::type>::type
        type;
};

template <typename... T> struct Expression0 {
    typedef typename Expression0Part0<T...>::type type;
};

template <typename... T> struct Expression1 {
    typedef typename Expression1Part0<T...>::type type;
};

template <typename... T> struct Expression2 {
    typedef typename Expression2Part0<T...>::type type;
};

template <typename... T> struct Expression3 {
    typedef typename Expression3Part0<T...>::type type;
};

template <typename... T> struct Expression4 {
    typedef typename Expression4Part0<T...>::type type;
};

template <typename... T> struct Expression5 {
    typedef typename Expression5Part0<T...>::type type;
};

template <typename... T> struct Expression6 {
    typedef typename Expression6Part0<T...>::type type;
};

template <typename... T> struct Expression7 {
    typedef typename Expression7Part0<T...>::type type;
};

template <typename... T> struct Expression8 {
    typedef typename Expression8Part0<T...>::type type;
};

template <typename... T> struct Expression9 {
    typedef typename Expression9Part0<T...>::type type;
};

template <typename... T> struct Expression10 {
    typedef typename Expression10Part0<T...>::type type;
};

template <typename... T> struct Expression11 {
    typedef typename Expression11Part0<T...>::type type;
};

template <typename... T> struct Expression12 {
    typedef typename Expression12Part0<T...>::type type;
};

template <typename... T> struct Expression13 {
    typedef typename Expression13Part0<T...>::type type;
};

template <typename... T> struct Expression14 {
    typedef typename Expression14Part0<T...>::type type;
};

template <typename... T> struct Expression15 {
    typedef typename Expression15Part0<T...>::type type;
};

template <typename... T> struct Expression16 {
    typedef typename Expression16Part0<T...>::type type;
};

template <typename... T> struct Expression17 {
    typedef typename Expression17Part0<T...>::type type;
};

template <typename... T> struct Expression18 {
    typedef typename Expression18Part0<T...>::type type;
};

template <typename... T> struct Expression19 {
    typedef typename Expression19Part0<T...>::type type;
};

template <typename... T> struct Expression20 {
    typedef typename Expression20Part0<T...>::type type;
};

template <typename... T> struct Expression21 {
    typedef typename Expression21Part0<T...>::type type;
};

template <typename... T> struct Expression22 {
    typedef typename Expression22Part0<T...>::type type;
};

template <typename... T> struct Expression23 {
    typedef typename Expression23Part0<T...>::type type;
};

template <typename... T> struct Expression24 {
    typedef typename Expression24Part0<T...>::type type;
};

template <typename... T> struct Expression25 {
    typedef typename Expression25Part0<T...>::type type;
};

template <typename... T> struct Expression26 {
    typedef typename Expression26Part0<T...>::type type;
};

template <typename... T> struct Expression27 {
    typedef typename Expression27Part0<T...>::type type;
};

template <typename... T> struct Expression28 {
    typedef typename Expression28Part0<T...>::type type;
};

template <typename... T> struct Expression29 {
    typedef typename Expression29Part0<T...>::type type;
};

template <typename... T> struct Expression30 {
    typedef typename Expression30Part0<T...>::type type;
};

template <typename... T> struct Expression31 {
    typedef typename Expression31Part0<T...>::type type;
};

template <typename... U>
struct Check0 : Conjunction<TrueType, IsSame<typename Expression0<U...>::type,
    MulSeven<GetOne::type>::type>> {};

template <typename... U>
struct Check1
    : Conjunction<Check0<U...>, IsSame<typename Expression1<U...>::type,
    MulTwo<GetOne::type>::type>> {};

template <typename... U>
struct Check2
    : Conjunction<Check1<U...>, IsSame<typename Expression2<U...>::type,
    MulFour<GetOne::type>::type>> {};

template <typename... U>
struct Check3
    : Conjunction<Check2<U...>, IsSame<typename Expression3<U...>::type,
    MulFive<GetOne::type>::type>> {};

template <typename... U>
struct Check4
    : Conjunction<Check3<U...>, IsSame<typename Expression4<U...>::type,
    MulFive<GetOne::type>::type>> {};

template <typename... U>
struct Check5
    : Conjunction<Check4<U...>, IsSame<typename Expression5<U...>::type,
    MulThree<GetOne::type>::type>> {};

template <typename... U>
struct Check6
    : Conjunction<Check5<U...>, IsSame<typename Expression6<U...>::type,
    MulZero<GetOne::type>::type>> {};

template <typename... U>
struct Check7
    : Conjunction<Check6<U...>, IsSame<typename Expression7<U...>::type,
    MulThree<GetOne::type>::type>> {};

template <typename... U>
struct Check8
    : Conjunction<Check7<U...>, IsSame<typename Expression8<U...>::type,
    MulTwo<GetOne::type>::type>> {};

template <typename... U>
struct Check9
    : Conjunction<Check8<U...>, IsSame<typename Expression9<U...>::type,
    MulOne<GetOne::type>::type>> {};

template <typename... U>
struct Check10
    : Conjunction<Check9<U...>, IsSame<typename Expression10<U...>::type,
    MulOne<GetOne::type>::type>> {};

template <typename... U>
struct Check11
    : Conjunction<Check10<U...>, IsSame<typename Expression11<U...>::type,
    MulSix<GetOne::type>::type>> {};

template <typename... U>
struct Check12
    : Conjunction<Check11<U...>, IsSame<typename Expression12<U...>::type,
    MulFive<GetOne::type>::type>> {};

template <typename... U>
struct Check13
    : Conjunction<Check12<U...>, IsSame<typename Expression13<U...>::type,
    MulSeven<GetOne::type>::type>> {};

template <typename... U>
struct Check14
    : Conjunction<Check13<U...>, IsSame<typename Expression14<U...>::type,
    MulZero<GetOne::type>::type>> {};

template <typename... U>
struct Check15
    : Conjunction<Check14<U...>, IsSame<typename Expression15<U...>::type,
    MulSeven<GetOne::type>::type>> {};

template <typename... U>
struct Check16
    : Conjunction<Check15<U...>, IsSame<typename Expression16<U...>::type,
    MulOne<GetOne::type>::type>> {};

template <typename... U>
struct Check17
    : Conjunction<Check16<U...>, IsSame<typename Expression17<U...>::type,
    MulSix<GetOne::type>::type>> {};

template <typename... U>
struct Check18
    : Conjunction<Check17<U...>, IsSame<typename Expression18<U...>::type,
    MulSix<GetOne::type>::type>> {};

template <typename... U>
struct Check19
    : Conjunction<Check18<U...>, IsSame<typename Expression19<U...>::type,
    MulSix<GetOne::type>::type>> {};

template <typename... U>
struct Check20
    : Conjunction<Check19<U...>, IsSame<typename Expression20<U...>::type,
    MulSix<GetOne::type>::type>> {};

template <typename... U>
struct Check21
    : Conjunction<Check20<U...>, IsSame<typename Expression21<U...>::type,
    MulZero<GetOne::type>::type>> {};

template <typename... U>
struct Check22
    : Conjunction<Check21<U...>, IsSame<typename Expression22<U...>::type,
    MulFour<GetOne::type>::type>> {};

template <typename... U>
struct Check23
    : Conjunction<Check22<U...>, IsSame<typename Expression23<U...>::type,
    MulSix<GetOne::type>::type>> {};

template <typename... U>
struct Check24
    : Conjunction<Check23<U...>, IsSame<typename Expression24<U...>::type,
    MulSeven<GetOne::type>::type>> {};

template <typename... U>
struct Check25
    : Conjunction<Check24<U...>, IsSame<typename Expression25<U...>::type,
    MulFour<GetOne::type>::type>> {};

template <typename... U>
struct Check26
    : Conjunction<Check25<U...>, IsSame<typename Expression26<U...>::type,
    MulOne<GetOne::type>::type>> {};

template <typename... U>
struct Check27
    : Conjunction<Check26<U...>, IsSame<typename Expression27<U...>::type,
    MulFour<GetOne::type>::type>> {};

template <typename... U>
struct Check28
    : Conjunction<Check27<U...>, IsSame<typename Expression28<U...>::type,
    MulThree<GetOne::type>::type>> {};

template <typename... U>
struct Check29
    : Conjunction<Check28<U...>, IsSame<typename Expression29<U...>::type,
    MulThree<GetOne::type>::type>> {};

template <typename... U>
struct Check30
    : Conjunction<Check29<U...>, IsSame<typename Expression30<U...>::type,
    MulThree<GetOne::type>::type>> {};

template <typename... U>
struct Check31
    : Conjunction<Check30<U...>, IsSame<typename Expression31<U...>::type,
    MulSeven<GetOne::type>::type>> {};

template <typename T, typename = void> struct ToInt : IntConstant<3> {};

template <typename T>
struct ToInt<T, typename EnableIf<
    Conjunction<Negation<TestBit2<T>>, Negation<TestBit1<T>>,
    TestBit0<T>>::value>::type> : IntConstant<5> {};

template <typename T>
struct ToInt<T,
    typename EnableIf<Conjunction<Negation<TestBit2<T>>, TestBit1<T>,
    Negation<TestBit0<T>>>::value>::type>
    : IntConstant<7> {};

template <typename T>
struct ToInt<T,
    typename EnableIf<Conjunction<Negation<TestBit2<T>>, TestBit1<T>,
    TestBit0<T>>::value>::type>
    : IntConstant<11> {};

template <typename T>
struct ToInt<T,
    typename EnableIf<Conjunction<TestBit2<T>, Negation<TestBit1<T>>,
    Negation<TestBit0<T>>>::value>::type>
    : IntConstant<13> {};

template <typename T>
struct ToInt<T,
    typename EnableIf<Conjunction<TestBit2<T>, Negation<TestBit1<T>>,
    TestBit0<T>>::value>::type>
    : IntConstant<17> {};

template <typename T>
struct ToInt<T,
    typename EnableIf<Conjunction<TestBit2<T>, TestBit1<T>,
    Negation<TestBit0<T>>>::value>::type>
    : IntConstant<19> {};

template <typename T>
struct ToInt<T, typename EnableIf<Conjunction<TestBit2<T>, TestBit1<T>,
    TestBit0<T>>::value>::type>
    : IntConstant<21> {};

template <typename... U> struct Nagi {
    static char buf[108];
    static const char* GetFlag() {
        if (Check31<U...>::value == false) {
            return "I don't know the flag, ask some else!";
        }
        int key[] = { ToInt<U>::value... };
        int S[256];
        int i, j = 0, t;
        for (i = 0; i < 256; i++) {
            S[i] = i;
        }
        for (i = 0; i < 256; i++) {
            j = (j + S[i] + key[i % sizeof...(U)]) & 0xff;
            t = S[i], S[i] = S[j], S[j] = t;
        }
        i = j = 0;
        for (int k = 0; k < 107; k++) {
            i = (i + 1) & 0xff;
            j = (j + S[i]) & 0xff;
            t = S[i], S[i] = S[j], S[j] = t;
            buf[k] ^= S[(S[i] + S[j]) & 0xff];
        }
        return buf;
    }
};

template <typename... U>
char Nagi<U...>::buf[108] =
"\xb0\x0d\x1f\x0e\x2a\x27\x08\xd4\x1b\x8a\xf9\xde\x67\x86\x95\x80\x4f\x92"
"\xca\xa1\x70\x2c\x53\xae\xd7\x4e\xf2\x86\x4f\x37\x03\xdc\xbe\xf2\xc4\x0e"
"\x7c\x8f\x8a\x00\x09\x93\xf0\xd0\xf3\x37\xd4\x7e\x6f\x83\x6d\x3e\x16\x99"
"\x63\x25\x7a\x3c\x30\x51\xaf\xf6\x3e\xc5\x0f\xc8\x93\xeb\x4f\x6b\xbd\xc2"
"\xa1\x96\x2b\x4e\xc4\xca\x91\xcd\x70\xc2\x24\xe8\xa2\x92\xbe\x1e\xea\x48"
"\xf9\x16\xb0\x78\x00\x6b\x7c\x95\xb1\xa0\xcb\xf7\x06\xaf\x4d\xe8\x96";

// x = n2*4+n1*2+n0
// n0:type bit(A=0 B=1)
// n1:const bit
// n2:reference bit

// A=int B=float
// 0=A 2=const A 4=A& 6=const A&
// 1=B 3=const B 5=B& 7=const B&

// field GF(2^3), p=0b1101
// add              mul              inv
// 0 1 2 3 4 5 6 7  0 0 0 0 0 0 0 0  0
// 1 0 3 2 5 4 7 6  0 1 2 3 4 5 6 7  1
// 2 3 0 1 6 7 4 5  0 2 4 6 5 7 1 3  6
// 3 2 1 0 7 6 5 4  0 3 6 5 1 2 7 4  4
// 4 5 6 7 0 1 2 3  0 4 5 1 7 3 2 6  3
// 5 4 7 6 1 0 3 2  0 5 7 2 3 6 4 1  7
// 6 7 4 5 2 3 0 1  0 6 1 7 2 4 3 5  2
// 7 6 5 4 3 2 1 0  0 7 3 4 6 1 5 2  5

// Ax=b
// A                                                               x b
// 5 7 7 4 1 1 0 6 3 0 4 7 5 5 7 1 4 2 0 7 1 1 4 3 0 1 3 4 4 6 7 2 1 7
// 4 1 3 1 5 5 3 3 3 6 5 1 2 5 2 5 1 3 5 7 5 2 7 6 6 4 6 5 0 0 2 6 7 2
// 7 2 4 7 4 7 1 5 3 6 1 4 3 1 1 2 5 4 7 1 2 3 6 1 0 0 1 5 6 2 5 4 7 4
// 7 2 2 6 1 6 0 0 1 5 5 6 5 1 7 6 7 7 3 4 4 6 6 3 3 1 6 3 3 7 7 5 1 5
// 3 7 0 2 1 2 1 4 2 2 7 7 5 5 0 6 5 0 3 2 2 1 7 5 2 1 2 1 3 1 6 5 5 5
// 1 1 2 7 5 0 4 6 2 7 7 3 2 3 2 6 7 1 6 5 4 6 1 7 6 0 7 4 0 0 4 6 2 3
// 5 7 0 0 7 2 1 4 1 6 1 7 1 1 5 0 6 0 0 6 7 2 0 4 4 5 2 1 6 4 1 5 3 0
// 3 2 4 1 0 4 1 7 4 5 7 1 0 7 2 5 7 7 7 6 5 3 2 5 0 6 0 7 6 4 3 2 3 3
// 1 1 1 7 2 5 0 0 7 4 1 1 5 4 6 2 3 1 4 3 7 1 5 1 1 4 7 6 2 1 1 1 7 2
// 7 4 6 1 6 4 6 7 5 6 3 7 5 2 0 1 1 7 2 5 2 5 7 5 3 0 3 4 1 2 4 4 5 1
// 7 5 3 4 1 2 3 7 6 3 5 0 3 4 1 0 0 5 4 3 6 3 1 1 0 2 5 3 3 2 4 7 2 1
// 0 6 7 0 1 3 2 6 3 3 7 3 7 0 7 4 6 4 3 3 5 7 5 1 5 5 2 5 4 1 2 6 7 6
// 6 1 3 1 3 2 2 6 0 5 7 1 5 6 4 5 3 5 4 2 0 7 5 2 7 0 7 7 6 4 0 3 6 5
// 4 1 4 1 2 1 2 6 4 0 0 7 5 4 5 3 2 2 4 5 6 1 3 6 5 2 6 7 1 6 5 6 0 7
// 2 0 1 5 7 6 0 4 6 3 4 5 7 1 1 7 7 4 6 5 2 7 6 2 1 2 6 3 3 4 7 6 1 0
// 6 4 5 6 1 6 7 2 2 3 2 3 2 0 7 1 7 7 2 4 3 0 7 7 7 3 7 3 5 7 1 1 5 7
// 3 5 6 1 4 7 5 1 6 4 3 4 3 4 4 6 6 6 6 2 1 1 6 4 3 2 6 2 1 4 0 3 5 1
// 3 2 3 2 3 6 7 4 6 2 1 4 7 0 7 0 6 6 6 1 7 7 6 6 2 3 0 7 1 3 4 7 4 6
// 3 1 1 3 3 7 1 2 4 6 1 6 1 7 4 0 7 4 4 3 3 2 3 3 5 5 4 2 2 4 3 5 1 6
// 2 4 6 0 4 5 7 6 0 6 4 6 5 1 7 1 1 6 6 4 6 4 3 4 7 7 1 5 6 0 4 7 3 6
// 0 2 1 1 2 5 4 5 3 4 2 1 2 0 7 7 2 7 7 4 0 7 6 6 6 6 6 1 4 2 0 5 7 6
// 6 1 2 2 5 4 4 3 6 4 3 3 5 7 7 3 0 5 7 1 1 2 2 1 7 7 5 0 1 4 0 6 6 0
// 6 0 7 2 6 1 2 6 4 6 1 3 4 0 1 6 7 6 0 4 1 4 0 6 4 5 5 4 7 7 6 5 1 4
// 3 5 3 3 1 0 1 3 3 7 3 1 4 3 5 2 7 3 5 4 7 3 1 7 4 5 1 1 1 1 7 1 3 6
// 4 3 1 6 0 2 7 1 0 7 2 2 5 6 4 2 5 0 1 6 0 1 5 5 0 5 3 5 5 2 1 1 6 7
// 1 6 0 6 5 1 2 2 6 2 1 5 4 5 5 7 4 0 7 0 0 1 0 1 7 3 6 4 6 1 5 3 0 4
// 5 4 0 0 5 1 4 1 5 4 3 1 5 5 0 7 7 2 3 4 1 6 3 3 0 7 5 2 0 4 5 4 1 1
// 1 5 5 3 3 2 3 2 2 7 7 0 7 2 2 6 1 1 2 5 7 4 4 7 2 5 4 7 0 1 1 6 2 4
// 4 4 5 2 7 0 2 4 2 2 6 2 7 4 1 5 2 6 1 1 6 0 5 4 1 7 1 6 4 0 7 3 3 3
// 6 7 0 3 4 7 7 0 1 6 2 5 2 0 0 2 7 2 5 0 7 1 3 6 2 1 4 2 7 4 1 1 3 3
// 0 5 6 5 4 7 7 5 2 7 2 2 2 2 1 4 1 4 5 6 4 3 5 7 6 2 6 1 0 6 3 3 5 3
// 6 6 2 4 2 2 2 7 6 1 6 3 3 7 0 6 6 0 6 6 4 3 2 7 6 3 7 4 2 7 1 1 0 7
