#ifndef PERLINNOISE_H
#define PERLINNOISE_H

class SimplexNoise
{
public:
    SimplexNoise();

    void CreateNoiseTexture(float* buffer, int width, int height, float fineFactor, float streak, int maxBrightness, float coarse);

private:
    int i, j, k;

    float u, v, w, s;

    int A[3];
    int T[8];

public:
    static const float onethird = 0.333333333f;
    static const float onesixth = 0.166666667f;


public:
    float noise(float x, float y, float z);


private:
    inline int fastfloor(float n)
    {
        return n > 0 ? (int) n : (int) n - 1;
    }

    inline float _k(int a)
    {
        s = (A[0] + A[1] + A[2]) * SimplexNoise::onesixth;
        float x = u - A[0] + s;
        float y = v - A[1] + s;
        float z = w - A[2] + s;
        float t = 0.6f - x * x - y * y - z * z;
        int h = shuffle(i + A[0], j + A[1], k + A[2]);
        A[a]++;
        if (t < 0) return 0;
        int b5 = h >> 5 & 1;
        int b4 = h >> 4 & 1;
        int b3 = h >> 3 & 1;
        int b2 = h >> 2 & 1;
        int b = h & 3;
        float p = b == 1 ? x : b == 2 ? y : z;
        float q = b == 1 ? y : b == 2 ? z : x;
        float r = b == 1 ? z : b == 2 ? x : y;
        p = b5 == b3 ? -p : p;
        q = b5 == b4 ? -q: q;
        r = b5 != (b4^b3) ? -r : r;
        t *= t;
        return 8 * t * t * (p + (b == 0 ? q + r : b2 == 0 ? q : r));
    }

    inline int shuffle(int i, int j, int k)
    {
        return b(i, j, k, 0) + b(j, k, i, 1) + b(k, i, j, 2) + b(i, j, k, 3) +
               b(j, k, i, 4) + b(k, i, j, 5) + b(i, j, k, 6) + b(j, k, i, 7);
    }

    inline int b(int i, int j, int k, int B)
    {
        return T[b(i, B) << 2 | b(j, B) << 1 | b(k, B)];
    }

    inline int b(int N, int B)
    {
        return N >> B & 1;
    }

};
/*
class PerlinNoise
{
    public:
        PerlinNoise();
        virtual ~PerlinNoise();

        float Noise2(float x, float y);
        float SmoothNoise2(float x, float y);
        float InterpolatedNoise2(float x, float y);
        float Interpolate(float a, float b, float x);
        float LinearInterpolate(float a, float b, float x);
        float CosineInterpolate(float a, float b, float x);
        float PerlinNoise2(float x, float y, float persistance, int octaves);

        double noise(double x,double y);

        inline double findnoise2(double x,double y)
        {
            long n=(long)x+(long)y*57;
            n=(n<<13)^n;
            long nn=(n*(n*n*60493+19990303)+1376312589)&0x7fffffff;
            return 1.0-((double)nn/1073741824.0);
        }

        inline double interpolate(double a,double b,double x)
        {
            double ft=x * 3.1415927;
            double f=(1.0-cos(ft))* 0.5;
            return a*(1.0-f)+b*f;
        }

    protected:
    private:
};
*/
#endif // PERLINNOISE_H
