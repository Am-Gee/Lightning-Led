#include <Arduino.h>
#include <EEProm.h>

#include "digitalwritefast.h"

#include "timer.h"
#include "onewire.h"
#include "fancontroller.h"
#include "lightcontroller.h"

#define WIDTH 5
#define HEIGHT 3



#define FRAMES_PER_SEC 25

#define CLOUD_IMAGE_HEIGHT HEIGHT * 5

#define DEFAULT_CLOUD_SPEED 5000

#define LIGHTINGMODE 0
#define TARGETLIGHTINGMODEAFTERSUNRISE 1

#define MOONLIGHTVAL 10
#define MOONLIGHTPIN 11

#define VALUE_WATCHDOG_COUNTER 5

#define CREATE_NEW_CLOUD_IMAGE  20
#define CLOUD_IMAGE_PARAM_ZOOM  21
#define CLOUD_IMAGE_PARAM_P     22
#define CLOUD_SPEED             23
//#define CLOUD_IMAGE_BRIGHTNESS  24
#define CLOUD_IMAGE_OCTAVES     24

#define SUNRISE_HOUR 40
#define SUNRISE_MINUTE 41
#define SUNSET_HOUR 42
#define SUNSET_MINUTE 43

#define MASTER_CONNECT 100
#define MASTER_PING 101

#define INIT_IMAGE { \
    {0, 0, 0, 0, 0},\
    {1, 0, 1, 0, 1},\
    {0, 0, 0, 0, 0}

byte dimTable[256] = {
  0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   4,   4,
  4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,
  8,   8,   9,   9,   9,  10,  10,  10,  11,  11,  12,  12,  12,  13,  13,  14,
 14,  15,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  22,
 22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,  30,  30,  31,  32,
 33,  33,  34,  35,  36,  36,  37,  38,  39,  40,  40,  41,  42,  43,  44,  45,
 46,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
 61,  62,  63,  64,  65,  67,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,
 80,  81,  82,  83,  85,  86,  87,  89,  90,  91,  93,  94,  95,  97,  98,  99,
101, 102, 104, 105, 107, 108, 110, 111, 113, 114, 116, 117, 119, 121, 122, 124,
125, 127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151,
153, 155, 157, 159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182,
184, 186, 189, 191, 193, 195, 197, 199, 201, 204, 206, 208, 210, 212, 215, 217,
219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 243, 245, 248, 250, 253, 255 };

volatile const int SUNSET_SUNRISE_TIME = 2700; // 45 min

const unsigned long UPDATE_IMAGE = 5000;
const unsigned long UPDATE_CLOUD_IMAGE = 3500;

const unsigned long INTERVAL = 1000 / FRAMES_PER_SEC;
/*
           8        45            12           3?       53

                46           11   7    13          6

           50       44            51          4       52

           6 -> 12
           5 -> 6

*/


volatile uint8_t arrDisplayPins[WIDTH][HEIGHT] = {   8,  46, 50,
                                            45, 11, 44,
                                            12,  7, 51,
                                             3, 13, 4,
                                             53,  6, 52
                                             };

byte arrDisplayValues[WIDTH][HEIGHT] = {   0,  0, 0,
                                            0, 0, 0,
                                            0,  0, 0,
                                             0, 0, 0,
                                             0,  0, 0
                                             };


const static int FRAMEBUFFER_SIZE = WIDTH * HEIGHT * sizeof(float);

float frameBuffer[HEIGHT * WIDTH];

float targetBuffer[HEIGHT * WIDTH];

const static int CLOUDIMAGE_SIZE = WIDTH * CLOUD_IMAGE_HEIGHT * sizeof(float);
const static int CLOUDIMAGE_PIXELS = WIDTH * CLOUD_IMAGE_HEIGHT;

float cloudImage1[CLOUD_IMAGE_HEIGHT * WIDTH];

CLightController::CLightController(OneWire* pDS, SoftPwm* pSoftPwm, Communicator* pCom)
{
    m_iNightWatchDogCounter = 0;

    m_bRemoteDataRequested = true; // on first start we request data from raspberry pi

    m_pCommunicator = pCom;

    m_iSunSetSunRiseTime = SUNSET_SUNRISE_TIME;

    m_iSunriseHour = 13;
    m_iSunriseMinute = 10;
    m_iSunsetHour = 23;
    m_iSunsetMinute = 0;

    m_iNightHour = 23;
    m_iNightMinute = 1;

    m_iCloudOctaves = 2;
    m_lCloudImageDuration = 0L;

    m_fCloudParamZoom = 0.3f;
    m_fCloudParamP = 0.5f;

    m_controllerMode = STANDALONE;

    m_iMoonLightPin = 8;

    m_fMoonLightValue = 0.0f;

    m_ScrollY = 0;

    m_lCloudImageDuration = UPDATE_CLOUD_IMAGE;


    m_lFadeFrames = 0L;

    m_bFirstImage = true;
    m_pSoftPwm = pSoftPwm;



//    m_Configuration.nGroupConfigs = 0;
//    m_Configuration.pGroupConfigs = NULL;
    m_bDrawBuffer = false;

    m_controllerState = OFF;

}

