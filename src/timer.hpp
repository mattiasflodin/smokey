#include <Windows.h>
#include <stdexcept>

class timer
{
public:
    timer()
    {
        if (TIMERR_NOERROR != timeBeginPeriod(1))
            throw std::runtime_error("cannot set global timer resolution");
        _start = timeGetTime();
    }
    ~timer()
    {
        timeEndPeriod(1);
    }

    unsigned get()
    {
        return (timeGetTime() - _start);
    }

    void sleep_for(unsigned milliseconds)
    {
        Sleep(milliseconds);
    }

    void sleep_until(unsigned milliseconds)
    {
        DWORD deadline = _start;
        deadline += milliseconds;
        DWORD now = timeGetTime();
        if(now < deadline) {
            Sleep(deadline - now - 1);
        }
        while(timeGetTime() < deadline) {
            Sleep(0);
        }
    }

private:
    DWORD _start;
};
