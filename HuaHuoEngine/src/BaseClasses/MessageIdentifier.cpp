#include "MessageIdentifier.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Utilities/sorting.h"
#include "Memory/MemoryMacros.h"

static MessageIdentifier::RegisteredMessages* gRegisteredMessageIdentifiers;
static size_t gOptimizedMessageCount = 0;

static void InitializeMessage(void* ptr)
{
    const MessageIdentifier* mi = static_cast<MessageIdentifier*>(ptr);

#if DEBUGMODE
    MessageIdentifier::RegisteredMessages::iterator findIter
        = std::find(gRegisteredMessageIdentifiers->begin(), gRegisteredMessageIdentifiers->end(), mi);
    AssertFormatMsg(findIter == gRegisteredMessageIdentifiers->end(), "Cannot register %s message more that once", mi->messageName);
    for (size_t i = 0; i < gRegisteredMessageIdentifiers->size(); ++i)
    {
        Assert((*gRegisteredMessageIdentifiers)[i]->messageName != mi->messageName);
    }
#endif
    gRegisteredMessageIdentifiers->push_back(mi);
}

MessageIdentifier::MessageIdentifier(const char* name, Options opts, const HuaHuo::Type* type, const char* scriptParamName)
    : registerCallbacks(InitializeMessage, NULL, 0, this)
{
    Assert(name != NULL);
    AssertMsg(gRegisteredMessageIdentifiers == NULL || (*gRegisteredMessageIdentifiers)[0]->messageID == -1,
        "Defining variables of type MessageIdenfier is not allowed after InitializeEngine() has been called");
    messageName = name;
    parameterType = type;
    scriptParameterName = scriptParamName;
    messageID = -1;
    options = (int)opts;
}

MessageIdentifier::RegisteredMessages& MessageIdentifier::GetRegisteredMessages()
{
    Assert(gRegisteredMessageIdentifiers != NULL);
    return *gRegisteredMessageIdentifiers;
}

size_t MessageIdentifier::GetOptimizedMessageCount()
{
    return gOptimizedMessageCount;
}
//
//void MessageIdentifier::CheckIntegrity()
//{
//    // Build sorted Messages map. Which contains all messages sorted by name.
//    // This is in turn used to check for duplication message registrations or
//    // duplicate message names.
//    typedef UNITY_VECTOR_MAP (kMemTempAlloc, TempString, const MessageIdentifier*) SortedMessages;
//    SortedMessages sortedMessages;
//
//    RegisteredMessages& messages = GetRegisteredMessages();
//    RegisteredMessages::iterator j;
//    for (j = messages.begin(); j != messages.end(); j++)
//    {
//        const MessageIdentifier& identifier = **j;
//        Assert(identifier.messageName != NULL);
//
//        SortedMessages::iterator found = sortedMessages.find(identifier.messageName);
//        if (found == sortedMessages.end())
//        {
//            sortedMessages.insert(std::make_pair(TempString(identifier.messageName), *j));
//        }
//        else
//        {
//            if (identifier.parameterType != found->second->parameterType)
//            {
//                TempString error = "There are conflicting definitions of the message: ";
//                error += identifier.messageName;
//                error += ". The parameter of one message has to be the same across all definitions of that message.";
//                ErrorString(error);
//            }
//
//            if (identifier.scriptParameterName != found->second->scriptParameterName)
//            {
//                TempString error = "There are conflicting definitions of the message: ";
//                error += identifier.messageName;
//                error += ". The parameter of one message has to be the same across all definitions of that message.";
//                ErrorString(error);
//            }
//
//            if (identifier.options != found->second->options)
//            {
//                TempString error = "There are conflicting options of the message: ";
//                error += identifier.messageName;
//                ErrorString(error);
//            }
//        }
//    }
//
//    // Now we will assign a unique message ID to the message identifiers
//
//    struct ByMessageOptimizationSorter
//    {
//        static bool Compare(const MessageIdentifier* i1, const MessageIdentifier* i2)
//        {
//            bool i1Opt = i1->options & kEnableMessageOptimization;
//            bool i2Opt = i2->options & kEnableMessageOptimization;
//            return
//                (i1Opt && !i2Opt) ||
//                (!i1Opt && !i2Opt && strcmp(i1->messageName, i2->messageName) <= 0) ||
//                (i1Opt && i2Opt && strcmp(i1->messageName, i2->messageName) <= 0);
//        }
//    };
//    QSort(messages.begin(), messages.end(), &ByMessageOptimizationSorter::Compare);
//
//    gOptimizedMessageCount = 0;
//
//    // Assign message ID now that we are sure the first entries are the optimized messages
//    int id = 0;
//    for (j = messages.begin(); j != messages.end(); j++)
//    {
//        const MessageIdentifier& identifier = **j;
//
//        identifier.messageID = id++;
//
//        bool isOptimized = identifier.options & kEnableMessageOptimization;
//        AssertFormatMsg(identifier.messageID < kMaxOptimizedMessages || isOptimized == 0,
//            "Exceeded limit of %i optimized message identifiers", kMaxOptimizedMessages);
//        if (isOptimized)
//            gOptimizedMessageCount++;
//    }
//}

static void CreateRegisteredMessageIdentifiersArray(void*)
{
    gRegisteredMessageIdentifiers = NEW(MessageIdentifier::RegisteredMessages);
}

static void DestroyRegisteredMessageIdentifiersArray(void*)
{
    DELETE(gRegisteredMessageIdentifiers);//, kMemPermanent);
    gOptimizedMessageCount = 0;
}

static RegisterRuntimeInitializeAndCleanup sInit_MessageIdentifier(CreateRegisteredMessageIdentifiersArray, DestroyRegisteredMessageIdentifiersArray, -1);
