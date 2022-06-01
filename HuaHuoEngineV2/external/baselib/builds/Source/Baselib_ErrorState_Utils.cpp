#include "Include/Baselib.h"
#include "Include/C/Baselib_ErrorState.h"
#include "Include/Cpp/Lock.h"
#include "Baselib_ErrorState_Utils.h"

#include <stdarg.h>
#include <iomanip>

namespace detail
{
    class ExtraInformation
    {
    public:
        uint64_t Write(const char* format, va_list args)
        {
            lock.Acquire();
            auto generationCounter = ++currentGenerationCounter;
            vsnprintf(buffer, sizeof(buffer), format, args);
            lock.Release();
            return generationCounter;
        }

        std::ostream& TryReadInto(std::ostream& out, uint64_t generationCounter)
        {
            lock.Acquire();
            if (generationCounter == currentGenerationCounter)
                out << buffer;
            else
                out << "<lost extra info>";
            lock.Release();
            return out;
        }

        // doing a singleton here to avoid static constructing internals of this class
        static ExtraInformation& Get()
        {
            static ExtraInformation container;
            return container;
        }

    private:
        ExtraInformation() = default;
        ~ExtraInformation() = default;

        char buffer[2048];
        uint64_t currentGenerationCounter = 1;
        baselib::Lock lock;
    };
}

BASELIB_CPP_INTERFACE
{
    uint64_t Baselib_ErrorState_FormatAndStoreExtraInformation(const char* format, va_list args)
    {
        return detail::ExtraInformation::Get().Write(format, args);
    }

    std::ostream& operator<<(std::ostream& os, const Baselib_ErrorCode& errorCode)
    {
        switch (errorCode)
        {
            case Baselib_ErrorCode_Success: os << "Success"; break;
            case Baselib_ErrorCode_OutOfMemory: os << "Out of memory"; break;
            case Baselib_ErrorCode_OutOfSystemResources: os << "Out of system resources"; break;
            case Baselib_ErrorCode_InvalidAddressRange: os << "Invalid address range"; break;
            case Baselib_ErrorCode_InvalidArgument: os << "Invalid argument"; break;
            case Baselib_ErrorCode_InvalidBufferSize: os << "Invalid buffer size"; break;
            case Baselib_ErrorCode_InvalidState: os << "Invalid state"; break;
            case Baselib_ErrorCode_NotSupported: os << "Not supported"; break;
            case Baselib_ErrorCode_Timeout: os << "Time out"; break;
            case Baselib_ErrorCode_UnsupportedAlignment: os << "Unsupported alignment"; break;
            case Baselib_ErrorCode_InvalidPageSize: os << "Invalid page size"; break;
            case Baselib_ErrorCode_InvalidPageCount: os << "Invalid page count"; break;
            case Baselib_ErrorCode_UnsupportedPageState: os << "Unsupported page state"; break;
            case Baselib_ErrorCode_ThreadCannotJoinSelf: os << "Thread can not join itself"; break;
            case Baselib_ErrorCode_UnexpectedError: os << "Unexpected error"; break;
            case Baselib_ErrorCode_NetworkInitializationError: os << "Network initialization error"; break;
            case Baselib_ErrorCode_AddressInUse: os << "Address in use"; break;
            case Baselib_ErrorCode_AddressUnreachable: os << "Address unreachable"; break;
            case Baselib_ErrorCode_AddressFamilyNotSupported: os << "Address family not supported"; break;
            case Baselib_ErrorCode_Disconnected: os << "Disconnected"; break;
            case Baselib_ErrorCode_InvalidPathname: os << "Invalid pathname"; break;
            case Baselib_ErrorCode_RequestedAccessIsNotAllowed: os << "Requested access is not allowed"; break;
            case Baselib_ErrorCode_IOError: os << "General IO error"; break;
            case Baselib_ErrorCode_FailedToOpenDynamicLibrary: os << "Failed to open the requested dynamic library"; break;
            case Baselib_ErrorCode_FunctionNotFound: os << "The requested function was not found"; break;
        }
        os << " (0x" << std::setfill('0') << std::setw(8) << std::hex << (uint32_t)errorCode << ")";
        return os;
    }

    bool Baselib_ErrorState_Explain_BaselibErrorCode(const Baselib_ErrorState* errorState, std::ostream& out, Baselib_ErrorState_ExplainVerbosity verbosity)
    {
        if (!errorState)
            return false;

        switch (verbosity)
        {
            case Baselib_ErrorState_ExplainVerbosity_ErrorType:
                break;

            case Baselib_ErrorState_ExplainVerbosity_ErrorType_SourceLocation_Explanation:
                if (errorState->code != Baselib_ErrorCode_Success)
                {
                    auto location = errorState->sourceLocation;
                    if (location.file && location.function)
                        out << location.file << "(" << location.lineNumber << "):" << location.function << ": ";
                }
                break;
        }

        out << errorState->code;

        switch (errorState->extraInformationType)
        {
            case Baselib_ErrorState_ExtraInformationType_None:
                break;
            case Baselib_ErrorState_ExtraInformationType_StaticString:
                out << " " << (const char*)errorState->extraInformation;
                break;
            case Baselib_ErrorState_ExtraInformationType_GenerationCounter:
                out << " ";
                detail::ExtraInformation::Get().TryReadInto(out, errorState->extraInformation);
                break;
        }

        return errorState->code != Baselib_ErrorCode_Success
            && errorState->nativeErrorCodeType != Baselib_ErrorState_NativeErrorCodeType_None;
    }
}
