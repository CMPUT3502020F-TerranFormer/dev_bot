#ifndef TF_UNIT_HPP
#define TF_UNIT_HPP

/**
 * Data class for a unit
 */
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"

using namespace sc2;

struct TF_unit {
    TF_unit() {}
    TF_unit(UNIT_TYPEID type, Tag tag)
        : type(type), tag(tag)
    {}
    UNIT_TYPEID type;
    Tag tag;
};

#endif