CLightController::~CLightController()
{
    //dtor
}
/*
bool CLightController::ReadConfiguration()
{

    //if(EEPROM.read(0) == 255) return false;

    int confSize = sizeof(m_Configuration) - 2;

    byte* p = (byte*)(void*)&m_Configuration;

    int pos = 1;

    for(pos = 1; pos < confSize + 1; pos++)
    {
        *p++ = EEPROM.read(pos);
    }

    char buf[32];
    sprintf(buf, "LC: # of led groups: %d", m_Configuration.nGroupConfigs);
    Serial.println(buf);

    m_Configuration.pGroupConfigs = new DimGroupConfig[m_Configuration.nGroupConfigs];

    p = (byte*)(void*)m_Configuration.pGroupConfigs;

    for(int i = pos; i < pos + (m_Configuration.nGroupConfigs * sizeof(DimGroupConfig)); i++)
    {
        *p++ = EEPROM.read(i);
    }

        sprintf(buf, "# configs: %d", m_Configuration.nGroupConfigs);
        Serial.println(buf);
        for(int i = 0; i < m_Configuration.nGroupConfigs; i++)
        {
            sprintf(buf, "pin of group %d: %d", i, m_Configuration.pGroupConfigs[i].iPinNumber);
            Serial.println(buf);

            sprintf(buf, "start of group %d: %d:%d", i, m_Configuration.pGroupConfigs[i].startTime.hour, m_Configuration.pGroupConfigs[i].startTime.minute);
            Serial.println(buf);

            sprintf(buf, "stop of group %d: %d:%d", i, m_Configuration.pGroupConfigs[i].stopTime.hour, m_Configuration.pGroupConfigs[i].stopTime.minute);
            Serial.println(buf);

            sprintf(buf, "dim duration of group %d: %d", i, m_Configuration.pGroupConfigs[i].iDuration);
            Serial.println(buf);
        }

    return true;
}

void CLightController::WriteConfiguration()
{
    byte* p = (byte*)(void*)&m_Configuration;

    EEPROM.write(0, 1);

    int confSize = sizeof(m_Configuration) - 2; // substract 2 bytes of the pGroupConfigs pointer.

    int pos = 1;
    for(pos = 1; pos < confSize + 1; pos++)
    {
        EEPROM.write(pos, *p++);
    }

    p = (byte*)m_Configuration.pGroupConfigs;

    for(int i = pos; i < pos + (m_Configuration.nGroupConfigs * sizeof(DimGroupConfig)); i++)
    {
        EEPROM.write(i, *p++);
    }
    Serial.println("LC: configuration successfully written.");
}
*/

void CLightController::Setup()
{


    for(int y = 0; y < HEIGHT; y++)
    {
        for(int x = 0; x < WIDTH; x++)
        {
            pinMode(arrDisplayPins[x][y], OUTPUT);
        }
    }

    ClearFrameBuffer(false);
    ClearFrameBuffer(true);
    DrawFrameBuffer();


    m_Timer.Setup();
#ifdef USE_SIM_TIMER
    m_Timer.SetTime(23, 1, 0);
    m_Timer.PrintTime();
#endif

// USE_SIM_TIMER
    randomSeed(analogRead(0));
/*
    if(!ReadConfiguration())
    {
        Serial.println("LC: System not configured.");
        m_Configuration.nGroupConfigs = 2;

        m_Configuration.pGroupConfigs = new DimGroupConfig[2];
        m_Configuration.pGroupConfigs[0].iPinNumber = 8;
        m_Configuration.pGroupConfigs[0].startTime.hour = 13;
        m_Configuration.pGroupConfigs[0].startTime.minute = 0;
        m_Configuration.pGroupConfigs[0].stopTime.hour =23;
        m_Configuration.pGroupConfigs[0].stopTime.minute = 0;
        m_Configuration.pGroupConfigs[0].iDuration = 1800;

        m_Configuration.pGroupConfigs[1].iPinNumber = 9;
        m_Configuration.pGroupConfigs[1].startTime.hour = 14;
        m_Configuration.pGroupConfigs[1].startTime.minute = 30;
        m_Configuration.pGroupConfigs[1].stopTime.hour = 22;
        m_Configuration.pGroupConfigs[1].stopTime.minute = 30;
        m_Configuration.pGroupConfigs[1].iDuration = 1800;

        WriteConfiguration();
    }
*/




    m_iCurrentDayOfMonth = m_Timer.GetDayOfMonth();

    Serial.print("day: ");
    Serial.println(m_iCurrentDayOfMonth);

    SetEffectMode(IMMEDIATE);

    m_LightingMode = NIGHT;
    m_TargetLightingMode = NIGHT;
    m_TargetLightingModeAfterSunrise = PARTLY_CLOUDY;

    SetLightingMode(NIGHT);

    //SetLightingMode(DEMO);

    UpdateCloudAnimationParams(DEFAULT_CLOUD_SPEED);

    m_lLastMillis = millis();
    m_lLastUpdateMillis = millis();
    m_lLastCloudImageMillis = 0L;
    m_controllerState = RUNNING;

    Serial.println("LC: set up.");
}

