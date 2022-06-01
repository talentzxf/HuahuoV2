#pragma once
#include "BaseClasses/MessageHandler.h"

/// Creates a wrapper that calls a function with no parameter
#define REGISTER_MESSAGE_VOID(NOTIFICATION_NAME_, FUNCTION_) \
    struct PP_CONCAT(FunctorImpl_, __LINE__, NOTIFICATION_NAME_) {  \
        static void Call(void* object, int, MessageData&) {         \
            ThisType* castedObject = static_cast<ThisType*> (object);   \
            castedObject->FUNCTION_();                                  \
        }                                                               \
    };                                                                  \
    MessageHandler::Get().RegisterMessageCallback(TypeOf<ThisType>(), NOTIFICATION_NAME_, \
                                                  PP_CONCAT(FunctorImpl_, __LINE__, NOTIFICATION_NAME_)::Call, NULL)

/// Creates a wrapper thats sends the specified DataType to the member functions
#define REGISTER_MESSAGE(NOTIFICATION_NAME_, FUNCTION_, DATA_TYPE_) \
    DETAIL__REGISTER_MESSAGE_FUNCTOR(NOTIFICATION_NAME_, FUNCTION_, DATA_TYPE_) \
    DETAIL__REGISTER_MESSAGE_CALLBACK(NOTIFICATION_NAME_, DATA_TYPE_)

/// Creates a wrapper thats sends the specified DataType ptr to the member functions
#define REGISTER_MESSAGE_PTR(NOTIFICATION_NAME_, FUNCTION_, DATA_TYPE_) \
    DETAIL__REGISTER_MESSAGE_FUNCTOR(NOTIFICATION_NAME_, FUNCTION_, DATA_TYPE_*) \
    DETAIL__REGISTER_MESSAGE_CALLBACK(NOTIFICATION_NAME_, DATA_TYPE_)

/// ----------------------------------------------------------------------------
/// Implementation details for above macros follow.
/// We have this in a separate section to cut down on clutter above.

#if DEBUGMODE
#define CHECK_MSG_DATA_TYPE \
if (!CheckMessageDataType(messageID, messageData))                  \
    DebugStringToFile("Check message data", __FILE__, 0, 0, kAssert);
#else
#define CHECK_MSG_DATA_TYPE
#endif

/// Helper to creates a wrapper functor thats sends the specified DataType to the member functions
#define DETAIL__REGISTER_MESSAGE_FUNCTOR(NOTIFICATION_NAME_, FUNCTION_, DATA_TYPE_) \
    struct PP_CONCAT(FunctorImpl_, __LINE__, NOTIFICATION_NAME_) {      \
        static void Call(void* object, int messageID, MessageData& messageData) { \
            CHECK_MSG_DATA_TYPE                                         \
            DATA_TYPE_ data = messageData.GetData<DATA_TYPE_>();        \
            ThisType* castedObject = static_cast<ThisType*> (object);   \
            castedObject->FUNCTION_(data);                              \
        }                                                               \
    };

#define DETAIL__REGISTER_MESSAGE_CALLBACK(NOTIFICATION_NAME_, DATA_TYPE_) \
    MessageHandler::Get().RegisterMessageCallback(TypeOf<ThisType>(),   \
                                                  NOTIFICATION_NAME_,   \
                                                  PP_CONCAT(FunctorImpl_, __LINE__, NOTIFICATION_NAME_)::Call, \
                                                  TypeOf<DATA_TYPE_>())
