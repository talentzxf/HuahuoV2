// The following sequence of replacements can generate the macros from the respective lines in the FMOD.hpp header:
//, ->),(
//FMOD_RESULT F_API -> FMOD_IMPLEMENT_METHOD(CLASSNAME, 
//;->)
//\(([^\)\(]*?)(\w*)\)->(\1,\2)
//\s+\(->,(

#include "fakemodChannel.inc"
#include "fakemodChannelGroup.inc"
#include "fakemodDSP.inc"
#include "fakemodReverb.inc"
#include "fakemodSound.inc"
#include "fakemodSystem.inc"