using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security;
using System.Text;
using UnityEditor.Utils;
using UnityEditorInternal;

namespace UnityEditor.Windows
{
    class VisualStudioProjectHelpers
    {
        // Template arguments:
        // {0} - Source files
        // {1} - Defines
        // {2} - Additional commands passed to building driver (-define=xxx -define=yyy)
        // {3} - .NET Profile
        public static void WriteIl2CppOutputProject(BuildTargetGroup buildTargetGroup, string il2CppOutputProjectDirectory, string projectTemplate, string additionalDefines, IIl2CppPlatformProvider il2cppPlatformProvider)
        {
            string[] sourceFiles;
            string projectItems, filterItems;
            var targetPath = Path.Combine(il2CppOutputProjectDirectory, "Il2CppOutputProject.vcxproj");

            using (new ProfilerBlock("VisualStudioProjectHelpers.FindSourceFilesForIl2CppOutputProject"))
                sourceFiles = FindSourceFilesForIl2CppOutputProject(il2CppOutputProjectDirectory);

            using (new ProfilerBlock("VisualStudioProjectHelpers.MakeProjectItems"))
                projectItems = MakeProjectItems(sourceFiles, il2CppOutputProjectDirectory, true);

            using (new ProfilerBlock("VisualStudioProjectHelpers.MakeFilterItems"))
                filterItems = MakeFilterItems(sourceFiles, il2CppOutputProjectDirectory);

            using (new ProfilerBlock("VisualStudioProjectHelpers.WriteIl2CppOutputProjectFile"))
                WriteIl2CppOutputProjectFile(buildTargetGroup, projectTemplate, projectItems, filterItems, targetPath, additionalDefines, il2cppPlatformProvider);
        }

        private static string[] FindSourceFilesForIl2CppOutputProject(string il2CppOutputProjectDirectory)
        {
            // Don't include generated header files because there are so many of them and it brings VS to a crawl
            var generatedSourceFiles = new[] { ".c", ".cpp" }.SelectMany(extension => Directory.GetFiles(Paths.Combine(il2CppOutputProjectDirectory, "Source", "il2cppOutput"), "*" + extension, SearchOption.AllDirectories));

            var cppPluginSourceFiles = Enumerable.Empty<string>();
            var cppPluginsFolder = Paths.Combine(il2CppOutputProjectDirectory, "Source", "CppPlugins");
            if (Directory.Exists(cppPluginsFolder))
                cppPluginSourceFiles = new[] { ".c", ".cpp", ".h" }.SelectMany(extension => Directory.GetFiles(cppPluginsFolder, "*" + extension, SearchOption.AllDirectories));

            var libil2cppSourceFiles = new[] { ".c", ".cpp", ".h" }.SelectMany(extension => Directory.GetFiles(Path.Combine(il2CppOutputProjectDirectory, "IL2CPP"), "*" + extension, SearchOption.AllDirectories));
            return generatedSourceFiles.Concat(cppPluginSourceFiles).Concat(libil2cppSourceFiles).Where(path => !path.Contains("IL2CPP\\MapFileParser")).ToArray();
        }

