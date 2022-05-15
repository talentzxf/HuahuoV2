#pragma once


namespace win
{
    class __declspec(novtable)IResource
    {
    public:
        virtual void Hold(void) = 0;
        virtual void Release(void) = 0;
    };


    template<typename TResource>
    class ResourcePtr
    {
    private:
        TResource *ptr;

    public:
        inline ResourcePtr(void) : ptr(NULL) {}
        inline ~ResourcePtr(void) { this->Free(); }

        ResourcePtr(TResource *ptr)
        {
            if (NULL != (this->ptr = ptr))
            {
                this->ptr->Hold();
            }
        }

        ResourcePtr(const ResourcePtr &ptr)
        {
            if (NULL != (this->ptr = ptr.ptr))
            {
                this->ptr->Hold();
            }
        }

        inline operator TResource *(void) { return this->ptr; }
        inline operator const TResource *(void) const { return this->ptr; }

        inline TResource *operator->(void) { return this->ptr; }
        inline const TResource *operator->(void) const { return this->ptr; }

        inline TResource &operator*(void) { return *this->ptr; }
        inline const TResource &operator*(void) const { return *this->ptr; }

        //inline TResource **operator &(void) { return &this->ptr; }

        const ResourcePtr &operator=(TResource *ptr)
        {
            this->Free();

            if (NULL != (this->ptr = ptr))
            {
                this->ptr->Hold();
            }

            return *this;
        }

        const ResourcePtr &operator=(const ResourcePtr &ptr)
        {
            this->Free();

            if (NULL != (this->ptr = ptr.ptr))
            {
                this->ptr->Hold();
            }

            return *this;
        }

        void Free(void)
        {
            if (NULL != this->ptr)
            {
                this->ptr->Release();
                this->ptr = NULL;
            }
        }
    };

    typedef ResourcePtr<IResource> IResourcePtr;


    template<typename TObject, typename TResource = IResource>
    class __declspec(novtable)ResourceRoot :
        public TResource
    {
    private:
        UInt32 count;

    public:
        inline static TObject *Create(void)
        {
            return new TObject();
        }

        template<typename TArg1>
        inline static TObject *Create(TArg1 arg1)
        {
            return new TObject(arg1);
        }

        /*template <typename TArg1, typename TArg2>
        inline static TObject *Create(TArg1 arg1, TArg2 arg2)
        {
            return new TObject(arg1, arg2);
        }*/

        inline ResourceRoot(void) : count(0) {}

        virtual void Hold(void)
        {
            ++this->count;
        }

        virtual void Release(void)
        {
            if (0 == --this->count)
            {
                delete static_cast<TObject *>(this);
            }
        }
    };
}
