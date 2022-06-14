
#if WEB_ENV

#include <emscripten/bind.h>

bool initStatus = false;

int main() {
    initStatus = true;
    return 0;
}

bool IsWASMInited(){
    return initStatus;
}

EMSCRIPTEN_BINDINGS(HuaHuoEngineV2) {
    emscripten::function("IsWASMInited", &IsWASMInited);
};
#endif