void CLightController::EnsureNightTime()
{
    bool nightTime = false;

    int hour = m_Timer.GetHour();
    int minute = m_Timer.GetMinute();

    if(m_iNightHour <= m_iSunriseHour) // night hour after midnight
    {
        //Serial.println("night hour after midnight");

        // check before or after midnight
        bool beforeMidnight = true;
        if(hour >= 0 && hour <= m_iSunriseHour) // current hour after midnight
        {
            //Serial.println("current hour after midnight");
            if(hour == m_iNightHour && minute > m_iNightMinute)
            {
                //Serial.println("NT #1");
                nightTime = true;
            }
            else if(hour > m_iNightHour && hour < m_iSunriseHour)
            {
                //Serial.println("NT #2");
                nightTime = true;
            }
            else if(hour == m_iSunriseHour && minute < m_iSunriseMinute)
            {
                //Serial.println("NT #3");
                nightTime = true;
            }

        }
       /* else
        {
            Serial.println("current hour before midnight");
        }*/
    }
    else // night time before midnight
    {
        //Serial.println("night time before midnight");

        if(hour >= m_iNightHour)
        {
            if(hour == m_iNightHour && minute > m_iNightMinute)
            {
                //Serial.println("NT #1.1");
                nightTime = true;
            }
            else if(hour > m_iNightHour)
            {
                //Serial.println("NT #1.2");
                nightTime = true;
            }
        }
    }

    if(nightTime)
    {
        bool forceNight;

        if(m_LightingMode != NIGHT)
        {
            m_iNightWatchDogCounter++;

            Serial.println("ALERT: NT.");

            m_LightingMode = NIGHT;
            m_TargetLightingMode = NIGHT;
            ClearFrameBuffer(true, 0.0f);
            ClearFrameBuffer(false, 0.0f);
            m_bDrawBuffer = true;
            DrawFrameBuffer();
            SetMoon(10);
        }
    }
}

void CLightController::SetSunset(byte hour, byte minute)
{
    char buf[32];
    sprintf(buf, "LC: sunset: %02d:%02d", hour, minute);
    Serial.println(buf);
    m_iSunsetHour = hour;
    m_iSunsetMinute = minute;
    CalculateNightTime();
}

void CLightController::SetSunrise(byte hour, byte minute)
{
    char buf[32];
    sprintf(buf, "LC: sunrise: %02d:%02d", hour, minute);
    Serial.println(buf);
    m_iSunriseHour = hour;
    m_iSunriseMinute = minute;
}

void CLightController::SetMoon(long fadeTime)
{
    for(int y = 0; y < HEIGHT; y++)
    {
        for(int x = 0; x < WIDTH; x++)
        {
            if(arrDisplayPins[x][y] == m_iMoonLightPin)
            {
                SetPixel(targetBuffer, x, y, m_fMoonLightValue);
                break;
            }
        }
    }
    m_lFadeFrames = (long)FRAMES_PER_SEC * (long)fadeTime;
    CalculateFading();
    SetEffectMode(FADE);
}

