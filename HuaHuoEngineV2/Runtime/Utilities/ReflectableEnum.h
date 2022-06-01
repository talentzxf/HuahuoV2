#pragma once

struct ReflectableEnum {};

// Use the REFLECTABLE_ENUM macro to define an enum which has properties that can be queried at runtime:
//
// REFLECTABLE_ENUM(MyEnumType,
//      Entry1,
//      (Entry2, 3),
//      (Entry3, Entry2 + 2),
//      (Entry4, (1 << 5), "Runtime-accessible annotation"),
//      (Entry5, "Annotations can be specified without values too")
//  );
//
// The macro takes a comma-separated list of enum entries.
// If an entry is just the identifier, then it can just be declared with no special concerns, like a regular enum member.
// If you want to specify the integer value for the entry, and/or an optional annotation string, then you must enclose the entry
// in parens. The value supports all the expressions you would normally use to initialize enum entries.
//
// You can then use the resulting MyEnumType like a regular enum, with some restrictions:
//
// * Enum entries must be refered to by type-qualified names (MyEnumType::Entry1) rather than just being in the
//   scope the enum was declared in. In general this is a good thing...
//
// * Enum entries cannot be implicitly converted to/from int. There's helpers to do this in the EnumTraits namespace
//   (see below) but overall type safety is more strict. In general this is also a good thing...
//
// The 'interesting' functionality for a REFLECTABLE_ENUM is all in the EnumTraits namespace, rather than being
// part of the enum type itself. See EnumTraits.h.
//
// If the reflection functionality on an enum is not used, the code *should* get stripped by the linker's dead code
// elimination process. It appears to work on Windows and Mac at least.

#define REFLECTABLE_ENUM(ENUM_, ...)         PP_EVAL(DETAIL__REFLECTABLE_ENUM(ENUM_, DETAIL__REFLECTABLE_ENUM_NOT_FLAGS, __VA_ARGS__))

#define REFLECTABLE_FLAGS_ENUM(ENUM_, ...)   PP_EVAL(DETAIL__REFLECTABLE_ENUM(ENUM_, DETAIL__REFLECTABLE_ENUM_WITH_FLAGS, __VA_ARGS__))

#define DETAIL__REFLECTABLE_ENUM(ENUM_, FLAGS_MACRO_, ...)                                                  \
    struct ENUM_ : ReflectableEnum                                                                          \
    {                                                                                                       \
    private:                                                                                                \
        struct detail                                                                                       \
        {                                                                                                   \
            enum { DETAIL__ENUM_DECL_SELECT_DECLARATORS(__VA_ARGS__) };                                     \
        };                                                                                                  \
                                                                                                            \
    public:                                                                                                 \
        enum ActualEnumType                                                                                 \
        {                                                                                                   \
            DETAIL__ENUM_DECL_SELECT_FINAL_DECLARATORS(__VA_ARGS__)                                         \
        };                                                                                                  \
                                                                                                            \
        ENUM_(ActualEnumType value) : m_Value(value) { }                                                    \
        inline operator ActualEnumType() const { return m_Value; }                                          \
                                                                                                            \
    private:                                                                                                \
        ActualEnumType m_Value;                                                                             \
                                                                                                            \
        enum { _StaticCount = DETAIL__ENUM_DECL_COUNT(__VA_ARGS__) };                                       \
        static inline size_t Count() { return _StaticCount; }                                               \
                                                                                                            \
        static const int* Values()                                                                          \
        {                                                                                                   \
            static const int values[] = { DETAIL__ENUM_DECL_SELECT_VALUES(__VA_ARGS__) };                   \
            return values;                                                                                  \
        }                                                                                                   \
                                                                                                            \
        static const char* const* Names()                                                                   \
        {                                                                                                   \
            static const char* names[] = { DETAIL__ENUM_DECL_SELECT_IDENTIFIERS(__VA_ARGS__) };             \
            return names;                                                                                   \
        }                                                                                                   \
                                                                                                            \
        static const char* const* Annotations()                                                             \
        {                                                                                                   \
            static const char* annotations[] = { DETAIL__ENUM_DECL_SELECT_ANNOTATIONS(__VA_ARGS__) };       \
            return annotations;                                                                             \
        }                                                                                                   \
                                                                                                            \
        friend size_t EnumTraits::Count<ENUM_>();                                                           \
        friend const char* const* EnumTraits::GetNames<ENUM_>();                                            \
        friend const int* EnumTraits::GetValues<ENUM_>();                                                   \
        friend const char* const* EnumTraits::GetAnnotations<ENUM_>();                                      \
        friend bool EnumTraits::IsFlags<ENUM_>();                                                           \
        friend struct EnumTraits::StaticTraits<ENUM_>;                                                      \
                                                                                                            \
        FLAGS_MACRO_(ENUM_);                                                                                \
                                                                                                            \
    };

// Some helper stuff to make REFLECTABLE_ENUM work:

