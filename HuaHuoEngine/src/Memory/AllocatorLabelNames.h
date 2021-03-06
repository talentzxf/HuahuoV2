DO_LABEL(Default)
DO_LABEL(Permanent)
DO_LABEL(Log)
DO_LABEL(NewDelete)
DO_LABEL(MallocFree)
DO_LABEL(Thread)
DO_LABEL(PVS)
DO_LABEL(Manager)
DO_LABEL(DynamicGeometry)
DO_LABEL(VertexData)
DO_LABEL(ImmediateGeometry)
DO_LABEL(Geometry)
DO_LABEL(BatchedGeometry)
DO_LABEL(Particles)
DO_LABEL(VFX)
DO_LABEL(Texture)
DO_LABEL(Shader)
DO_LABEL(Material)
DO_LABEL(TextureCache)
DO_LABEL(GfxDevice)
DO_LABEL(GfxThread)
DO_LABEL(Animation)
DO_LABEL(Audio)
DO_LABEL(AudioData)
DO_LABEL(AudioProcessing)
DO_LABEL(AudioTemp)
DO_LABEL(FMODOther)
DO_LABEL(FMODStreamFile)
DO_LABEL(FMODStreamDecode)
DO_LABEL(FMODSample)
DO_LABEL(Font)
DO_LABEL(FontEngine)
DO_LABEL(Physics)
DO_LABEL(Physics2D)
DO_LABEL(Serialization)
DO_LABEL(Input)
DO_LABEL(IO)
DO_LABEL(IO2)
DO_LABEL(ThreadStack)
DO_LABEL(JobScheduler)
DO_LABEL(TextAsset)
DO_LABEL(GarbageCollector)
DO_LABEL(GLib)
DO_LABEL(GLibImage)
DO_LABEL(Mono)
DO_LABEL(MonoCode)
DO_LABEL(ScriptingNativeRuntime)
DO_LABEL(BaseObject)
DO_LABEL(Resource)
DO_LABEL(Renderer)
DO_LABEL(Transform)
DO_LABEL(File)
DO_LABEL(Network)
DO_LABEL(WebCam)
DO_LABEL(Profiler)
DO_LABEL(MemoryProfiler)
DO_LABEL(MemoryProfilerString)
DO_LABEL(Culling)
DO_LABEL(Skinning)
DO_LABEL(Terrain)
DO_LABEL(TerrainPhysics)
DO_LABEL(Wind)
DO_LABEL(Shadow)
DO_LABEL(STL)
DO_LABEL(String)
DO_LABEL(StaticString)
DO_LABEL(DynamicArray)
DO_LABEL(HashMap)
DO_LABEL(Pair)
DO_LABEL(UTF16String)
DO_LABEL(Utility)
DO_LABEL(Curl)
DO_LABEL(PoolAlloc)
DO_LABEL(AI)
DO_LABEL(TypeTree)
DO_LABEL(ScriptManager)
DO_LABEL(ScriptEventManager) // Added by VZ
DO_LABEL(RuntimeInitializeOnLoadManager)
DO_LABEL(AncestorCache)
DO_LABEL(Sprites)
DO_LABEL(SpriteAtlas)
DO_LABEL(GI)                        // Enlighten: runtime realtime GI calculations
DO_LABEL(Unet)
DO_LABEL(ClusterRenderer)
DO_LABEL(ClusterInput)
DO_LABEL(Director)
DO_LABEL(GPUMemory)
DO_LABEL(CloudService)
DO_LABEL(WebRequest)
DO_LABEL(VR)
DO_LABEL(AR)
DO_LABEL(SceneManager)
DO_LABEL(Video)
DO_LABEL(LazyScriptCache)
DO_LABEL(Tilemap)
DO_LABEL(Speech)
DO_LABEL(SceneLoad)
DO_LABEL(ManagedAttributeManager)
DO_LABEL(TextureStreaming)
DO_LABEL(iOSReplayKit)
DO_LABEL(NativeArray)
DO_LABEL(ScreenCapture)
DO_LABEL(Camera)
DO_LABEL(Secure)
DO_LABEL(Yoga)
DO_LABEL(Image)
DO_LABEL(CrashReporter)
DO_LABEL(SerializationCache)
DO_LABEL(APIUpdating)
DO_LABEL(Subsystems)
DO_LABEL(VirtualTexturing)
DO_LABEL(StaticSafetyDebugInfo)


// For native testing.
DO_LABEL(Test)

// Editor Specific
DO_LABEL(EditorGui)
DO_LABEL(EditorUtility)
DO_LABEL(VersionControl)
DO_LABEL(UndoBuffer)
DO_LABEL(Undo)
DO_LABEL(AssetDatabase)
DO_LABEL(AssetUsage)
DO_LABEL(StreamingManager)
DO_LABEL(PreviewImage)
DO_LABEL(AssetImporter)
DO_LABEL(RestService)
DO_LABEL(FBXImporter)
DO_LABEL(EditorGi)                  // Enlighten: precomputing realtime GI and generating baked GI
DO_LABEL(ProgressiveLightmapper)    // Progressive Lightmapper: generating baked GI
DO_LABEL(Yaml)
DO_LABEL(License)
DO_LABEL(UnityConnect)
DO_LABEL(WebViewCallback)
DO_LABEL(Collab)
DO_LABEL(Upm)
DO_LABEL(UpmDiagnostics)
DO_LABEL(DrivenProperties)
DO_LABEL(HubClient)
DO_LABEL(WebSocketClient)
DO_LABEL(ChannelService)
DO_LABEL(LocalIPC)
DO_LABEL(ProfilerEditor)
DO_LABEL(CoreBusinessMetrics)


// Labels for temporary allocations without profiler information
DO_TEMP_LABEL(TempAlloc)
DO_LABEL(TempOverflow)

// For jobs (almost locking-free) - with lifetime check
DO_TEMP_LABEL(TempJobAlloc)             // alias for TempJob4Frame
DO_TEMP_LABEL(TempJob1Frame)            // checks 1 frame lifetime
DO_TEMP_LABEL(TempJob2Frame)            // checks 2 frame lifetime
DO_TEMP_LABEL(TempJob4Frame)            // checks 4 frame lifetime

// Temporary allocations that (potentially) last more than a few frames (no lifetime check)
DO_TEMP_LABEL(TempBackgroundJobAlloc)   // alias for TempJobAsync
DO_TEMP_LABEL(TempJobAsync)             // no lifetime checking

// Mapped to fallback allocator
// Used by code running before memory manager is initialized
DO_LABEL(Bootstrap)

// Shader microcode allocation, for platforms that do their own allocations for micro-code.
DO_LABEL(ShaderUcode)

DO_LABEL(TLS)

// Xbox One specific
DO_LABEL(XboxOneGpuOnionMemory)
DO_LABEL(XboxOneGpuGarlicMemory)

DO_LABEL(InvalidLabel)

// SketchUp
DO_LABEL(SketchUp)

//Switch Specific
DO_LABEL(SwitchDynLibMemory)

// Licensing
DO_LABEL(Licensing)

DO_LABEL(AssetReference)
DO_LABEL(CachingManager)