        private static void WriteIl2CppOutputProjectFile(BuildTargetGroup buildTargetGroup, string projectTemplate, string projectItems, string filterItems, string targetPath, string additionalDefines, IIl2CppPlatformProvider il2cppPlatformProvider)
        {
            var builderArgs = IL2CPPUtils.GetBuildingIL2CPPArguments(il2cppPlatformProvider, buildTargetGroup);
            var debuggerArgs = IL2CPPUtils.GetDebuggerIL2CPPArguments(il2cppPlatformProvider, buildTargetGroup);
            var debuggerArgsString = debuggerArgs.Any() ? debuggerArgs.Aggregate((x, y) => x + " " + y) : string.Empty;
            var definesArgs = additionalDefines.Split(new[] { ';' }).Select(d => "--additional-defines=" + d);
            var additionalArgs = builderArgs.Concat(definesArgs).Aggregate((x, y) => x + " " + y);
            var apiCompatibilityLevel = PlayerSettings.GetApiCompatibilityLevel(buildTargetGroup);
            var dotNetProfile = IL2CPPUtils.ApiCompatibilityLevelToDotNetProfileArgument(apiCompatibilityLevel);
            var defines = IL2CPPUtils.GetBuilderDefinedDefines(il2cppPlatformProvider, buildTargetGroup).Aggregate((x, y) => x + ";" + y) + ";" + additionalDefines;
            var projectText = string.Format(projectTemplate, projectItems, defines, additionalArgs, dotNetProfile, debuggerArgsString);

            FileUtil.DeleteFileOrDirectory(targetPath);
            File.WriteAllText(targetPath, projectText, Encoding.UTF8);

            var filtersPath = targetPath + ".filters";
            var filtersText = string.Format(GetFiltersTemplate(), filterItems);

            FileUtil.DeleteFileOrDirectory(filtersPath);
            File.WriteAllText(filtersPath, filtersText, Encoding.UTF8);
        }

        public static string MakeProjectItems(IEnumerable<string> files, string projectDirectory, bool excludeFromResourceIndex, string pathPrefix = "", Dictionary<string, string> predeterminedTags = null, Dictionary<string, string> additionalItemAttributes = null)
        {
            var projectItems = new StringBuilder();

            foreach (var file in files)
            {
                var fileNameWithoutExtension = Path.GetFileNameWithoutExtension(file);
                var relativePath = EscapeXML(pathPrefix + Paths.MakeRelativePath(projectDirectory, file));
                var fileTag = DetermineFileTag(file, predeterminedTags);
                var additionalAttributes = DetermineAdditionalAttributes(relativePath, additionalItemAttributes);

                projectItems.AppendLine($@"    <{fileTag} Include=""{relativePath}""{additionalAttributes}>");

                switch (fileTag)
                {
                    case "ClCompile":
                        if (Path.GetFileName(file).Equals("pch.cpp", StringComparison.InvariantCultureIgnoreCase))
                            projectItems.AppendLine("      <PrecompiledHeader>Create</PrecompiledHeader>");

                        goto case "ClInclude";

                    case "ClInclude":
                        if (Path.GetFileNameWithoutExtension(file).EndsWith(".xaml", StringComparison.InvariantCultureIgnoreCase))
                            projectItems.AppendFormat("      <DependentUpon>{0}</DependentUpon>{1}", fileNameWithoutExtension, Environment.NewLine);

                        break;

                    case "None":
                        if (!string.Equals(Path.GetExtension(file), ".pfx", StringComparison.OrdinalIgnoreCase) && // Don't deploy certificates. Makes WACK fail.
                            !string.Equals(Path.GetExtension(file), ".debug", StringComparison.OrdinalIgnoreCase)) // Don't deploy POSIX debug symbols
                        {
                            projectItems.AppendLine("      <DeploymentContent>true</DeploymentContent>");

                            // Exlude Unity generated files from resource indexing but not user files
                            if (excludeFromResourceIndex)
                            {
                                projectItems.AppendLine("      <ExcludeFromResourceIndex>true</ExcludeFromResourceIndex>"); // case 976639
                            }
                        }
                        break;

                    case "Reference":
                        if (Path.GetExtension(file) == ".winmd")
                            projectItems.AppendLine("      <IsWinMDFile>true</IsWinMDFile>");
                        break;

                    case "AppxManifest":
                    case "Page":
                        projectItems.AppendLine("      <SubType>Designer</SubType>");
                        break;
                }

                projectItems.AppendFormat("    </{0}>{1}", fileTag, Environment.NewLine);
            }

            return projectItems.ToString();
        }