void CLightController::SetLightingMode(LightingMode mode)
{
    m_LightingMode = mode;

    Serial.print("LC: lm: "); // lighting mode

    switch(mode)
    {
    case NIGHT:
        {
            ClearFrameBuffer(true, 0.0f);
            SetMoon(10);
            Serial.println("NIGHT.");
        }
        break;
    case SUNSET:
        {
            ClearFrameBuffer(true, 0.0f);
            SetMoon(m_iSunSetSunRiseTime);
            Serial.print("SUNSET: ");
            Serial.println(m_lFadeFrames);
        }
        break;
    case CLEAR:
        {

            Serial.println("CLEAR SUN LIGHT");

            ClearFrameBuffer(true, 255.0f);

            m_lFadeFrames = (long)FRAMES_PER_SEC * (long)10;

            CalculateFading();

            SetEffectMode(FADE);
        }
        break;
    case SUNRISE:
        {

            ClearFrameBuffer(true, 255.0f);

            m_lFadeFrames = (long)FRAMES_PER_SEC * (long)m_iSunSetSunRiseTime;
            Serial.print("SUNRISE: ");
            Serial.println(m_lFadeFrames);

            CalculateFading();
            SetEffectMode(FADE);
        }
        break;
    case PARTLY_CLOUDY:
        {
            Serial.println("PARTLY CLOUDY");

            CreateCloudImages();

        }
        break;
    case MOSTLY_CLOUDS:
        {
            Serial.println("MOSTLY CLOUDS");

            CreateCloudImages();
        }
        break;
    case CLOUDS:
        {
            Serial.println("CLOUDS");

            CreateCloudImages();
        }
        break;
    case DEMO:
        {
            Serial.print("DEMO");
        }
        break;

    }


}
void CLightController::SetEffectMode(LightEffect effect, long lEffectTime)
{
    m_lEffectCounter = 0;
    m_currentEffect = effect;
    m_lLastMillis = millis();
    if(effect == FADE)
    {
        m_bDrawBuffer = false;
    }
}



void CLightController::Update()
{
    unsigned long curMillis = millis();

    if(m_controllerState == RUNNING)
    {
        if(m_currentEffect == FADE)
        {
            if(curMillis - m_lLastMillis >= INTERVAL)
            {
                m_lLastMillis = curMillis;
                m_lEffectCounter++;
                if(m_lEffectCounter > m_lMaxEffectSteps)
                {
                    SetEffectMode(IMMEDIATE);
                }
                else
                {
                    RenderBuffer();
                }
            }
        }
        else
        {
            if((m_LightingMode == CLOUDS || m_LightingMode == MOSTLY_CLOUDS || m_LightingMode == PARTLY_CLOUDY) && curMillis - m_lLastCloudImageMillis >= m_lCloudImageDuration)
            {
                UpdateClouds();
                m_lLastCloudImageMillis = millis();
            }

            if(curMillis - m_lLastUpdateMillis >= UPDATE_IMAGE)
            {

                bool modeSwitched = false;
                if(m_LightingMode != DEMO)
                {
                    if(m_LightingMode != m_TargetLightingMode)
                    {
                        Serial.print("Target lighting mode reached: ");
                        Serial.println(m_TargetLightingMode);
                        SetLightingMode(m_TargetLightingMode);

                        m_LightingMode = m_TargetLightingMode;

                        modeSwitched = true;

                    }

                    EnsureNightTime();


                }

                if(m_Timer.GetDayOfMonth() != m_iCurrentDayOfMonth)
                {
                    Serial.print("LC: next day: ");
                    Serial.println( m_Timer.GetDayOfMonth() );

                    m_bRemoteDataRequested = false;
                    m_iCurrentDayOfMonth = m_Timer.GetDayOfMonth();
                }

                switch(m_LightingMode)
                {
                case DEMO:
                    {
                        UpdateDemo();
                    }
                    break;
                case NIGHT:
                    {
                        bool startSunrise = false;

                        m_iSunSetSunRiseTime = SUNSET_SUNRISE_TIME;

                        if(!m_bRemoteDataRequested && m_Timer.GetHour() == m_iSunriseHour - 1)
                        {
                            Serial.println("requesting remote lighting data...");
                            m_pCommunicator->RequestRemoteConnection();
                            m_bRemoteDataRequested = true;
                        }
                        if(m_Timer.GetHour() == m_iSunriseHour && m_Timer.GetMinute() == m_iSunriseMinute)
                        {
                            startSunrise = true;
                        }
                        else if(m_Timer.GetHour() == m_iSunriseHour && m_Timer.GetMinute() > m_iSunriseMinute)
                        {
                            int minutesOver = m_Timer.GetMinute() - m_iSunriseMinute;

                            int sunriseMinutes = SUNSET_SUNRISE_TIME / 60;

                            if(minutesOver > sunriseMinutes)
                            {
                                 m_iSunSetSunRiseTime = 30; // 30 seconds
                            }
                            else
                            {
                                m_iSunSetSunRiseTime = (sunriseMinutes - minutesOver) * 60;
                            }

                            startSunrise = true;
                        }
                        else if(m_Timer.GetHour() > m_iSunriseHour && (m_Timer.GetHour() < m_iSunsetHour || (m_Timer.GetHour() <= 23 && m_iSunsetHour < m_iSunriseHour)))
                        {
                            Serial.println("LC: fast sunrise.");

                            m_iSunSetSunRiseTime = 30;
                            startSunrise = true;
                        }
                        else if(m_Timer.GetHour() == m_iSunsetHour && m_Timer.GetMinute() < m_iSunsetMinute)
                        {
                            Serial.println("LC: fast sunrise, shortly before sunset, or Timer problem.");
                            m_iSunSetSunRiseTime = 30;
                            startSunrise = true;
                        }
                        if(startSunrise)
                        {
                            m_Timer.PrintTime();

                            SetLightingMode(SUNRISE);
                            m_TargetLightingMode = m_TargetLightingModeAfterSunrise;
                        }

                    }
                    break;
                default:
                    {
                        if(m_Timer.GetHour() == m_iSunsetHour && m_Timer.GetMinute() >= m_iSunsetMinute)
                        {
                            m_iSunSetSunRiseTime = SUNSET_SUNRISE_TIME;
                            SetLightingMode(SUNSET);
                            m_TargetLightingMode = NIGHT;
                        }
                    }
                    break;

                }
                m_lLastUpdateMillis = millis();
                //m_bDrawBuffer = true;
            }
            if(m_bDrawBuffer)
            {
                memcpy(frameBuffer, targetBuffer, FRAMEBUFFER_SIZE);
            }
        }
        if(m_bDrawBuffer)
        {
            DrawFrameBuffer();
        }
    }

    m_Timer.Update();

    //m_LedModule.Update();
    //m_LedModule1.Update();


}

