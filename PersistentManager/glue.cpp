
#include <emscripten.h>

extern "C" {

// Not using size_t for array indices as the values used by the javascript code are signed.

EM_JS(void, array_bounds_check_error, (size_t idx, size_t size), {
  throw 'Array index ' + idx + ' out of bounds: [0,' + size + ')';
});

void array_bounds_check(const int array_size, const int array_idx) {
  if (array_idx < 0 || array_idx >= array_size) {
    array_bounds_check_error(array_idx, array_size);
  }
}

// VoidPtr

void EMSCRIPTEN_KEEPALIVE emscripten_bind_VoidPtr___destroy___0(void** self) {
  delete self;
}

// PersistentManager

PersistentManager* EMSCRIPTEN_KEEPALIVE emscripten_bind_PersistentManager_getInstance_0(PersistentManager* self) {
  return self->getInstance();
}

unsigned char* EMSCRIPTEN_KEEPALIVE emscripten_bind_PersistentManager_getBuffer_0(PersistentManager* self) {
  return self->getBuffer();
}

int EMSCRIPTEN_KEEPALIVE emscripten_bind_PersistentManager_getBufferSize_0(PersistentManager* self) {
  return self->getBufferSize();
}

void EMSCRIPTEN_KEEPALIVE emscripten_bind_PersistentManager___destroy___0(PersistentManager* self) {
  delete self;
}

}

