/*==================================================================================================

InputToggleList.h

The list of toggle inputs

INPUT_TOGGLE(name, resetOnKeyUp)

name - an identifier of the input
resetOnKeyUp - true or false.  if true, when a key is released, it will reset the state.  if false,
  it won't reset the state, the game has to.

==================================================================================================*/

INPUT_TOGGLE(WalkForward,	true)
INPUT_TOGGLE(WalkLeft,		true)
INPUT_TOGGLE(WalkRight,		true)
INPUT_TOGGLE(WalkBack,		true)
INPUT_TOGGLE(Jump,			false)
INPUT_TOGGLE(Crouch,		true)

// clean it up here for convincience
#undef INPUT_TOGGLE