void CLightController::RenderBuffer()
{
    switch(m_currentEffect)
    {
    case FADE:
        {
            for(byte x = 0; x < WIDTH; x++)
            {
                for(byte y = 0; y < HEIGHT; y++)
                {
                    int yoff = y * WIDTH;

                    float col = *(frameBuffer + x + yoff);

                    float targetCol = *(targetBuffer + x + yoff);

                    if(targetCol < col)
                    {
                        col = col - m_fFadeSteps;
                        if(col <= 0) col = 0;
                        *(frameBuffer + x + yoff) = col;
                    }
                    else if(targetCol > col)
                    {
                        col = col + m_fFadeSteps;
                        if(col > targetCol) col = targetCol;
                        *(frameBuffer + x + yoff) = col;
                    }
                }
            }
        }
        break;
    }
    m_bDrawBuffer = true;
}


void CLightController::DrawFrameBuffer()
{
    for(byte x = 0; x < WIDTH; x++)
    {
        for(byte y = 0; y < HEIGHT; y++)
        {
            int yoff = y * WIDTH;

            uint16_t col = (uint16_t)*(frameBuffer + x + yoff);

            if(m_currentEffect == FADE)
            {
                col = dimTable[(byte)col];
            }

            byte pin = arrDisplayPins[x][y];
            if((byte)col != arrDisplayValues[x][y])
            {
               arrDisplayValues[x][y] = (byte)col;
                if(pin < 50)
                {
                    analogWrite(pin, col);
                }
                else
                {
                    m_pSoftPwm->SetBrightness(pin - 50, col);
                }

            }

        }
    }
    m_bDrawBuffer = false;
}

void CLightController::ClearFrameBuffer(bool clearTargetBuffer, float fValue)
{
    for(byte x = 0; x < WIDTH; x++)
    {
        for(byte y = 0; y < HEIGHT; y++)
        {
            int yoff = y * WIDTH;

            float* pBuffer = (!clearTargetBuffer) ? frameBuffer : targetBuffer;
            *(pBuffer + x + yoff) = fValue;
            if(pBuffer == frameBuffer)
            {

                arrDisplayValues[x][y] = (byte)fValue;
            }
        }
    }
    m_bDrawBuffer = true;
}

void CLightController::UpdateClouds()
{

    if(m_ScrollY >= CLOUD_IMAGE_HEIGHT - 1)
    {
        CreateCloudImages();
        m_ScrollY = 0;

    }

    int offSet = m_ScrollY * WIDTH * sizeof(float);

    int copyBytes = CLOUDIMAGE_SIZE - offSet;

    if(copyBytes > FRAMEBUFFER_SIZE) copyBytes = FRAMEBUFFER_SIZE;

    int off = m_ScrollY * WIDTH * sizeof(float);

    memcpy(targetBuffer, (cloudImage1 + (m_ScrollY * WIDTH)), copyBytes);

    int copiedRows = CLOUD_IMAGE_HEIGHT - (m_ScrollY + 1);
    // wieviele reihen sind noch verf��gbar im Wolkenbild
    if(copiedRows < HEIGHT)
    {
        int remainingRows = HEIGHT - copiedRows;

        float* start = targetBuffer + (copiedRows * WIDTH);

        memcpy(start, cloudImage1, remainingRows * WIDTH * sizeof(float));
    }

    m_ScrollY++;

    SetEffectMode(FADE);
/*
    Serial.println("scrolled cloud image:");
    for(int y = 0; y < HEIGHT; y++)
    {
        for(int x = 0; x < WIDTH; x++)
        {
            int yoff = y * WIDTH;

            Serial.print(*(targetBuffer + x + yoff));
            Serial.print("  ");
        }
        Serial.println();
    }*/




}

