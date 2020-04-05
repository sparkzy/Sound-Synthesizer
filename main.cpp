#include <iostream>
using namespace std;

#include "olcNoiseMaker.h"

atomic<double> dFrequencyOutput = 0.0;

/**
 * olcMakeNoise uses doubles
 * sound hardware normally does everything using integers
 *
 * dTime = time that has passed since the start of the program
 */
double makeNoise(double dTime) {
    // sine wave
    // 440.0Hz = A4 on a piano
    // 2pi to turn Hz into angular velocity
    // return 0.3 * sin(220.0 * 2 * 3.1459 * dTime);

    // single note
    // double dOutput = 1.0 * sin(dFrequencyOutput * 2 * 3.1459 * dTime);
    // square wave
    /*
    if (dOutput > 0.0)
        return 0.2;
    else
        return -0.2;
    */
    // sine wave
    // return dOutput * 0.5;

    // chord
    double dOutput = 1.0 * (sin(dFrequencyOutput * 2 * 3.1459 * dTime) + sin((dFrequencyOutput + 20.0) * 2.0 * 3.1459 * dTime));
    return dOutput * 0.4;
}

int main() {
    wcout << "Synthesizer" << endl;

    // Get all sound hardware
    vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

    //Display findings
    for (auto d : devices) wcout << "Found Output Device:" << d << endl;

    /**
     * Create sound machine!!
     * short = 16 bit (2 bytes) to capture the accuracy of the amplitude
     * 44100Hz, ned 2x the highest sample rate we want to record (human range ~20Hz - 20000Hz)
     * 1 chanel (non stereo/1 speaker)
     * 8, 512 used for latency management (reduce delay from pressing key and hearing a sound)
     */
    olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

     // Link noise function with sound machine
     sound.SetUserFunction(makeNoise);

     double dOctaveBaseFrequency = 110.0; //A2
     double d12thRootOf2 = pow(2.0, 1.0 / 12.0);

     while (1) {
         // Add a keyboard
         /*if (GetAsyncKeyState('A') & 0x8000) {
             dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, 1);
         }
         else {
             dFrequencyOutput = 0.0;
         }*/

         // Add a keyboard like a piano
         bool bKeyPRessed = false;
         for (int k = 0; k < 15; k++)
         {
             if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe"[k])) & 0x8000) {
                 dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
                 bKeyPRessed = true;
             }
         }

         if (!bKeyPRessed) {
             dFrequencyOutput = 0.0;
         }
         
     }

     return 0;
}