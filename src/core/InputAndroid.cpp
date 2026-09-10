#include "InputAndroid.h"

namespace core {

void InputAndroid::Reset() { inputState_.Reset(); }

void InputAndroid::Update(const AInputEvent* event) {
  if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION) {
    return;
  }

  int32_t action = AMotionEvent_getAction(event);
  int32_t actionType = action & AMOTION_EVENT_ACTION_MASK;
  int32_t index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                  AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

  switch (actionType) {
    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
      ProcessDown(event, index);
      break;
    case AMOTION_EVENT_ACTION_MOVE:
      ProcessMove(event);
      break;
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
      ProcessUp(event, index);
      break;
    case AMOTION_EVENT_ACTION_CANCEL:
      ProcessCancel();
      break;
    default:
      break;
  }
}

void InputAndroid::ProcessDown(const AInputEvent* event, int32_t index) {
  auto& pointers = inputState_.pointers();
  int32_t id = AMotionEvent_getPointerId(event, index);
  float x = AMotionEvent_getX(event, index);
  float y = AMotionEvent_getY(event, index);

  Pointer t;
  t.startX = t.currentX = x;
  t.startY = t.currentY = y;
  t.pressure = AMotionEvent_getPressure(event, index);
  t.phase = Pointer::Phase::Began;

  pointers[id] = t;
}

void InputAndroid::ProcessMove(const AInputEvent* event) {
  size_t count = AMotionEvent_getPointerCount(event);

  auto& pointers = inputState_.pointers();

  for (size_t i = 0; i < count; ++i) {
    int32_t id = AMotionEvent_getPointerId(event, i);
    auto it = pointers.find(id);
    if (it == pointers.end()) {
      continue;
    }

    float x = AMotionEvent_getX(event, i);
    float y = AMotionEvent_getY(event, i);

    Pointer& t = it->second;
    t.deltaX = x - t.prevX;
    t.deltaY = y - t.prevY;
    t.currentX = x;
    t.currentY = y;
    t.pressure = AMotionEvent_getPressure(event, i);
    t.phase = Pointer::Phase::Moved;
  }
}

void InputAndroid::ProcessUp(const AInputEvent* event, int32_t index) {
  int32_t id = AMotionEvent_getPointerId(event, index);
  auto& pointers = inputState_.pointers();
  auto it = pointers.find(id);
  if (it == pointers.end()) {
    return;
  }

  float x = AMotionEvent_getX(event, index);
  float y = AMotionEvent_getY(event, index);

  Pointer& t = it->second;
  t.deltaX = x - t.prevX;
  t.deltaY = y - t.prevY;
  t.currentX = x;
  t.currentY = y;
  t.phase = Pointer::Phase::Ended;
}

void InputAndroid::ProcessCancel() {
  auto& pointers = inputState_.pointers();
  for (auto& [id, t] : pointers) {
    t.phase = Pointer::Phase::Cancelled;
  }
}

}  // namespace core