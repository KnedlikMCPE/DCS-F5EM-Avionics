#include "DCS/DCS.h"

class avSimplestLuaDevice : public cockpit::avLuaDevice {

};

extern "C" __declspec(dllexport) void ed_module_initialize(void) {
    static bool first_time = true;
    if (first_time) {
        first_time = false;
        static WorldFactory<avSimplestLuaDevice> avSimplestLuaDevice_factory;
    }
}
