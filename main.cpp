#include <iostream>
using namespace std;

#include "olcNoiseMaker.h"

// Converts frequency (Hz) into angular velocity
double w(double dHertz) {
    return dHertz * 2.0 * PI;
}

// General purpose oscillator
#define OSC_SINE 0
#define OSC_SQUARE 1
#define OSC_TRIANGLE 2
#define OSC_SAW_ANA 3
#define OSC_SAW_DIG 4
#define OSC_NOISE 5

double osc(double dHertz, double dTime, int nType = OSC_SINE) {
    switch (nType) {
    case OSC_SINE: // sine wave between -1 and +1
        return sin(w(dHertz) * dTime);
    case OSC_SQUARE: // square wave between -1 and +1
        return sin(w(dHertz) * dTime) > 0 ? 1.0 : -1.0;
    case OSC_TRIANGLE: // triangle wave between -1 and +1
        return asin(sin(w(dHertz) * dTime) * 2.0 / PI);
    case OSC_SAW_ANA: // Saw wave (analogue / warm / slow)
    {
        double dOutput = 0.0;

        for (double n = 1.0; n < 100.0; n++) {
            dOutput += (sin(w(dHertz) * dTime)) / n;
        }

        return dOutput * (2.0 / PI);
    }
    case OSC_SAW_DIG: // Saw wave (optimised / harsh / fast)
        return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));
    case OSC_NOISE: // Psuedo Random Noise
        return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;
    default:
        return 0.0;
    }
}

struct sEnvelopeADSR {
    double dAttackTime;
    double dDecayTime;
    double dReleaseTime;

    double dSustainAmplitude;
    double dStartAmplitude;

    double dTriggerOnTime;
    double dTriggerOffTime;

    bool bNoteOn;

    sEnvelopeADSR() {
        dAttackTime = 0.01;
        dDecayTime = 0.01;
        dStartAmplitude = 1.0;
        dSustainAmplitude = 0.8;
        dReleaseTime = 0.02;
        dTriggerOnTime = 0.0;
        dTriggerOffTime = 0.0;
        bNoteOn = false;
    }

    // Get the correct amplitude at the requested point in time
    double GetAmplitude(double dTime) {
        double dAmplitude = 0.0;
        double dLifeTime = dTime - dTriggerOnTime;

        if (bNoteOn) {
            // ADS

            // Attack
            if (dLifeTime <= dAttackTime) {
                // In teh attack phase - approach max amplitude
                dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;
            }

            // Decay
            if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime)) {
                // In the decay phase - reduce to sustained amplitude
                dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;
            }

            // Sustain
            if (dLifeTime > (dAttackTime + dDecayTime)) {
                // In sustain phase - don't change until note released
                dAmplitude = dSustainAmplitude;
            }
        }
        else {
            // Release
            // Note has been released, so in release phase
            dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude;
        }

        // Epsilon check
        // Amplitude should not be negative
        if (dAmplitude <= 0.0001)
            dAmplitude = 0;

        return dAmplitude;
    }

    // Call when key is pressed
    void NoteOn(double dTimeOn) {
        dTriggerOnTime = dTimeOn;
        bNoteOn = true;
        // wcout << bNoteOn <<endl;
    }

    // Call when key is released
    void NoteOff(double dTimeOff) {
        dTriggerOffTime = dTimeOff;
        bNoteOn = false;
        // wcout << bNoteOn << endl;
    }
};

atomic<double> dFrequencyOutput = 0.0;      // dominant output frequency of inrtument, i.e. the note
double dOctaveBaseFrequency = 110.0; //A2   // frequency of octave represented by ketboard
double d12thRootOf2 = pow(2.0, 1.0 / 12.0); // assuming western 12 notes per octave
sEnvelopeADSR envelope;

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
    double dOutput = envelope.GetAmplitude(dTime) * osc(dFrequencyOutput, dTime, OSC_SAW_ANA);
    return dOutput * 0.4; // Master volume

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
    // double dOutput = 1.0 * (sin(dFrequencyOutput * 2 * 3.1459 * dTime) + sin((dFrequencyOutput + 20.0) * 2.0 * 3.1459 * dTime));
    // return dOutput * 0.4;
}

int main() {
    wcout << "Synthesizer" << endl;

    // Get all sound hardware
    vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

    //Display findings
    for (auto d : devices) wcout << "Found Output Device:" << d << endl;

    // Display a keyboard
    wcout << endl <<
        "|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << endl <<
        "|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << endl <<
        "|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
        "|     |     |     |     |     |     |     |     |     |     |" << endl <<
        "|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
        "|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

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

     while (1) {
         // Add a keyboard
         /*if (GetAsyncKeyState('A') & 0x8000) {
             dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, 1);
         }
         else {
             dFrequencyOutput = 0.0;
         }*/

         // Add a keyboard like a piano
         int nCurrentKey = -1;
         bool bKeyPressed = false;
         for (int k = 0; k < 16; k++)
         {
             bKeyPressed = false;
             if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000) {
                 if (nCurrentKey != k) {
                     dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
                     envelope.NoteOn(sound.GetTime());
                     wcout << "\rNote On : " << sound.GetTime() << "s " << dFrequencyOutput << "Hz";
                     nCurrentKey = k;
                 }
                 bKeyPressed = true;
             }
         }

         if (!bKeyPressed) {
             if (nCurrentKey != -1) {
                wcout << "\rNote Off : " << sound.GetTime() << "s                        ";
                envelope.NoteOff(sound.GetTime());
             }
             // dFrequencyOutput = 0.0; // Not needed when using our envelope
         }
         
     }

     return 0;
}