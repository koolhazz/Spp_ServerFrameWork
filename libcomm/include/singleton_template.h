
#ifndef _SINGLETON_TEMPLATE_H_
#define _SINGLETON_TEMPLATE_H_

template <class TYPE>
class Singleton
{
    public:
        static TYPE * instance(void)
        {
            if(m_singleton == NULL)
            {
                m_singleton = new Singleton;
            }
            return &m_singleton->m_instance;
        }
    protected:
        Singleton();
        TYPE m_instance;
        static Singleton<TYPE> *m_singleton;

};


template<class TYPE>
Singleton<TYPE>* Singleton<TYPE>::m_singleton = NULL;

template<class TYPE>
Singleton<TYPE>::Singleton()
{

}


#endif 