        public static string MakeFilterItems(IEnumerable<string> files, string UserProjectDirectory, string pathPrefix = "", Dictionary<string, string> predeterminedTags = null)
        {
            var filterItems = new StringBuilder();
            var filters = new Dictionary<string, string>(StringComparer.InvariantCultureIgnoreCase);

            foreach (var file in files)
            {
                var relativePath = Paths.MakeRelativePath(UserProjectDirectory, file);
                var fileTag = DetermineFileTag(file, predeterminedTags);
                var folder = Path.GetDirectoryName(relativePath);
                relativePath = EscapeXML(pathPrefix + relativePath);

                if (string.IsNullOrEmpty(folder))
                {
                    filterItems.AppendFormat(@"    <{0} Include=""{1}"" />{2}", fileTag, relativePath, Environment.NewLine);
                    continue;
                }

                if (!filters.ContainsKey(folder))
                    filters.Add(folder, Guid.NewGuid().ToString());

                filterItems.AppendFormat(@"    <{0} Include=""{1}"">{2}", fileTag, relativePath, Environment.NewLine);
                filterItems.AppendFormat(@"      <Filter>{0}</Filter>{1}", folder, Environment.NewLine);
                filterItems.AppendFormat(@"    </{0}>{1}", fileTag, Environment.NewLine);
            }

            var filtersToAdd = new HashSet<string>();

            foreach (var originalFilter in filters)
            {
                var filter = Path.GetDirectoryName(originalFilter.Key);

                while (!string.IsNullOrEmpty(filter))
                {
                    if (!filters.ContainsKey(filter))
                        filtersToAdd.Add(filter);

                    filter = Path.GetDirectoryName(filter);
                }
            }

            foreach (var filter in filtersToAdd)
                filters.Add(filter, Guid.NewGuid().ToString());

            var filtersSection = new StringBuilder();

            foreach (var filter in filters)
            {
                filtersSection.AppendFormat(@"    <Filter Include=""{0}"">{1}", filter.Key, Environment.NewLine);
                filtersSection.AppendFormat(@"      <UniqueIdentifier>{{{0}}}</UniqueIdentifier>{1}", filter.Value, Environment.NewLine);
                filtersSection.AppendFormat(@"    </Filter>{0}", Environment.NewLine);
            }

            return filtersSection + filterItems.ToString();
        }

        private static string DetermineFileTag(string file, Dictionary<string, string> predeterminedTags)
        {
            string tag;
            if (predeterminedTags != null && predeterminedTags.TryGetValue(file, out tag))
                return tag;

            if (Path.GetFileName(file).Equals("App.xaml", StringComparison.InvariantCultureIgnoreCase))
                return "ApplicationDefinition";

            switch (Path.GetExtension(file))
            {
                case ".cpp":
                case ".c":
                    return "ClCompile";

                case ".h":
                    return "ClInclude";

                case ".appxmanifest":
                    return "AppxManifest";

                case ".xaml":
                    return "Page";

                case ".winmd":
                    return "Reference";

                case ".res":
                    return "Resource";

                default:
                    return "None";
            }
        }

        private static string DetermineAdditionalAttributes(string file, Dictionary<string, string> additionalItemAttributes)
        {
            string attributes;
            if (additionalItemAttributes == null || !additionalItemAttributes.TryGetValue(file, out attributes))
                return string.Empty;

            return " " + attributes;
        }

        public static string GetFiltersTemplate()
        {
            // {0} - items
            return string.Join(Environment.NewLine, new[]
            {
                @"<?xml version=""1.0"" encoding=""utf-8""?>",
                @"<Project ToolsVersion=""14.0"" xmlns=""http://schemas.microsoft.com/developer/msbuild/2003"">",
                @"  <ItemGroup>",
                @"{0}  </ItemGroup>",
                @"</Project>"
            });
        }

        public static string EscapeXML(string str)
        {
            // MSBUILD/VS doesn't like &apos; for some reason. It digests %27 as an apotrophe just fine, though
            return SecurityElement.Escape(str.Replace("'", "%27"));
        }
    }
}