void CLightController::CreateCloudImages()
{
    int fullSunLevel = 0;

    //Serial.println("creating cloud images...");

    //persistence affects the degree to which the "finer" noise is seen
    //float persistence = 0.5;
    //octaves are the number of "layers" of noise that get computed
    //int octaves = 2;


    //Serial.print("max brightness: ");
    //Serial.println(m_iCloudImageBrightness);

    //Serial.print("octaves: ");
    //Serial.println(m_iCloudOctaves);

    //Serial.print("zoom: ");
    //Serial.println(m_fCloudParamZoom);

    //Serial.print("p: ");
    //Serial.println(m_fCloudParamP);

    //Serial.print("speed: ");
    //Serial.println(m_lCloudImageDuration);

//    float noise1 =  float(random(1, 99))/100.0f;
//    float noise2 = float(millis())/100.0f;

    SimplexNoise simplexNoise;

    unsigned long start = millis();
    float fCoarse = 1.02f;
    float fBrightness = 256.0f;

    switch(m_LightingMode)
    {
    case MOSTLY_CLOUDS:
        {
            fCoarse = 1.01f;
            fBrightness = 128.0f;
        }
        break;
    case CLOUDS:
        {
            fCoarse = 1.01f;
             fBrightness = 200.0f;
             fullSunLevel = 4;
        }
        break;
    case PARTLY_CLOUDY:
        {
            fCoarse = 1.05f;
            fullSunLevel = 40;
        }
        break;
    }

    //Serial.print("coarse: ");
    //Serial.println(fCoarse);

    //Serial.print("mode: ");
    //Serial.println(m_LightingMode);

    simplexNoise.CreateNoiseTexture((float*)cloudImage1, WIDTH, CLOUD_IMAGE_HEIGHT, m_fCloudParamP, m_fCloudParamZoom, fBrightness, fCoarse);
    unsigned long end = millis() - start;

    int fullSunCells = 0;
    int fullSunPercent = 0;

    bool abort = false;

    while(!abort && fullSunLevel > 0)
    {
        fullSunCells = 0;

        for(int y = 0; y < CLOUD_IMAGE_HEIGHT && !abort; y++)
        {
            int yoff = y * WIDTH;

            for(int x = 0; x < WIDTH; x++)
            {

                float col = (float)*(cloudImage1 + x + yoff);
                if(col < 254)
                {
                    col++;

                    if(col > 255) col = 255;

                    *(cloudImage1 + x + yoff) = col;
                }
                if(col >= 235)
                {
                    fullSunCells++;

                    fullSunPercent = fullSunCells * 100 / CLOUDIMAGE_PIXELS;
                    if(fullSunPercent >= fullSunLevel)
                    {
                        abort = true;
                        break;
                    }
                }

            }

        }




    }

    Serial.print("time for noise: ");
    Serial.println(end);
    Serial.print("full sun cells: ");
    Serial.println(fullSunCells);
    Serial.print("full sun percent: ");
    Serial.println(fullSunPercent);

    Serial.println("new cloud image:");
    for(int y = 0; y < CLOUD_IMAGE_HEIGHT; y++)
    {
        int yoff = y * WIDTH;
        for(int x = 0; x < WIDTH; x++)
        {

            Serial.print(*(cloudImage1+x+yoff));
            Serial.print("  ");
        }
        Serial.println();
    }


/* HERE IS PERLIN NOISE
    for(byte x = 0; x < WIDTH; x++)
    {
        for(byte y = 0; y < CLOUD_IMAGE_HEIGHT; y++)
        {

            double getnoise =0;

            for(int a=0;a<m_iCloudOctaves-1;a++)//This loops trough the octaves.
            {
                double frequency = pow(2,a);//This increases the frequency with every loop of the octave.
                double amplitude = pow(m_fCloudParamP,a);//This decreases the amplitude with every loop of the octave.
                noise1 += x;
                noise2 += y;

                getnoise += m_PerlinNoise.noise(((double)noise1)*frequency/m_fCloudParamZoom,((double)noise2)/m_fCloudParamZoom*frequency)*amplitude;//This uses our perlin noise functions. It
                s all our zoom and frequency and amplitude
                *


            }


            int color= (int)((getnoise*(float)halfTheBrightness)+(float)halfTheBrightness);//Convert to 0-256 values.

            if(color>255)
                color=255;
            if(color<0)
                color=0;

            int yoff = y * WIDTH;

            *(cloudImage1 + x + yoff) = color;

        }
    }
*/

    m_lLastCloudImageMillis = millis();
}


