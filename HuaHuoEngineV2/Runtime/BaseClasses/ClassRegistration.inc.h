//
// To create a valid persistent-type-id for the builtins / non-hierarchy classes / structs:
// use the following perl expression and copy/paste its result as the id:
//      perl -e "printf('0x%08X', int(rand(0x80000000 - 20000)) + 20000)"
//

// builtins
HUAHUO_KERNEL_BUILTIN(int, 100000)
HUAHUO_KERNEL_BUILTIN(bool, 100001)
HUAHUO_KERNEL_BUILTIN(float, 100002)
HUAHUO_KERNEL_BUILTIN(void, 100011)

//// non-hierarchy classes
//HUAHUO_KERNEL_NONHIERARCHY_CLASS(Polygon2D, 100010)
//HUAHUO_KERNEL_NONHIERARCHY_CLASS(Vector3f, 100005)
//
//// non-hierarchy structs (generally message data)
//HUAHUO_KERNEL_STRUCT(AudioMixerLiveUpdateBool, 100009)
//HUAHUO_KERNEL_STRUCT(AudioMixerLiveUpdateFloat, 100008)
//HUAHUO_KERNEL_STRUCT(Collision, 100004)
//HUAHUO_KERNEL_STRUCT(Collision2D, 100007)
//HUAHUO_KERNEL_STRUCT(MonoObject, 100003)
//HUAHUO_KERNEL_STRUCT(RootMotionData, 100006)
//
//// We normally don't register classes manually here any more.
//// But some classes only have a native representation and no managed class corresponding to it.
//// For those, we use the old manual class registration system. But we generally don't recommend this
//// setup for new code. It is cleaner to have matching public scripting APIs for HUAHUO classes containing
//// serialized data.
//
//HUAHUO_KERNEL_CLASS(LevelGameManager)
//HUAHUO_KERNEL_CLASS(GlobalGameManager)
//HUAHUO_KERNEL_CLASS(ResourceManager)
//HUAHUO_KERNEL_CLASS(GameManager)
//HUAHUO_KERNEL_CLASS(ScriptMapper)
//HUAHUO_KERNEL_CLASS(EditorExtension)
//HUAHUO_KERNEL_CLASS(NamedObject)
//HUAHUO_KERNEL_CLASS(DelayedCallManager)
//HUAHUO_KERNEL_CLASS(CGProgram)
//HUAHUO_KERNEL_CLASS(HaloLayer)
//HUAHUO_KERNEL_CLASS(BuildSettings)
//HUAHUO_KERNEL_CLASS(RuntimeInitializeOnLoadManager)
//HUAHUO_KERNEL_CLASS(MonoScript)
//HUAHUO_KERNEL_CLASS(PlayerSettings)
//HUAHUO_KERNEL_CLASS(MonoManager)
//HUAHUO_KERNEL_CLASS(TagManager)
//HUAHUO_KERNEL_CLASS(InputManager)
//HUAHUO_KERNEL_CLASS(TimeManager)
//
//#if PLATFORM_SWITCH
//HUAHUO_KERNEL_CLASS(MovieTexture)
//#endif
//
//#if ENABLE_UNIT_TESTS
//HUAHUO_KERNEL_CLASS(ObjectProduceTestTypes, Derived);
//HUAHUO_KERNEL_CLASS(ObjectProduceTestTypes, SubDerived);
//HUAHUO_KERNEL_CLASS(ObjectProduceTestTypes, SiblingDerived);
//HUAHUO_KERNEL_CLASS(EmptyObject);
//#endif
//
//#if ENABLE_UNIT_TESTS_WITH_FAKES
//HUAHUO_KERNEL_CLASS(FakeComponent)
//#endif
//
//#if PLATFORM_PS4
//HUAHUO_KERNEL_CLASS(TextureRawPS4)
//#endif
//
//#if PLATFORM_PS5
//HUAHUO_KERNEL_CLASS(TextureRawPS5)
//#endif
//
//#if HUAHUO_EDITOR
//HUAHUO_KERNEL_CLASS(AnnotationManager)
//HUAHUO_KERNEL_CLASS(AssetMetaData)
//HUAHUO_KERNEL_CLASS(CachedSpriteAtlas)
//HUAHUO_KERNEL_CLASS(CachedSpriteAtlasRuntimeData)
//HUAHUO_KERNEL_CLASS(EditorExtensionImpl)
//HUAHUO_KERNEL_CLASS(HierarchyState)
//HUAHUO_KERNEL_CLASS(InspectorExpandedState)
//HUAHUO_KERNEL_CLASS(Mesh3DSImporter)
//HUAHUO_KERNEL_CLASS(Prefab)
//HUAHUO_KERNEL_CLASS(PrefabInstance)
//HUAHUO_KERNEL_CLASS(SpriteAtlasDatabase)
//HUAHUO_KERNEL_CLASS(LightingDataAssetParent)
//HUAHUO_KERNEL_CLASS(LibraryAssetImporter)
//HUAHUO_KERNEL_CLASS(DefaultImporter)
//HUAHUO_KERNEL_CLASS(PreviewAnimationClip)
//HUAHUO_KERNEL_CLASS(FBXImporter)
//HUAHUO_KERNEL_CLASS(NativeFormatImporter)
////HUAHUO_KERNEL_CLASS(PrefabImporter)
//HUAHUO_KERNEL_CLASS(PlatformModuleSetup)
//HUAHUO_KERNEL_CLASS(AssetImporterLog)
//HUAHUO_KERNEL_CLASS(RayTracingShaderImporter)
//HUAHUO_KERNEL_CLASS(SceneVisibilityState)
//HUAHUO_KERNEL_CLASS(RuleSetFileAsset)
//HUAHUO_KERNEL_CLASS(RuleSetFileImporter)
//
//#if ENABLE_UNIT_TESTS_WITH_FAKES
//HUAHUO_KERNEL_CLASS(TestObjectWithSerializedArray);
//HUAHUO_KERNEL_CLASS(TestObjectWithSerializedAnimationCurve);
//HUAHUO_KERNEL_CLASS(TestObjectWithSpecialLayoutOne);
//HUAHUO_KERNEL_CLASS(TestObjectWithSpecialLayoutTwo);
//HUAHUO_KERNEL_CLASS(TestObjectVectorPairStringBool);
//HUAHUO_KERNEL_CLASS(TestObjectWithSerializedMapStringBool);
//HUAHUO_KERNEL_CLASS(TestObjectWithSerializedMapStringNonAlignedStruct);
//HUAHUO_KERNEL_CLASS(RendererFake);
//HUAHUO_KERNEL_CLASS(NativeObjectType);
//HUAHUO_KERNEL_CLASS(SerializableManagedHost);
//HUAHUO_KERNEL_CLASS(PropertyModificationsTargetTestObject);
//#endif
//
//#endif // HUAHUO_EDITOR
//
//#if ENABLE_UNIT_TESTS_WITH_FAKES && !HUAHUO_IOS
//HUAHUO_KERNEL_CLASS(SerializableManagedRefTestClass);
//#endif
