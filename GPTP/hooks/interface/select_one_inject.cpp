#include "select_one.h"
#include <hook_tools.h>

const u32 Func_SelectOne = 0x0047B770;

namespace {

//Inject with jmpPatch()
void __declspec(naked) setSelectOne() {
  static CUnit *unit;
  static bool selectGroup;

  __asm {
    PUSHAD
    MOV EBP, ESP
    MOV unit, ECX
  }

  selectGroup = hooks::setSelectOneHook(unit);

  __asm {
	POPAD
    MOVZX EAX, selectGroup
	RETN
  }
}

} //unnamed namespace

namespace hooks {

void injectSelectOneHooks() {
  jmpPatch(setSelectOne, Func_SelectOne);
}

} //hooks