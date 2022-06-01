#pragma once

#include <vector>
//#include "External/boost/dynamic_bitset.h"
#include "TypeSystem/Type.h"
//#include "Runtime/Utilities/RuntimeStatic.h"
#include "MessageIdentifier.h"
#include "boost/dynamic_bitset.h"

struct MessageData;

// A MessageForwarder does callbacks specified by a given MessageIdentifier id (see MessageIdentifier.h)
//
// For each type added to the MessageHandler singleton (see definition below this class), an instance
// of a MessageForwarder is created being in charge of handling message to that specific type.
//
// As an extra support for scripting any MessageIdentifiers not having a callback explicitly registered in this
// class can be directed to a common callback using RegisterAllMessagesCallback(...). This is currently
// only used by the MessageForwarder associated with the MonoBehaviour type since that is the class all
// scripting classes are derived from.
//
class MessageForwarder
{
public:
    typedef void(*MessageCallback)(void* Receiver, int messageID, MessageData& data);
    typedef bool(*CanHandleMessageCallback)(void* Receiver, int messageID, MessageData& data);

    MessageForwarder();

    // Returns true if a message callback exists for the class and the messageID
    bool HasMessageCallback(const MessageIdentifier& messageID);

    // Returns true a message callback exists and the message will actually be handled.
    // This is used to find out if a message will *actually* be forwared, eg. HasMessageCallback will always return true
    // for e.g. ScriptBehaviours which checks at runtime if the message is supported by the script.
    bool WillHandleMessage(void* receiver, const MessageIdentifier& messageID);

    /// Calls the message
    /// the notification can be handled using CanHandleNotification
    void HandleMessage(void* receiver, int messageID, MessageData& notificationData);

    void RegisterMessageCallback(int messageID, MessageCallback message, const HuaHuo::Type* type);
    void RegisterAllMessagesCallback(MessageCallback message, CanHandleMessageCallback canHandleMessage);

    /// Returns the parameter RTTI that the receiver expects from a message. If
    /// the method doesn't expect a parameter, is not supported, or uses a
    /// general message handler, NULL is returned.
    const HuaHuo::Type* GetExpectedParameterType(int messageID);

    /// Adds all messages that baseMessages contains but this MessageReceiver does not handle yet.
    /// AddBaseNotifications is used to implement derivation by calling AddBaseNotifications for all base classes.
    void AddBaseMessages(const MessageForwarder& baseMessages);
private:

    // Indexed array on MessageIdentifier.id to callback for that message type.
    // Entry is NULL if no callback is registered for a given id.
    std::vector<MessageCallback> m_SupportedMessages;

    // Index as m_SupportedMessages and specifies the expected type of the message parameter. It
    // must be equal to the parameter type defined in the associated MessageIdentifier or a runtime
    // error will occur on initialization of the MessageHandler singleton this MessageForwarder is part of.
    std::vector<const HuaHuo::Type*>  m_SupportedMessagesParameter;
    MessageCallback                  m_GeneralMessage;
    CanHandleMessageCallback         m_CanHandleGeneralMessage;
};

typedef std::vector<MessageForwarder> MessageForwarders;

class TypeManager;

// A MessageHandler is a singleton class
//
// This class is in charge of registering message handlers for types that have
// RTTI info (currently only for Object derived classes).
// Supported message types are specified in MessageIdentifiers.h as instances of MessageIdentifier.
//
// Basically this class keeps a map of types=>MessageForwarders and a MessageForwarder will take
// care of the actual calling back for each registered type (i.e. Object derived class) and message.
// Also see class MessageForwarder above.
//
class MessageHandler
{
public:
    typedef MessageForwarder::MessageCallback MessageCallback;
    typedef MessageForwarder::CanHandleMessageCallback CanHandleMessageCallback;

    MessageHandler(); // public for unit testing purposes
    ~MessageHandler();

    // Make sure that Initialize(...) below has been called before using the
    // singleton returned.
    static MessageHandler& Get();

