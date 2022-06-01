#pragma once

#include "Utilities/NonCopyable.h"
#include "BaseClasses/MessageIdentifier.h"
#include "BaseClasses/BaseTypes.h"
#include "Logging/LogAssert.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Utilities/EnumFlags.h"
#include <vector>

namespace HuaHuo { class Type;  }

#define DECLARE_MESSAGE_IDENTIFIER(NAME_) extern EXPORT_COREMODULE const MessageIdentifier NAME_
#define DEFINE_MESSAGE_IDENTIFIER(NAME_, ARGS_) EXPORT_COREMODULE const MessageIdentifier NAME_ ARGS_

// Usage if you want to expose a new message type in you header file e.g. kOnTransformChanged:
//
// DECLARE_MESSAGE_IDENTIFIER(kOnTransformChanged);
//
// Then make sure to define the message somewhere in a .cpp file as well
//
// DEFINE_MESSAGE_IDENTIFIER(kOnTransformChanged ("OnTransformChanged", MessageIdentifier::kDontSendToScripts));
// or if the message accept and int as argument:
// DEFINE_MESSAGE_IDENTIFIER(kOnTransformChanged ("OnTransformChanged", MessageIdentifier::kDontSendToScripts, TypeOf<int>()));
class MessageIdentifier : public NonCopyable
{
public:

    // The kEnableMessageOptimization options needs further explanation:
    // Some messages are are broadcast/frequently send but also rarely listened for
    // e.g. kTransformChanged. Specifying a message as optimized will reserve a bit in
    // a mask for the message. A GameObject contains such a mask and if the bit is
    // set it means that that something is listening for that message. In case it
    // is unset the component that would send the message can skip broadcasting
    // the message altogether ie. the message broadcast is optimized away.
    enum Options
    {
        kDontSendToScripts =         0,
        kSendToScripts =             1 << 0,
        // kUseNotificationManager = 1 << 1 This is legacy
        kDontSendToDisabled =        1 << 2,
        kEnableMessageOptimization = 1 << 3
    };

    // Number of optimized messages is constrained by number of bits in a UInt32
    typedef UInt32 OptimizedMessageMask;
    enum { kMaxOptimizedMessages = sizeof(OptimizedMessageMask) * 8 };

    const char*         messageName;
    const char*         scriptParameterName;
    const HuaHuo::Type*  parameterType;
    int                 options;

    int GetMessageId() const  { return messageID; }

    /// Place the MessageIdentifier as a global variable!
    /// The constructor must be called before InitializeEngine
    explicit MessageIdentifier(const char* name, Options opt, const HuaHuo::Type* type = NULL, const char* scriptParamName = NULL);

    // See Options enum above for explanation about optimized messages
    OptimizedMessageMask GetOptimizedMask() const
    {
        Assert(options & kEnableMessageOptimization);
        return (options & kEnableMessageOptimization) ? 1 << messageID : 0;
    }

    typedef std::vector<const MessageIdentifier*> RegisteredMessages;

    // All registered message identifiers
    static RegisteredMessages& GetRegisteredMessages();
    static size_t GetOptimizedMessageCount();

    // Check for duplicate message identifiers
    static void CheckIntegrity();

private:
    mutable int                 messageID;  // mutable because we write to it from inside a const static variable, required on some platforms to ensure it's not placed in a R/O section
    RegisterRuntimeInitializeAndCleanup registerCallbacks;
};

ENUM_FLAGS(MessageIdentifier::Options);
