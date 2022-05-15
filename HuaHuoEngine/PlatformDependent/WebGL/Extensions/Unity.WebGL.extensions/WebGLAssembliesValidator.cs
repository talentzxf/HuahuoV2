#if ENABLE_WEBGL_ASSEMBLY_VALIDATION
using System.Text.RegularExpressions;
using UnityEditor.Modules;

namespace UnityEditor.WebGL
{
    class WebGLAssembliesValidator : IUserAssembliesValidator
    {
        // private static readonly Regex s_ProcessingAssemblyRegex = new Regex(@"\[icall-verifier\] Processing Assembly: (.*)");
        // private static readonly Regex s_InternalCallContextRegex = new Regex(@"[icall-verifier] '([^']+)' in '([^']+)'");
        private static readonly Regex s_UnsupportedICallRegex = new Regex(@"\[icall-verifier\] Found unsupported icall '([^']+)' referenced by:");

        private InternalCallVerifierProgram m_Verifier;

        public bool canRunInBackground
        {
            get { return true; }
        }

        public void Validate(string[] userAssemblies)
        {
            m_Verifier = InternalCallVerifierProgram.Managed(userAssemblies);

            m_Verifier.OutputDataReceived += (process, line) => ProcessOutputMessage(line.Data);
            m_Verifier.ErrorDataReceived += (process, line) => ProcessErrorMessage(line.Data);
            m_Verifier.Start();
        }

        private static void ProcessOutputMessage(string message)
        {
            if (string.IsNullOrEmpty(message))
                return;

            var match = s_UnsupportedICallRegex.Match(message);
            if (match.Success)
                UnityEngine.Debug.LogWarning("Your code might invoke an unsupported internal call: " + match.Groups[1].Value);
        }

        private static void ProcessErrorMessage(string message)
        {
            if (string.IsNullOrEmpty(message))
                return;

            UnityEngine.Debug.LogError("ERROR: " + message);
        }

        public void Cleanup()
        {
            if (m_Verifier == null)
                return;

            m_Verifier.Dispose();
            m_Verifier = null;
        }
    }
}
#endif