    void Initialize(const HuaHuo::Type* targetType);
    void Cleanup();

    // TODO: Modularization - when module registration is in place the ResolveCallback should
    // simply accept an argument specifying enough info to setup forwarders. For now we
    // let the "modules" register forwarders directly.
    // So on startup all class initialization code should register messages. The ResolveCallbacks()
    // must be called and no more message registration should be done.
    void RegisterMessageCallback(const HuaHuo::Type* type, const MessageIdentifier& messageIdentifier,
        MessageCallback message, const HuaHuo::Type* messageParameterType);

    void RegisterAllMessagesCallback(const HuaHuo::Type* type, MessageCallback message, CanHandleMessageCallback canHandleNotification);

    // Initializes all message forwarders and pre-calculates the supported messages bit array
    void ResolveCallbacks();

    // Returns true if a message callback exists for the class and the messageID
    bool HasMessageCallback(RuntimeTypeIndex typeIndex, const MessageIdentifier& messageIdentifier) { return m_SupportedMessages.test(messageIdentifier.GetMessageId() * m_ClassCount + typeIndex); }

    // Returns true if a message callback exists and the message will actually be handled.
    // This is used to find out if a message will *actually* be forwared, eg. HasMessageCallback will always return true
    // for e.g. ScriptBehaviours which checks at runtime if the message is supported by the script.
    bool WillHandleMessage(void* receiver, RuntimeTypeIndex typeIndex, const MessageIdentifier& messageIdentifier);

    /// Forwards a message to the appropriate MessageForwarder
    void HandleMessage(void* receiver, RuntimeTypeIndex typeIndex, const MessageIdentifier& messageIdentifier, MessageData& messageData)
    {
        Assert(typeIndex < m_ClassCount);
        // CLEAR_ALLOC_OWNER;
        m_Forwarder[typeIndex].HandleMessage(receiver, messageIdentifier.GetMessageId(), messageData);
    }

    void SetMessageEnabled(const HuaHuo::Type* type, const MessageIdentifier& messageIdentifier, bool enabled)
    {
        if (type->HasValidRuntimeTypeIndex())
            SetMessageEnabled(type->GetRuntimeTypeIndex(), messageIdentifier, enabled);
    }

    void SetMessageEnabled(RuntimeTypeIndex typeIndex, const MessageIdentifier& messageIdentifier, bool enabled)
    {
        // You are probably doing something wrong if you enable/disable a message twice
        DebugAssert(m_SupportedMessages[messageIdentifier.GetMessageId() * m_ClassCount + typeIndex] != enabled);
        m_SupportedMessages[messageIdentifier.GetMessageId() * m_ClassCount + typeIndex] = enabled;
    }

    // Converts a messageID to its parameter RTTI e.g. TypeOf<float>(). The messageID has to exist.
    const HuaHuo::Type* MessageIDToParameterType(int messageID);
    const MessageIdentifier& MessageIDToMessageIdentifier(int messageID);

    // Returns the number of registered messages
    int GetMessageCount() { return m_MessageCount; }

private:

    // static RuntimeStatic<MessageHandler> ms_Instance;
    static MessageHandler* ms_Instance;

    // Targets of messages are of this type or derived from it
    const HuaHuo::Type* m_TargetType;

    // Used to quickly test if a specific type + message has a callback associated.
    // This is only to make things faster since the same info is available by inspecting
    // m_Forwarder array.
    dynamic_bitset m_SupportedMessages;

    // A indexed array to get a Forwarder for a specific RTTI type.
    // The index used it RTTI.GetTypeIndex() for a given type and there is an entry for
    // each Object derived type in this array.
    MessageForwarders m_Forwarder;

    int m_ClassCount;   // Cached value of m_Forwarder.size() that doesn't change after initialization.
    int m_MessageCount; // Cached value of m_MessageNameToIndex.size() that doesn't change after initialization
};

// Compares the MessageData's type with the method signatures expected parameter type
bool EXPORT_COREMODULE CheckMessageDataType(int messageIdentifier, MessageData& data);
