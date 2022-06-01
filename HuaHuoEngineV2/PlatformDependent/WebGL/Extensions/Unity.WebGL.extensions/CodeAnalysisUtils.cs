using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace UnityEditor.WebGL
{
    internal class CodeAnalysisUtils
    {
        static bool IsWordCharacter(char ch)
        {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '$';
        }

        static public Dictionary<string, string> ReadMinificationMap(string mapPath)
        {
            var result = new Dictionary<string, string>();
            if (File.Exists(mapPath))
                foreach (var line in File.ReadAllLines(mapPath))
                {
                    var split = line.Split(':');
                    result[split[0]] = split[1];
                }
            return result;
        }

        static public void ExtractFunctionsFromJS(string code, Action<string, string> processFunction, Action<string> processOther)
        {
            string curFunction = null;

            int index = code.IndexOf("// EMSCRIPTEN_START_FUNCS");
            int endIndex = code.IndexOf("// EMSCRIPTEN_END_FUNCS");
            int oldIndex = 0;
            int codeStartIndex = 0;
            while (true)
            {
                index = code.IndexOf("function ", index);
                if (index == -1)
                    break;

                if (index > 0 && IsWordCharacter(code[index - 1]))
                    continue;

                if (index > endIndex)
                    break;

                if (curFunction != null)
                {
                    var funcCode = code.Substring(oldIndex, index - oldIndex);
                    processFunction(curFunction, funcCode);
                }
                else
                    processOther(code.Substring(oldIndex, index - oldIndex));
                oldIndex = index;
                index += 9;
                codeStartIndex = code.IndexOf('(', index);
                curFunction = code.Substring(index, codeStartIndex - index);
            }
            processOther(code.Substring(oldIndex));
        }

        static string ReplaceDuplicates(string codeIn, Dictionary<string, string> minificationMap)
        {
            StringBuilder patchedCode = new StringBuilder();
            var hashFuncs = new Dictionary<string, List<string>>();
            var functionReplacement = new Dictionary<string, string>();
            ExtractFunctionsFromJS(codeIn,
                (name, code) => {
                    var codeStartIndex = code.IndexOf('(');
                    var hashStr = code.Substring(codeStartIndex);
                    if (!hashFuncs.ContainsKey(hashStr))
                    {
                        hashFuncs[hashStr] = new List<string>();
                        patchedCode.Append(code);
                    }
                    else
                    {
                        functionReplacement[name] = hashFuncs[hashStr][0];
                        minificationMap.Remove(name);
                        minificationMap.Remove(hashFuncs[hashStr][0]);
                    }
                    hashFuncs[hashStr].Add(name);
                },
                (code) => {
                    patchedCode.Append(code);
                }
            );

            var codeStr = patchedCode.ToString();
            patchedCode = new StringBuilder();
            var currentWord = "";
            bool functionTable = false;
            bool exportObjectSymbols = false;
            foreach (char ch in codeStr)
            {
                if (IsWordCharacter(ch))
                    currentWord += ch;
                else
                {
                    if (currentWord.Length > 0)
                    {
                        if (currentWord == "EMSCRIPTEN_END_FUNCS")
                            functionTable = true;
                        if (functionTable && currentWord == "return")
                            exportObjectSymbols = true;
                        // Replace occurances of duplicated functions in...
                        if (
                            // ...function calls
                            ch == '(' ||
                            // ...function table entries
                            (functionTable && !exportObjectSymbols) ||
                            // ...object exported symbols - but only for the function reference, not for the exported name.
                            (exportObjectSymbols && ch != ':')
                        )
                        {
                            if (functionReplacement.ContainsKey(currentWord))
                            {
                                currentWord = functionReplacement[currentWord];
                            }
                        }
                        patchedCode.Append(currentWord);
                        currentWord = "";
                    }

                    patchedCode.Append(ch);
                }
            }
            codeStr = patchedCode.ToString();

            System.Console.WriteLine("" + functionReplacement.Count + " functions replaced.");
            return codeStr;
        }

        static public void ReplaceDuplicates(string asmPath, string symbolsPath, string symbolsStrippedPath, int interations)
        {
            var minificationMap = ReadMinificationMap(symbolsPath);

            var code = File.ReadAllText(asmPath);
            for (int i = 0; i < interations; i++)
                code = ReplaceDuplicates(code, minificationMap);
            File.WriteAllText(asmPath, code);

            using (StreamWriter file = new StreamWriter(symbolsStrippedPath))
            {
                foreach (KeyValuePair<string, string> symbol in minificationMap)
                    file.Write(symbol.Key + ":" + symbol.Value + "\n");
            }
        }
    }
}
