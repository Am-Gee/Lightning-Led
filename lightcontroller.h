#ifndef CLIGHTCONTROLLER_H
#define CLIGHTCONTROLLER_H
#include "defines.h"
#include "executor.h"
#include "timer.h"
#include "softpwm.h"
#include "perlinnoise.h"
#include "communicator.h"
#include "csimulatedclock.h"

#define _USE_SIM_TIMER


struct DimChannel
{
    int iPinNumber;
    short iCurrentDimModifier;
    float fCurrentDimValue;
    float fAmount;
    DimTime startTime;
    DimTime stopTime;
    int iDuration;
};


enum LightEffect
{
    FADE = 0,
    IMMEDIATE = 1
};

enum LightingMode
{
   NIGHT, // 0
   SUNRISE, // 1
   SUNSET, // 2
   CLEAR, // 3
   CLOUDS, // 4
   PARTLY_CLOUDY, // 5
   MOSTLY_CLOUDS, // 6
   DEMO
};

enum ControllerState
{
    OFF,
    HANDSHAKE,
    RUNNING
};

enum ControllerMode
{
    REMOTE_CONTROLLED,
    STANDALONE
};

class CLightController : public Executor
{
    public:

        CLightController(OneWire* pDS, SoftPwm* pSoftPwm, Communicator* pCom);
        virtual ~CLightController();

        virtual void SetValue(int value, float fValue);
        virtual float GetValue(int value);
        virtual long Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer);

        void Setup();
        void Update();

        void ClearFrameBuffer(bool clearTargetBuffer, float fValue = 0);

        void DrawFrameBuffer();
        void SetLightingMode(LightingMode mode);
/*
        struct DimGroupConfig
        {
            short iPinNumber;
            DimTime startTime;
            DimTime stopTime;
            int iDuration;
        };

        struct Configuration
        {
            int nGroupConfigs;

            LightingMode lastMode;

            DimGroupConfig* pGroupConfigs;
        };
*/

    protected:
        bool m_bFirstImage;

    private:
        void SetSunset(byte hour, byte minute);
        void SetSunrise(byte hour, byte minute);
        void RenderBuffer();
        //bool ReadConfiguration();
        //void WriteConfiguration();
        void SetEffectMode(LightEffect effect, long effectTime = 0L);
        void UpdateDemo();
        void UpdateClouds();
        void CreateCloudImages();
        void CalculateNightTime();
        void UpdateCloudAnimationParams(long speed);
        void EnsureNightTime();

        void SetPixel(float* pBuffer, byte x, byte y, float fVal);

        void SetMoon(long fadeTime);

        //Configuration m_Configuration;

        //PerlinNoise m_PerlinNoise;

        SoftPwm* m_pSoftPwm;
        Communicator* m_pCommunicator;

        LightingMode m_LightingMode;
        LightingMode m_TargetLightingMode;
        LightingMode m_TargetLightingModeAfterSunrise;

        bool m_bRemoteDataRequested;
        bool m_bDrawBuffer;
        unsigned long m_lLastMillis;
        unsigned long m_lLastCloudImageMillis;
        unsigned long m_lLastUpdateMillis;
        long m_lEffectCounter;
        long m_lMaxEffectSteps;
        float m_fFadeSteps;
        int m_cloudIterations;
//        int m_currentCloudIteration;
        long m_lFadeFrames;
        float m_fCloudParamZoom;
        float m_fCloudParamP;

        byte m_iCloudOctaves;

        byte m_iCurrentDayOfMonth;

        byte m_iNightWatchDogCounter;

        byte m_iSunriseHour;
        byte m_iSunriseMinute;
        byte m_iSunsetHour;
        byte m_iSunsetMinute;

        byte m_iNightHour;
        byte m_iNightMinute;

        byte m_PixelX;
        byte m_PixelY;

        int m_iSunSetSunRiseTime;
        long m_lCloudImageDuration;
        byte m_iMoonLightPin;
        float m_fMoonLightValue;
        LightEffect m_currentEffect;
        ControllerMode m_controllerMode;
        ControllerState m_controllerState;
        byte m_ScrollY;

#ifndef USE_SIM_TIMER
        CTimer m_Timer;
#else
        CSimulatedClock m_Timer;
#endif

  private:

        inline void CalculateFading(){ m_fFadeSteps = (float)(MAX_PULSE) / (float)m_lFadeFrames; //((float)FRAMES_PER_SEC * ((float)m_lFadeFrames / (float) FRAMES_PER_SEC) );
                                m_lMaxEffectSteps = m_lFadeFrames;}; //((float)FRAMES_PER_SEC * ((float)m_lFadeFrames / (float) FRAMES_PER_SEC) );};

};

#endif // CLIGHTCONTROLLER_H
