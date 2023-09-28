#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

bool setSelectOneHook(const CUnit *unit);

void injectSelectOneHooks();

} //hooks