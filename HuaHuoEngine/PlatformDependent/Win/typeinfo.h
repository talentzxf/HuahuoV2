// VS2019 16.4 has removed <typeinfo.h> but some code (e.g. PhysX)
// tries to include it. The <typeinfo> does the same and has existed
// for a long time too, see
// https://github.com/NVIDIAGameWorks/PhysX/issues/164

#include <typeinfo>
