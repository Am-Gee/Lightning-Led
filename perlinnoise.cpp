//#include <math.h>
#include <arduino.h>
#include "perlinnoise.h"

SimplexNoise::SimplexNoise()
{
    A[0] = A[1] = A[2] = 0;
    for(int t = 0; t < 8; t++)
    {
       T[t] = random(0, 255);

    }/*
    T[0]= 0x15;
    T[1] = 0x38;
    T[2] = 0x32;
    T[3] =  0x2c;
    T[4] =  0x0d;
    T[5] =  0x13;
    T[6] =  0x07;
    T[7] =  0x2a;*/
}

float SimplexNoise::noise(float x, float y, float z)
{
    // Skew input space to relative coordinate in simplex cell
    s = (x + y + z) * SimplexNoise::onethird;
    i = fastfloor(x+s);
    j = fastfloor(y+s);
    k = fastfloor(z+s);

    // Unskew cell origin back to (x, y , z) space
    s = (i + j + k) * SimplexNoise::onesixth;
    u = x - i + s;
    v = y - j + s;
    w = z - k + s;;

    A[0] = A[1] = A[2] = 0;

    // For 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we're in
    int hi = u >= w ? u >= v ? 0 : 1 : v >= w ? 1 : 2;
    int lo = u < w ? u < v ? 0 : 1 : v < w ? 1 : 2;

    return _k(hi) + _k(3 - hi - lo) + _k(lo) + _k(0);
}

void SimplexNoise::CreateNoiseTexture(float* pBuffer, int width, int height, float streak, float fineFactor , int maxBrightness, float coarse)
{
    int halfBright = maxBrightness / 2;

    float z = 0.0f;
    byte red = 0;
   for (int y = 0; y < height; y++)
   {
       int yoff = y * width;

        for (int x = 0; x < width; x++)
        {
            // Octave 1: Grain
            float fnoise = (noise(x * 5, y * 5, z) + 1) * 20;
            fnoise = (fnoise - (int) fnoise) * 1.0f;

            // Octave 2: Fine noise
            fnoise += noise(x * 200, y * 200, z) * fineFactor;

            // Octave 3: Streak
            fnoise += noise(x, y * 100, z) * streak;

            // Adjust range to [0, 1]
            fnoise = (fnoise + 1) / 2;

            //red = (int)((fnoise*(float)halfBright)+(float)halfBright);
            // Convert noise to colour
            fnoise = ((fnoise*(float)halfBright)+(float)halfBright);
            red = (int) (fnoise * coarse);


            // Bounds check colour. The persistence of octaves doesn't sum to one,
            // this will catch any values the fly outside the valid range [0, 255].
            if (red > maxBrightness)
            {
                red = maxBrightness;
            }
            else if (red < 0) red = 0;

            *(pBuffer + x + yoff) = red;
        }
    }
}


/*
PerlinNoise::PerlinNoise()
{
    //ctor
}

PerlinNoise::~PerlinNoise()
{
    //dtor
}


float PerlinNoise::SmoothNoise2(float x, float y)
{
  float corners, sides, center;
  corners = ( Noise2(x-1, y-1)+Noise2(x+1, y-1)+Noise2(x-1, y+1)+Noise2(x+1, y+1) ) / 16;
  sides   = ( Noise2(x-1, y)  +Noise2(x+1, y)  +Noise2(x, y-1)  +Noise2(x, y+1) ) /  8;
  center  =  Noise2(x, y) / 4;
  return (corners + sides + center);
}

float PerlinNoise::InterpolatedNoise2(float x, float y)
{
  float v1,v2,v3,v4,i1,i2,fractionX,fractionY;
  long longX,longY;

  longX = long(x);
  fractionX = x - longX;

  longY = long(y);
  fractionY = y - longY;

  v1 = SmoothNoise2(longX, longY);
  v2 = SmoothNoise2(longX + 1, longY);
  v3 = SmoothNoise2(longX, longY + 1);
  v4 = SmoothNoise2(longX + 1, longY + 1);

  i1 = Interpolate(v1 , v2 , fractionX);
  i2 = Interpolate(v3 , v4 , fractionX);

  return(Interpolate(i1 , i2 , fractionY));
}

float PerlinNoise::Interpolate(float a, float b, float x)
{
  //cosine interpolations
  return(CosineInterpolate(a, b, x));
}

float PerlinNoise::LinearInterpolate(float a, float b, float x)
{
  return(a*(1-x) + b*x);
}

float PerlinNoise::CosineInterpolate(float a, float b, float x)
{
  float ft = x * 3.1415927;
  float f = (1 - cos(ft)) * .5;

  return(a*(1-f) + b*f);
}

float PerlinNoise::Noise2(float x, float y)
{
  long noise;
  noise = x + y * 57;
  noise = (noise << 13) ^ noise;
  return ( 1.0 - ( long(noise * (noise * noise * 15731L + 789221L) + 1376312589L) & 0x7fffffff) / 1073741824.0);
}

float PerlinNoise::PerlinNoise2(float x, float y, float persistance, int octaves)
{
  float frequency, amplitude;
  float total = 0.0;

  for (int i = 0; i <= octaves - 1; i++)
  {
    frequency = pow(2,i);
    amplitude = pow(persistance,i);

    total = total + InterpolatedNoise2(x * frequency, y * frequency) * amplitude;
  }

  return(total);
}

double PerlinNoise::noise(double x,double y)
{
    double floorx = (double)((int)x);//This is kinda a cheap way to floor a double integer.
    double floory = (double)((int)y);
    double s,t,u,v;//Integer declaration
    s = findnoise2(floorx,floory);
    t = findnoise2(floorx+1,floory);
    u = findnoise2(floorx,floory+1);//Get the surrounding pixels to calculate the transition.
    v = findnoise2(floorx+1,floory+1);
    double int1 = interpolate(s,t,x-floorx);//Interpolate between the values.
    double int2 = interpolate(u,v,x-floorx);//Here we use x-floorx, to get 1st dimension. Don't mind the x-floorx thingie, it's part of the cosine formula.
    return interpolate(int1,int2,y-floory);//Here we use y-floory, to get the 2nd dimension.
}
*/

