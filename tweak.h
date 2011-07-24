/*
    Copyright (c) 2011, spaceape [[ spaceape99@gmail.com ]]
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the organization nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TWEAK_H
#define TWEAK_H
#include <cstdlib>
#include <ctime>

#define TIME_RECYCLE 2.0f

#define CELL_MAX_LIFETIME 5.0f
#define CELL_EXPLODE_LIFETIME rndfloat(0.5f, 0.25f, 0.0f)
#define CELL_G_LIFETIME rndfloat(3.8f, 0.2f, 0.0f)
#define CELL_G_EXPLODE 2.0f
#define CELL_X_EVO rndint(3, 0, -1)
#define CELL_Y_EVO 0

inline void   seed()
{
       srand(time(0));
}

inline int    rndint(int range, int offset = 0, int bias = 0)
{
       return offset + bias + rand() % range;
}

inline float  rndfloat(float range = 2.0f, float offset = 0.0f, float bias = -1.0f)
{
       float  r = (range * (float)rand()) / (float)(RAND_MAX - 1);
       return offset + bias + r;
}
#endif
