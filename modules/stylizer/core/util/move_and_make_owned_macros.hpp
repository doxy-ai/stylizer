#pragma once

#define STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(type)\
    type&& move() { return std::move(*this); }\
    virtual stylizer::maybe_owned<type> move_to_owned() = 0;
#define STYLIZER_MOVE_AND_MAKE_OWNED_METHODS(type)\
    type&& move() { return std::move(*this); }\
    virtual stylizer::maybe_owned<type> move_to_owned() {\
        return stylizer::maybe_owned<type>::make_owned_and_move(*this);\
    }
#define STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(type, lowest_base)\
    type&& move() { return std::move(*this); }\
    stylizer::maybe_owned<lowest_base> move_to_owned() override {\
        return stylizer::maybe_owned<type>::make_owned_and_move(*this).template move_as<lowest_base>();\
    }