#define DETAIL__ENUM_DECL_SELECT_DECLARATOR(ARG_)                               PP_VARG_SELECT_OVERLOAD(DETAIL__ENUM_DECL_SELECT_DECLARATOR_, (PP_UNPAREN(ARG_)))
#define DETAIL__ENUM_DECL_SELECT_DECLARATOR_1(NAME_)                            NAME_,
#define DETAIL__ENUM_DECL_SELECT_DECLARATOR_2(NAME_, PARAM_)                    NAME_##_DefaultValue, NAME_=(PP_IS_STRING(PARAM_) ? NAME_##_DefaultValue : PP_CONST_VALUE(PARAM_)),
#define DETAIL__ENUM_DECL_SELECT_DECLARATOR_3(NAME_, VALUE_, ANNOTATION_)       NAME_=VALUE_,
#define DETAIL__ENUM_DECL_SELECT_DECLARATORS(...)                               PP_MAP(DETAIL__ENUM_DECL_SELECT_DECLARATOR, __VA_ARGS__)

#define DETAIL__ENUM_DECL_SELECT_FINAL_DECLARATOR_OUTER(ARG_)                   PP_DEFER(DETAIL__ENUM_DECL_SELECT_FINAL_DECLARATOR_INNER)(PP_UNPAREN(ARG_))
#define DETAIL__ENUM_DECL_SELECT_FINAL_DECLARATOR_INNER(NAME_, ...)             NAME_ = detail::NAME_,
#define DETAIL__ENUM_DECL_SELECT_FINAL_DECLARATORS(...)                         PP_MAP(DETAIL__ENUM_DECL_SELECT_FINAL_DECLARATOR_OUTER, __VA_ARGS__)

#define DETAIL__ENUM_DECL_SELECT_IDENTIFIER_OUTER(ARG_)                         PP_DEFER2(DETAIL__ENUM_DECL_SELECT_IDENTIFIER_INNER)(PP_UNPAREN(ARG_))
#define DETAIL__ENUM_DECL_SELECT_IDENTIFIER_INNER(NAME_, ...)                   #NAME_,
#define DETAIL__ENUM_DECL_SELECT_IDENTIFIERS(...)                               PP_MAP(DETAIL__ENUM_DECL_SELECT_IDENTIFIER_OUTER, __VA_ARGS__)

#define DETAIL__ENUM_DECL_SELECT_VALUE_OUTER(ARG_)                              PP_DEFER(DETAIL__ENUM_DECL_SELECT_VALUE_INNER)(PP_UNPAREN(ARG_))
#define DETAIL__ENUM_DECL_SELECT_VALUE_INNER(NAME_, ...)                        NAME_,
#define DETAIL__ENUM_DECL_SELECT_VALUES(...)                                    PP_MAP(DETAIL__ENUM_DECL_SELECT_VALUE_OUTER, __VA_ARGS__)

#define DETAIL__ENUM_DECL_SELECT_ANNOTATION(ARG_)                               PP_VARG_SELECT_OVERLOAD(DETAIL__ENUM_DECL_SELECT_ANNOTATION_, (PP_UNPAREN(ARG_)))
#define DETAIL__ENUM_DECL_SELECT_ANNOTATION_1(NAME_)                            NULL,
#define DETAIL__ENUM_DECL_SELECT_ANNOTATION_2(NAME_, PARAM_)                    (PP_IS_STRING(PARAM_) ? (const char*)(PARAM_) : (const char*)NULL),
#define DETAIL__ENUM_DECL_SELECT_ANNOTATION_3(NAME_, VALUE_, ANNOTATION_)       ANNOTATION_,
#define DETAIL__ENUM_DECL_SELECT_ANNOTATIONS(...)                               PP_MAP(DETAIL__ENUM_DECL_SELECT_ANNOTATION, __VA_ARGS__)

#define DETAIL__ENUM_DECL_COUNT_ELEM(ARG_)                                      +1
#define DETAIL__ENUM_DECL_COUNT(...)                                            PP_MAP(DETAIL__ENUM_DECL_COUNT_ELEM, __VA_ARGS__)

#define DETAIL__REFLECTABLE_ENUM_NOT_FLAGS(ENUM_)                               static bool IsFlags() { return false; }

#define DETAIL__REFLECTABLE_ENUM_WITH_FLAGS(ENUM_)                              static bool IsFlags() { return true; }  \
                                                                                friend ENUM_& operator |=(ENUM_& left, const ENUM_::ActualEnumType right) { return left = static_cast<ENUM_::ActualEnumType>(left | right); } \
                                                                                friend ENUM_& operator &=(ENUM_& left, const ENUM_::ActualEnumType right) { return left = static_cast<ENUM_::ActualEnumType>(left & right); } \
                                                                                friend ENUM_& operator ^=(ENUM_& left, const ENUM_::ActualEnumType right) { return left = static_cast<ENUM_::ActualEnumType>(left ^ right); } \
                                                                                ENUM_FLAGS_AS_MEMBER(ENUM_::ActualEnumType);

// Forward-declarations of things in the EnumTraits namespace, for the friend definitions
namespace EnumTraits
{
    template<typename TEnumImpl> size_t Count();
    template<typename TEnumImpl> const char* const* GetNames();
    template<typename TEnumImpl> const int* GetValues();
    template<typename TEnumImpl> const char* const* GetAnnotations();
    template<typename TEnumImpl> bool IsFlags();

    struct ReflectionInfo;
    template<typename TEnumImpl> const ReflectionInfo& GetReflectionInfo();

    template<typename TEnumImpl> struct StaticTraits;
}
