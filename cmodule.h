#ifndef CMODULE_H
#define CMODULE_H


class CModule
{
    public:
        CModule();
        CModule(unsigned long lUpdateInterval);

        virtual ~CModule();

        virtual void Setup();
        virtual void Update();
        virtual void OnUpdate();

    protected:
        unsigned long m_lUpdateInterval;
        unsigned long m_lLastMillis;

    private:

};

#endif // CMODULE_H
