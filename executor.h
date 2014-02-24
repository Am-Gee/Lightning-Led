#ifndef EXECUTOR_H
#define EXECUTOR_H
#include <arduino.h>


class Executor
{
    public:
        Executor();
        virtual ~Executor();

        virtual void SetValue(int value, float fValue) = 0;
        virtual float GetValue(int value) = 0;
        virtual long Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer) = 0;

    protected:
    private:
};

#endif // EXECUTOR_H