void CLightController::UpdateCloudAnimationParams(long speed)
{
     m_lCloudImageDuration = speed;

     if(m_lCloudImageDuration < 1000) m_lCloudImageDuration = 1000;
    if(m_lCloudImageDuration > 60000) m_lCloudImageDuration = 60000;

    m_lFadeFrames = (long)FRAMES_PER_SEC * (long)(m_lCloudImageDuration / 1000);

    CalculateFading();
    m_lEffectCounter = 0;
    /*
    Serial.print("LC: cloud duration:");
    Serial.println(m_lCloudImageDuration);


    Serial.print("LC: fade frames:");
    Serial.println(m_lFadeFrames);

    Serial.println(m_fFadeSteps);

    Serial.print("LC: fade iterations:");
    Serial.println(m_fFadeSteps);

    Serial.print("LC: effect iterations:");
    Serial.println(m_lMaxEffectSteps);
*/
}

void CLightController::UpdateDemo()
{
    if(m_bFirstImage)
    {
        m_PixelY = 0;
        m_PixelX = 0;

        ClearFrameBuffer(true, 0.0f);

        SetPixel(targetBuffer, 2, 0, 150.0f);
        m_lFadeFrames = 50;
        CalculateFading();


        SetEffectMode(FADE);
        m_bFirstImage = false;

    }
    else
    {
        ClearFrameBuffer(true, 0.0f);
        SetPixel(targetBuffer, m_PixelX, m_PixelY, 255.0f);
        m_PixelX++;
        if(m_PixelX == WIDTH)
        {
            m_PixelX = 0;
            m_PixelY++;
            if(m_PixelY == HEIGHT)
            {
                m_PixelY = 0;
                m_PixelX = 0;
            }
        }

        SetEffectMode(FADE);
    }
}

long CLightController::Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer)
{
    Serial.println("LC: unknown method.");
}

float CLightController::GetValue(int value)
{
    if(value == LIGHTINGMODE)
    {
        return (byte)m_LightingMode;
    }
    else if(value == MOONLIGHTVAL)
    {
        return m_fMoonLightValue;
    }
    else if(value == MOONLIGHTPIN)
    {
        return m_iMoonLightPin;
    }
    else if(value == VALUE_WATCHDOG_COUNTER)
    {
        return m_iNightWatchDogCounter;
    }
    return -1.0f;
}

