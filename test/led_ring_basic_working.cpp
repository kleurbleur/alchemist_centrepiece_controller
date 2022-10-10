
#include <Arduino.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>


// SETTINGS

const uint16_t loop_time = 1;         // the time it costs to loop over a ring
const uint16_t TailLength = 80;       // length of the tail, must be shorter than PixelCount
const uint16_t PixelCount = 720;      // make sure to set this to the number of pixels in your strip

const float MaxLightness = 0.2f;       // max lightness at the head of the tail (0.5f is full bright)
const float saturation = 0.05f;         // saturation scales from 0.0f to 1.0f with 0.0f no saturation
const float hue = 0.16667f;            // hue scales from 0.0f to 1.0f


const uint16_t PixelPin = 3;            // make sure to set this to the correct pin, ignored for Esp8266
const uint16_t AnimCount = 1;           // we only need one
const uint16_t timeScale = NEO_MILLISECONDS; // make sure it's set at milliseconds, although it is the default



// for any fade animations, best to correct gamma
NeoGamma<NeoGammaTableMethod> colorGamma; 
// four element pixels ----> G R B W
NeoPixelBus<NeoGrbwFeature, NeoSk6812Method> strip(PixelCount, PixelPin);
// NeoPixel animation management object
NeoPixelAnimator animations(AnimCount, timeScale ); 


void LoopAnimUpdate(const AnimationParam& param)
{
    // wait for this animation to complete,
    // we are using it as a timer of sorts
    if (param.state == AnimationState_Completed)
    {
        // done, time to restart this position tracking animation/timer
        animations.RestartAnimation(param.index);

        // rotate the complete strip one pixel to the right on every update
        strip.RotateRight(8);
    }
}

void DrawTailPixels()
{
    // using Hsl as it makes it easy to pick from similiar saturated colors
    // float hue = 1.0f;
    for (uint16_t index = 0; index < strip.PixelCount() && index <= TailLength; index++)
    {
        float lightness = index * MaxLightness / TailLength;
        RgbwColor color = HslColor(hue, saturation, lightness);

        strip.SetPixelColor(index, colorGamma.Correct(color));
    }
}

void setup()
{
    // Serial.begin(9600);
    strip.Begin();
    strip.Show();

    // Draw the tail that will be rotated through all the rest of the pixels
    DrawTailPixels();

    // we use the index 0 animation to time how often we rotate all the pixels
    animations.StartAnimation(0, loop_time, LoopAnimUpdate); 
}


void loop()
{

    // this is all that is needed to keep it running
    // and avoiding using delay() is always a good thing for
    // any timing related routines
    animations.UpdateAnimations();
    strip.Show();
}