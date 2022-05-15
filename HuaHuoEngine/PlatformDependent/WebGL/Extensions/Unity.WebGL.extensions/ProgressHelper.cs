using System;

namespace UnityEditor.WebGL
{
    internal class ProgressHelper
    {
        internal float m_CurrentBuildStep = 0f;
        internal float m_NumBuildSteps = 0f;

        public void Reset(float numSteps)
        {
            m_CurrentBuildStep = 0f;
            m_NumBuildSteps = numSteps;
        }

        public float Advance()
        {
            return ++m_CurrentBuildStep / m_NumBuildSteps;
        }

        public float Get()
        {
            return m_CurrentBuildStep / m_NumBuildSteps;
        }

        public float LastValue()
        {
            return (m_CurrentBuildStep - 1f) / m_NumBuildSteps;
        }

        public void Show(string title, string message)
        {
            if (EditorUtility.DisplayCancelableProgressBar(title, message, Get()))
                throw new Exception(title);
        }

        public void Step(string title, string message)
        {
            Advance();
            Show(title, message);
        }
    }
}
