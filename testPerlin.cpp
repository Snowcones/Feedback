//
//  perlinNoiseGenerator.m
//  proceduralTerrain
//
//  Created by William Henning on 10/26/14.
//  Copyright (c) 2014 William Henning. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "testPerlin.h"
#include "string.h"
#include <math.h>


void genNoiseFloat(int width, int height, int initialHorDiv, int initialVertDiv, int octaves, float persist, float* store, float* octaveNoise, int interp)
{
    srand(0);
    memset(store, 0, sizeof(float)*width*height);
    //float* octaveNoise=(float*)malloc(sizeof(float)*width*height);
    for (int octave=0; octave<octaves; octave++)
    {
        int horDiv=powf(2, octave)*initialHorDiv;
        int vertDiv=powf(2, octave)*initialVertDiv;
        float strength=powf(persist, octave);
        genOctaveFloat(horDiv, vertDiv, octaveNoise, width, height, interp);
        addArrToArrFloat(octaveNoise, store, strength, width*height);
    }
}

void genNoiseShort(int width, int height, int octaves, float persist, short* store, short* octaveNoise, int interp)
{
    srand(0);
    memset(store, 0, sizeof(float)*width*height);
    //float* octaveNoise=(float*)malloc(sizeof(float)*width*height);
    for (int octave=0; octave<octaves; octave++)
    {
        int divs=powf(2, octave);
        float strength=powf(persist, octave);
        genOctaveShort(divs, divs, octaveNoise, width, height, interp);
        addArrToArrShort((int*)octaveNoise, (int*)store, width*height);
    }
}

void genOctaveFloat(int totalHorDiv, int totalVertDiv, float* store, int width, int height, int interp)
{
    int horizontalSubWidth=width/totalHorDiv;
    int verticalSubWidth=height/totalVertDiv;
    float *angles=(float*)malloc(((totalHorDiv+1)*(totalVertDiv+1))*sizeof(float));
    for (int x=0; x<totalHorDiv; x++)
    {
        for (int y=0; y<totalVertDiv; y++)
        {
            angles[(totalHorDiv+1)*y+x]=2*M_PI*(float)rand()/INT32_MAX;
        }
    }
    
    for (int xDiv=0; xDiv<totalHorDiv; xDiv++)
    {
        for (int yDiv=0; yDiv<totalVertDiv; yDiv++)
        {
            float theta[4];
            float cornerVectors[8];
            
            theta[0]=angles[xDiv+yDiv*(totalHorDiv+1)];
            theta[1]=angles[xDiv+1+yDiv*(totalHorDiv+1)];
            theta[2]=angles[xDiv+1+(yDiv+1)*(totalHorDiv+1)];
            theta[3]=angles[xDiv+(yDiv+1)*(totalHorDiv+1)];
            for (int i=0; i<4; i++)
            {
                cornerVectors[2*i]=cosf(theta[i]);
                cornerVectors[2*i+1]=sinf(theta[i]);
                //                cornerVectors[2*i]=fastCos(theta[i]);
                //                cornerVectors[2*i+1]=fastSin(theta[i]);
            }
            for (int pixelX=0; pixelX<horizontalSubWidth; pixelX++)
            {
                float scaledX=(float)pixelX/horizontalSubWidth;
                for (int pixelY=0; pixelY<verticalSubWidth; pixelY++)
                {
                    float scaledY=(float)pixelY/verticalSubWidth;
                    float vectorsToCorner[8];

                    vectorsToCorner[0]=-scaledX;
                    vectorsToCorner[1]=-scaledY;
                    vectorsToCorner[2]=1-scaledX;
                    vectorsToCorner[3]=-scaledY;
                    vectorsToCorner[4]=1-scaledX;
                    vectorsToCorner[5]=1-scaledY;
                    vectorsToCorner[6]=-scaledX;
                    vectorsToCorner[7]=1-scaledY;
                    
                    float dotProducts[4];
                    for (int i=0; i<4; i++)
                    {
                        //dotProducts[i]=[self dotProductX1:cornerVectors[2*i] y1:cornerVectors[2*i+1] x2:vectorsToCorner[2*i] y2:vectorsToCorner[2*i+1]]/(2+sqrtf(2)/2)*strength;
                        dotProducts[i]=dotProd(cornerVectors[2*i], cornerVectors[2*i+1], vectorsToCorner[2*i], vectorsToCorner[2*i+1]);
                    }
                    float xWeight=0;
                    float yWeight=0;
                    if (interp==0)
                    {
                        xWeight=scaledX;
                        yWeight=scaledY;
                    }
                    else if (interp==1)
                    {
                        xWeight=(3-2*scaledX)*(scaledX*scaledX);
                        yWeight=(3-2*scaledY)*(scaledY*scaledY);
                    }
                    else if (interp==2)
                    {
                        xWeight=(10-scaledX*(15-6*scaledX))*(scaledX*scaledX*scaledX);
                        yWeight=(10-scaledY*(15-6*scaledY))*(scaledY*scaledY*scaledY);
                        
                    }
                    float xAvrLow=(dotProducts[1]-dotProducts[0])*xWeight+dotProducts[0];
                    float xAvrHigh=(dotProducts[2]-dotProducts[3])*xWeight+dotProducts[3];
                    float totalAvr=(xAvrHigh-xAvrLow)*yWeight+xAvrLow;
                    int horizontalPos=pixelX+xDiv*horizontalSubWidth;
                    int verticalPos=pixelY+yDiv*verticalSubWidth;
                    store[horizontalPos+verticalPos*width]=totalAvr;
                }
            }
        }
    }
    free(angles);
}

