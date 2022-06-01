#include "MessageHandler.h"
#include "MessageData.h"

MessageHandler* MessageHandler::ms_Instance = HUAHUO_NEW(MessageHandler, kMemPermanent);

MessageForwarder::MessageForwarder()
//    : m_SupportedMessages(kMemPermanent)
//    , m_SupportedMessagesParameter(kMemPermanent)
{
    m_GeneralMessage = NULL;
    m_CanHandleGeneralMessage = NULL;
}

void MessageForwarder::HandleMessage(void* receiver, int messageID, MessageData& messageData)
{
    MessageCallback messagePtr = m_GeneralMessage;
    if (messageID < (int)m_SupportedMessages.size() && m_SupportedMessages[messageID] != NULL)
        messagePtr = m_SupportedMessages[messageID];

    Assert(messagePtr != NULL);
    messagePtr(receiver, messageID, messageData);
}

bool MessageForwarder::HasMessageCallback(const MessageIdentifier& identifier)
{
    if (identifier.GetMessageId() < (int)m_SupportedMessages.size() && m_SupportedMessages[identifier.GetMessageId()])
        return true;
    else
        return m_GeneralMessage != NULL && (identifier.options & MessageIdentifier::kSendToScripts);
}

bool MessageForwarder::WillHandleMessage(void* receiver, const MessageIdentifier& identifier)
{
    if (identifier.GetMessageId() < (int)m_SupportedMessages.size() && m_SupportedMessages[identifier.GetMessageId()])
        return true;

    if (m_GeneralMessage && (identifier.options & MessageIdentifier::kSendToScripts))
    {
        Assert(m_CanHandleGeneralMessage != NULL);
        MessageData data;
        return m_CanHandleGeneralMessage(receiver, identifier.GetMessageId(), data);
    }
    return false;
}

const HuaHuo::Type* MessageForwarder::GetExpectedParameterType(int messageID)
{
    if (messageID < (int)m_SupportedMessages.size())
        return m_SupportedMessagesParameter[messageID];
    else
        return 0;
}

void MessageForwarder::RegisterMessageCallback(int messageID, MessageCallback message, const HuaHuo::Type* type)
{
    Assert(messageID != -1);
    if (messageID >= (int)m_SupportedMessages.size())
    {
        m_SupportedMessages.resize(messageID + 1, NULL);
        m_SupportedMessagesParameter.resize(messageID + 1, 0);
    }
    m_SupportedMessages[messageID] = message;
    m_SupportedMessagesParameter[messageID] = type;
}

void MessageForwarder::RegisterAllMessagesCallback(MessageCallback message, CanHandleMessageCallback canHandle)
{
    m_GeneralMessage = message;
    m_CanHandleGeneralMessage = canHandle;
}

void MessageForwarder::AddBaseMessages(const MessageForwarder& baseClass)
{
    int maxsize = std::max(m_SupportedMessages.size(), baseClass.m_SupportedMessages.size());
    m_SupportedMessages.resize(maxsize, NULL);
    m_SupportedMessagesParameter.resize(maxsize, 0);
    for (size_t i = 0; i < m_SupportedMessages.size(); i++)
    {
        if (m_SupportedMessages[i] == NULL && i < baseClass.m_SupportedMessages.size())
        {
            m_SupportedMessages[i] = baseClass.m_SupportedMessages[i];
            m_SupportedMessagesParameter[i] = baseClass.m_SupportedMessagesParameter[i];
        }
    }

    if (m_GeneralMessage == NULL)
        m_GeneralMessage = baseClass.m_GeneralMessage;
}

MessageHandler::MessageHandler()
    : m_TargetType(NULL)
//    , m_SupportedMessages(kMemPermanent)
//    , m_Forwarder(kMemPermanent)
    , m_ClassCount(0)
    , m_MessageCount(0)
{
}

MessageHandler& MessageHandler::Get()
{
    return *ms_Instance;
}

void MessageHandler::Initialize(const HuaHuo::Type* targetType)
{
    Assert(m_TargetType == NULL);

    m_TargetType = targetType;

    MessageIdentifier::CheckIntegrity();
}

MessageHandler::~MessageHandler()
{
    // NOTE (ulfj) : Cannot assert here as native test framework doesn't shut down properly
    //AssertMsg(
    //  m_SupportedMessages.size() == 0 &&
    //  m_Forwarder.size() == 0 &&
}

void MessageHandler::Cleanup()
{
    m_TargetType = NULL;
    m_SupportedMessages.clear();
    m_Forwarder.clear();
    m_ClassCount = 0;
    m_MessageCount = 0;
}

static RuntimeTypeIndex GetHighestObjectTypeIndex(const HuaHuo::Type* targetType)
{
    RuntimeTypeIndex highestClassTypeIndex = targetType->GetRuntimeTypeIndex() + targetType->GetDescendantCount() - 1;
    DebugAssert(highestClassTypeIndex != 0);
    return highestClassTypeIndex;
}

