#pragma once


namespace win
{
    template<typename T>
    class AutoVectorPtr
    {
    private:
        T *ptr;
        std::size_t m_CurrentSize;

    public:
        AutoVectorPtr()
            : ptr(NULL)
            , m_CurrentSize(0u)
        {}
        inline ~AutoVectorPtr(void) { this->Free(); }

        inline operator T *(void) { return this->ptr; }
        inline operator const T *(void) const { return this->ptr; }

        inline T *GetData(void) { return this->ptr; }
        inline const T *GetData(void) const { return this->ptr; }

        std::size_t Size() const
        {
            return m_CurrentSize;
        }

        bool Allocate(std::size_t length)
        {
            if (m_CurrentSize < length)
            {
                this->Free();

                if (NULL == (this->ptr = new T[length]))
                {
                    return false;
                }
            }
            m_CurrentSize = length;
            return true;
        }

        void Free(void)
        {
            delete[] this->ptr;
            this->ptr = NULL;
            m_CurrentSize = 0u;
        }

        void Swap(AutoVectorPtr<T>& rhs)
        {
            std::swap(this->ptr, rhs.ptr);
            std::swap(this->m_CurrentSize, rhs.m_CurrentSize);
        }
    };
}
