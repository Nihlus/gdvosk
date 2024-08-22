//
// Created by jarl on 22/08/24.
//

#ifndef GDVOSK_AUTO_PROPERTY_H
#define GDVOSK_AUTO_PROPERTY_H

/**
 * Defines backing members for a Godot property (a private field, a public getter, and a public setter). Accessibility
 * after using this macro will be private, as properties are expected to be declared in the private section of a class.
 */
#define GODOT_PROPERTY(TYPE, NAME, DEFAULT) \
private:                           \
    TYPE _##NAME = DEFAULT;                 \
public:                                        \
    [[nodiscard]] TYPE get_##NAME() const; \
    void set_##NAME(std::conditional_t<std::is_trivially_copyable_v<TYPE>, TYPE, const TYPE&> NAME); \
private:

#define REGISTER_GODOT_PROPERTY(TYPE, NAME)                                  \
ClassDB::bind_method(D_METHOD("set_" #NAME), &self_type::set_##NAME); \
ClassDB::bind_method(D_METHOD("get_" #NAME), &self_type::get_##NAME);        \
ADD_PROPERTY                                                                 \
(                                                                            \
    PropertyInfo(TYPE, #NAME),                                               \
    "set_" #NAME,                                                            \
    "get_" #NAME                                                             \
);

#define REGISTER_GODOT_PROPERTY_WITH_HINT(TYPE, NAME, ...)                   \
ClassDB::bind_method(D_METHOD("set_" #NAME, #NAME), &self_type::set_##NAME); \
ClassDB::bind_method(D_METHOD("get_" #NAME), &self_type::get_##NAME);        \
ADD_PROPERTY                                                                 \
(                                                                            \
    PropertyInfo(TYPE, #NAME, __VA_ARGS__),                                  \
    "set_" #NAME,                                                            \
    "get_" #NAME                                                             \
);

#endif //GDVOSK_AUTO_PROPERTY_H