void genOctaveShort(int totalHorDiv, int totalVertDiv, short* store, int width, int height, int interp)
{
    int horizontalSubWidth=width/totalHorDiv;
    int verticalSubWidth=height/totalVertDiv;
    float *angles=(float*)malloc(((totalHorDiv+1)*(totalVertDiv+1))*sizeof(float));
    for (int x=0; x<totalHorDiv+1; x++)
    {
        for (int y=0; y<totalVertDiv+1; y++)
        {
            angles[(totalHorDiv+1)*y+x]=2*M_PI*(float)rand()/INT32_MAX;
        }
    }
    
    for (int xDiv=0; xDiv<totalHorDiv; xDiv++)
    {
        for (int yDiv=0; yDiv<totalVertDiv; yDiv++)
        {
            float theta[4];
            float cornerVectors[8];
            
            theta[0]=angles[xDiv+yDiv*(totalHorDiv+1)];
            theta[1]=angles[xDiv+1+yDiv*(totalHorDiv+1)];
            theta[2]=angles[xDiv+1+(yDiv+1)*(totalHorDiv+1)];
            theta[3]=angles[xDiv+(yDiv+1)*(totalHorDiv+1)];
            for (int i=0; i<4; i++)
            {
                cornerVectors[2*i]=cosf(theta[i]);
                cornerVectors[2*i+1]=sinf(theta[i]);
                //                cornerVectors[2*i]=fastCos(theta[i]);
                //                cornerVectors[2*i+1]=fastSin(theta[i]);
            }
            for (int pixelX=0; pixelX<horizontalSubWidth; pixelX++)
            {
                float scaledX=(float)pixelX/horizontalSubWidth;
                for (int pixelY=0; pixelY<verticalSubWidth; pixelY++)
                {
                    float scaledY=(float)pixelY/verticalSubWidth;
                    float vectorsToCorner[8];
                    
                    vectorsToCorner[0]=-scaledX;
                    vectorsToCorner[1]=-scaledY;
                    vectorsToCorner[2]=1-scaledX;
                    vectorsToCorner[3]=-scaledY;
                    vectorsToCorner[4]=1-scaledX;
                    vectorsToCorner[5]=1-scaledY;
                    vectorsToCorner[6]=-scaledX;
                    vectorsToCorner[7]=1-scaledY;
                    
                    float dotProducts[4];
                    for (int i=0; i<4; i++)
                    {
                        //dotProducts[i]=[self dotProductX1:cornerVectors[2*i] y1:cornerVectors[2*i+1] x2:vectorsToCorner[2*i] y2:vectorsToCorner[2*i+1]]/(2+sqrtf(2)/2)*strength;
                        dotProducts[i]=dotProd(cornerVectors[2*i], cornerVectors[2*i+1], vectorsToCorner[2*i], vectorsToCorner[2*i+1]);
                    }
                    float xWeight=0;
                    float yWeight=0;
                    if (interp==0)
                    {
                        xWeight=scaledX;
                        yWeight=scaledY;
                    }
                    else if (interp==1)
                    {
                        xWeight=(3-2*scaledX)*(scaledX*scaledX);
                        yWeight=(3-2*scaledY)*(scaledY*scaledY);
                    }
                    else if (interp==2)
                    {
                        xWeight=(10-scaledX*(15-6*scaledX))*(scaledX*scaledX*scaledX);
                        yWeight=(10-scaledY*(15-6*scaledY))*(scaledY*scaledY*scaledY);
                        
                    }
                    float xAvrLow=(dotProducts[1]-dotProducts[0])*xWeight+dotProducts[0];
                    float xAvrHigh=(dotProducts[2]-dotProducts[3])*xWeight+dotProducts[3];
                    float totalAvr=(xAvrHigh-xAvrLow)*yWeight+xAvrLow;
                    int horizontalPos=pixelX+xDiv*horizontalSubWidth;
                    int verticalPos=pixelY+yDiv*verticalSubWidth;
                    store[horizontalPos+verticalPos*width]=totalAvr;
                }
            }
        }
    }
    free(angles);
}

void addArrToArrFloat(float* arr1, float* arr2, float strength, int len)
{
    for (int i=0; i<len; i++)
    {
        arr2[i]+=strength*arr1[i];
    }
}

void addArrToArrShort(int* arr1, int* arr2, int len)
{
    for (int i=0; i<len/4; i++)
    {
        arr2[i]+=arr1[i];
    }
}

float dotProd(float x1, float y1, float x2, float y2)
{
    return x1*x2+y1*y2;
}