void MessageHandler::RegisterMessageCallback(const HuaHuo::Type* type, const MessageIdentifier& messageIdentifier, MessageCallback message, const HuaHuo::Type* messageParameterType)
{
    RuntimeTypeIndex typeIndex = type->GetRuntimeTypeIndex();
    m_Forwarder.resize(std::max(typeIndex, GetHighestObjectTypeIndex(m_TargetType)) + 1);
    m_Forwarder[typeIndex].RegisterMessageCallback(messageIdentifier.GetMessageId(), message, messageParameterType);
}

void MessageHandler::RegisterAllMessagesCallback(const HuaHuo::Type* type, MessageCallback message, CanHandleMessageCallback canHandleNotification)
{
    RuntimeTypeIndex typeIndex = type->GetRuntimeTypeIndex();
    m_Forwarder.resize(std::max(typeIndex, GetHighestObjectTypeIndex(m_TargetType)) + 1);
    m_Forwarder[typeIndex].RegisterAllMessagesCallback(message, canHandleNotification);
}

static void PropagateNotificationsToDerivedClasses(MessageForwarders& notifications, const HuaHuo::Type* targetType)
{
    RuntimeTypeIndex highestClassTypeIndex = GetHighestObjectTypeIndex(targetType);

    DebugAssert(highestClassTypeIndex + 1 == notifications.size());

    for (size_t typeIndex = 0; typeIndex < notifications.size(); typeIndex++)
    {
        const HuaHuo::Type* type = HuaHuo::Type::GetTypeByRuntimeTypeIndex(typeIndex);
        if (type == NULL)
            continue;

        const HuaHuo::Type* baseType = type->GetBaseClass();
        while (baseType != NULL)
        {
            notifications[typeIndex].AddBaseMessages(notifications[baseType->GetRuntimeTypeIndex()]);
            baseType = baseType->GetBaseClass();
        }
    }
}

// PROFILER_INFORMATION(kProfileResolveCallbacks, "MessageHandler ResolveCallbacks", kProfilerScripts);
void MessageHandler::ResolveCallbacks()
{
    // PROFILER_AUTO(kProfileResolveCallbacks);
    PropagateNotificationsToDerivedClasses(m_Forwarder, m_TargetType);

    MessageIdentifier::RegisteredMessages& messages = MessageIdentifier::GetRegisteredMessages();

    m_MessageCount = messages.size();
    m_ClassCount = m_Forwarder.size();

    // Precalculate supported messages
    m_SupportedMessages.resize(m_ClassCount * m_MessageCount);
    for (int c = 0; c < m_ClassCount; c++)
    {
        for (int m = 0; m < m_MessageCount; m++)
        {
            const MessageIdentifier& message = *(messages[m]);
            bool hasCallback = m_Forwarder[c].HasMessageCallback(message);
            if (hasCallback)
            {
                // Check if the parameter is correct and print an error if they dont match
                const HuaHuo::Type* wantedParameter = m_Forwarder[c].GetExpectedParameterType(m);
                const HuaHuo::Type* messageParameter = message.parameterType;
                if (wantedParameter != NULL && messageParameter != wantedParameter)
                {
                    char buffy[4096];
                    char const format[] = "The message: %s in the class with "
                        "type index: %d uses a parameter type "
                        "that is different from the "
                        "message's parameter type: %s != %s.";
                    snprintf(buffy, sizeof(buffy), format, message.messageName,
                        c, wantedParameter->GetName(), messageParameter == NULL ? "null" : messageParameter->GetName());
                    ErrorString(buffy);
                    hasCallback = false;
                }
            }
            m_SupportedMessages[m * m_ClassCount + c] = hasCallback;
        }
    }
}

bool MessageHandler::WillHandleMessage(void* receiver, RuntimeTypeIndex typeIndex, const MessageIdentifier& messageIdentifier)
{
    Assert(HasMessageCallback(typeIndex, messageIdentifier));
    return m_Forwarder[typeIndex].WillHandleMessage(receiver, messageIdentifier);
}

const HuaHuo::Type* MessageHandler::MessageIDToParameterType(int messageID)
{
    Assert(messageID >= 0);
    MessageIdentifier::RegisteredMessages& messages = MessageIdentifier::GetRegisteredMessages();
    Assert(messageID < (int)messages.size());
    return messages[messageID]->parameterType;
}

const MessageIdentifier& MessageHandler::MessageIDToMessageIdentifier(int messageID)
{
    MessageIdentifier::RegisteredMessages& messages = MessageIdentifier::GetRegisteredMessages();
    Assert(messageID >= 0);
    Assert(messageID < (int)messages.size());
    return *(messages[messageID]);
}

//bool CheckMessageDataType(int messageIdentifier, MessageData& data)
//{
//    return MessageHandler::Get().MessageIDToParameterType(messageIdentifier) == data.type;
//}
