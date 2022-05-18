//
// Created by VincentZhang on 5/17/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICEASYNCCOMMAND_H
#define HUAHUOENGINE_GFXDEVICEASYNCCOMMAND_H


class GfxDevice;
struct GfxContextData;
struct GfxDeviceAsyncCommand {
    struct Arg { //: public ThreadSharedObject<Arg> {
        Arg(/*MemLabelId label*/) {} //: ThreadSharedObject<Arg>(label) {}

        virtual ~Arg() {}
    };

    // Scratch data for graphics jobs.
    // Can be used to setup special per-job cleanup.
    // Can be used to index into the main data when splitting up jobs.
    struct ArgScratch : public GfxDeviceAsyncCommand::Arg
    {
        GfxDevice* device; // Assigned in ExecuteAsync
        GfxContextData* contextData;
        UInt8* serializedCBData;
#if GFXDEVICE_DEBUG_SETTINGS
        float sleepAtStart;
#endif
        bool hasGrabPass;

        ArgScratch(/*MemLabelId label*/)
                //: Arg(label)
                : device(NULL)
                , contextData(NULL)
#if GFXDEVICE_DEBUG_SETTINGS
                , sleepAtStart(0.0f)
#endif
                , hasGrabPass(false)
        {
        }

        // This will (and should) be executed only on the job thread. It's a good place to cleanup the queue and such.
        virtual void ThreadedCleanup()
        {
            device = NULL;
        }

#if GFXDEVICE_DEBUG_SETTINGS
        void SetSleepAtStart(float time)
        {
            sleepAtStart = time;
        }

        void SleepAtStart() const
        {
            if (sleepAtStart > 0.0f)
                Thread::Sleep(sleepAtStart);
        }

#else
        void SetSleepAtStart(float time) {}
        void SleepAtStart() const {}
#endif
    };

    typedef void Func (ArgScratch*, const Arg*);

    Func*       func;
    ArgScratch* argScratch;
    const Arg*  argShared;
    void*       customData;     // customData is for device specific information

    GfxDeviceAsyncCommand() : func(NULL) , argScratch(NULL), argShared(NULL), customData(NULL) {}
    GfxDeviceAsyncCommand(Func* f, ArgScratch* scratch, const Arg* arg, void* data = NULL) : func(f) , argScratch(scratch), argShared(arg), customData(data) {}
};
#endif //HUAHUOENGINE_GFXDEVICEASYNCCOMMAND_H