void CLightController::SetValue(int value, float fValue)
{
    switch(value)
    {
    case LIGHTINGMODE:
        {
            if(m_controllerMode == STANDALONE)
            {
                if(m_LightingMode == NIGHT || m_LightingMode == SUNRISE)
                {
                    SetValue(TARGETLIGHTINGMODEAFTERSUNRISE, fValue);
                    return;
                }

            }
            SetLightingMode((LightingMode)fValue);
            m_TargetLightingMode = (LightingMode)fValue;

        }
        break;
    case TARGETLIGHTINGMODEAFTERSUNRISE:
        {
            m_TargetLightingModeAfterSunrise = (LightingMode)fValue;
            Serial.print("TLM after sunrise set: ");
            Serial.println(m_TargetLightingModeAfterSunrise);

            if(m_LightingMode == SUNRISE)
            {
                m_TargetLightingMode = m_TargetLightingModeAfterSunrise;
            }
        }
        break;
    case MOONLIGHTVAL:
        {
            if(fValue != m_fMoonLightValue)
            {
                 m_fMoonLightValue = fValue;
                 if(m_LightingMode == NIGHT)
                 {
                     Serial.println("LC: update moon image.");
                     SetMoon(10);
                 }
            }
            Serial.print("LC: moon: ");
            Serial.println(m_fMoonLightValue);

        }
        break;
    case MOONLIGHTPIN:
        {
             m_iMoonLightPin = (byte)fValue;
            Serial.print("LC: moon pin: ");
            Serial.println(m_iMoonLightPin);

        }
        break;
    case CLOUD_SPEED:
        {
            if(m_LightingMode == SUNRISE || m_LightingMode == SUNSET || m_LightingMode == NIGHT)
            {
                Serial.println("LC: wind ignored.");
                return;
            }
            Serial.print("LC: cloud speed: " );
            long val = (long)fValue;
            Serial.println(val);
            UpdateCloudAnimationParams(val);
        }
        break;
    case CLOUD_IMAGE_PARAM_ZOOM:
        {
            if(fValue > 0 && fValue < 1)
            {
                Serial.print("LC: cloud zoom: " );
                Serial.println(fValue);
                m_fCloudParamZoom = fValue;
            }
            else
            {
                Serial.println("LC: vnr"); // value not in range!
            }
        }
        break;
    case CLOUD_IMAGE_PARAM_P:
        {
            if(fValue > 0 && fValue < 1)
            {
                Serial.print("LC: setting cloud P to: " );
                Serial.println(fValue);
                m_fCloudParamP = fValue;
            }
            else
            {
                Serial.println("LC: vnr");
            }
        }
        break;
        case CLOUD_IMAGE_OCTAVES:
        {
            m_iCloudOctaves = (byte)fValue;
            Serial.print("LC: cloud octaves: ");
            Serial.println(m_iCloudOctaves);
        }
        break;
    case CREATE_NEW_CLOUD_IMAGE:
        {
            CreateCloudImages();
        }
        break;
    case MASTER_CONNECT:
        {
            byte connect = (byte)fValue;
            if(connect)
            {
                Serial.println("master connected.");
                m_TargetLightingMode = NIGHT;
                SetLightingMode(NIGHT);
                m_controllerMode = REMOTE_CONTROLLED;
            }
            else
            {
                Serial.println("master disconnected.");
                m_controllerMode = STANDALONE;
            }
        }
        break;
    case SUNRISE_HOUR:
        {
            m_iSunriseHour = (byte)fValue;
            Serial.print("LC: Sunrise hour: ");
            Serial.println(m_iSunriseHour);
        }
        break;
    case SUNRISE_MINUTE:
        {
            m_iSunriseMinute = (byte)fValue;
            Serial.print("LC: Sunrise minute: ");
            Serial.println(m_iSunriseMinute);
        }
        break;
    case SUNSET_HOUR:
        {
            m_iSunsetHour = (byte)fValue;
            if(m_iSunsetHour == 24) m_iSunsetHour == 0;
            CalculateNightTime();
            Serial.print("LC: Sunset hour: ");
            Serial.println(m_iSunsetHour);
        }
        break;
    case SUNSET_MINUTE:
        {
            m_iSunsetMinute = (byte)fValue;
            CalculateNightTime();
            Serial.print("LC: Sunset minute: ");
            Serial.println(m_iSunsetMinute);
        }
        break;
    default:
        {
            Serial.print("LC: sv unknown: ");
            Serial.println(fValue);
        }
        break;
    }

}

void CLightController::CalculateNightTime()
{

    m_iNightHour = m_iSunsetHour;
    m_iNightMinute = m_iSunsetMinute + 1;

//    Serial.print("before night hour: ");
//    Serial.println(m_iNightHour);

//    Serial.print("before night minute: ");
//    Serial.println(m_iNightMinute);
    if(m_iNightMinute == 60)
    {
        m_iNightMinute = 0;

        m_iNightHour++;

        if(m_iNightHour == 24)
        {
            m_iNightHour = 0;
        }
    }

    int sunsetMinutes = SUNSET_SUNRISE_TIME / 60;


    int hours = 0;
    int minutes = sunsetMinutes;

    if(sunsetMinutes >= 60)
    {
        hours = sunsetMinutes % 60;
        minutes = sunsetMinutes - (hours * 60);
    }

 //   Serial.print("hours: ");
 //   Serial.println(hours);

 //   Serial.print("minutes: ");
 //   Serial.println(minutes);

    m_iNightHour += hours;
    if(m_iNightHour >= 24)
    {
        m_iNightHour = m_iNightHour - 24;
    }

    m_iNightMinute += minutes;
    if(m_iNightMinute >= 60)
    {
        m_iNightMinute = m_iNightMinute - 60;

        m_iNightHour++;

        if(m_iNightHour == 24)
        {
            m_iNightHour = 0;
        }
    }

    Serial.print("night time: ");
    Serial.print(m_iNightHour);
    Serial.print(" : ");
    Serial.println(m_iNightMinute);
}

void CLightController::SetPixel(float* pBuffer, byte x, byte y, float val)
{
    *(pBuffer + (y * WIDTH) + x) = val;
}
//using the algorithm from http://freespace.virgin.net/hugo.elias/models/m_perlin.html
// thanks to hugo elias
