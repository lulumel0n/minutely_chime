// ConsoleApplication1.cpp
// Midi voice code comes from:
// https://www.daniweb.com/programming/software-development/code/216360/play-a-midi-voice-dev-c-console-program

#include "stdafx.h"
#include <iostream> 
#include <cmath>
#include <ctime>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

using namespace std;

#define SNDQUE 10000

typedef struct _soundtype
{
	double  Freq;
	int     Dura;
	int     Vol;
	int     Voice;
	double  Tempo;
	int     sndTid;
} soundtype, *LPSOUNDTYPE;

static soundtype  SndPmtr[SNDQUE + 1];
static int        gTenter;
static int        gTwait;
static int        gTexit;
static int        gTarray;
static BOOL       gTsig;
static HANDLE     gSThread = NULL;


// Declare Methods
double  Round(double, int);
double  Abs(double);
int     Sound(float, int = 0, int = 127, int = 0, float = 1);
// changed this from int PlaySnd(void) to:
DWORD WINAPI PlaySnd(LPVOID);


int main()
{
	time_t t = time(0);   // get time now

	while (true)
	{
		int wait_time = 60 - (time(0) % 60);

		// beep next minute 00:00
		Sleep(wait_time * 1000);

		// Tugboat whistle sound 783 hertz, 355ms, 100 = loud
		Sound(783, 355, 100, 10);  // 3.55 second blast		
	}

	// wait till que is empty
	while (Sound(0) != 0)
	{
		Sleep(10);
	}

	return 0;
}

double Round(double n, int d)
{
	return (floor((n)*pow(10.0, (d)) + 0.5) / pow(10.0, (d)));
}
double Abs(double a)
{
	if (a < 0)
	{
		return -a;
	}

	return  a;
}

// https://www.daniweb.com/programming/software-development/code/216360/play-a-midi-voice-dev-c-console-program
int Sound(float Freq, int Dura, int Vol, int Voice, float Tempo)
{
	DWORD  dwThreadId;

	if (Freq == 0 && Dura < 1)
	{
		return gTenter - gTexit;
	}

	// silence
	if (Freq == 0) Vol = 0;
	if (Dura < 5) Dura = 5;

	gTenter++;
	gTsig = FALSE;

	if (gTenter >= SNDQUE)
	{
		gTarray = gTenter % SNDQUE + 1;
	}
	else
	{
		gTarray = gTenter;
	}

	SndPmtr[gTarray].Freq = Freq;
	SndPmtr[gTarray].Dura = Dura;
	SndPmtr[gTarray].Tempo = Tempo;
	SndPmtr[gTarray].Vol = Vol;
	SndPmtr[gTarray].Voice = Voice;
	SndPmtr[gTarray].sndTid = gTenter;

	if (gSThread == NULL && (Freq == Abs(Freq) || Freq == 0))
	{
		// "PlaySnd" needs casting (void *)
		gSThread = CreateThread(NULL, 0, PlaySnd, (void *)"PlaySnd", 0, &dwThreadId);
		Sleep(1);
		return 0;
	}

	if (Freq != Abs(Freq))
	{
		if (Freq == -1)
		{
			Freq = 0;
			SndPmtr[gTarray].Vol = 0;
		}

		SndPmtr[gTarray].Freq = Abs(Freq);
		gTsig = TRUE;

		while (gSThread != NULL)
		{
			Sleep(10);
		}

		gTexit = gTenter - 1;
		gTwait = gTenter - 1;
		gTsig = FALSE;

		return PlaySnd(0);  // needs some kind of argument
	}
	return 0;
}

// https://www.daniweb.com/programming/software-development/code/216360/play-a-midi-voice-dev-c-console-program
DWORD WINAPI PlaySnd(LPVOID)
{
	soundtype  LocSndPar;
	int  lTarray;

	while (gTenter > gTexit && gTsig == FALSE)
	{
		gTwait++;

		if (gTwait >= SNDQUE)
			lTarray = gTwait % SNDQUE + 1;
		else
			lTarray = gTwait;

		LocSndPar = SndPmtr[lTarray];
		int Note = 0;
		int Phrase = 0;

		HMIDIOUT hMidi;

		midiOutOpen(&hMidi, (UINT)-1, 0, 0, CALLBACK_NULL);
		midiOutShortMsg(hMidi, (256 * LocSndPar.Voice) + 192);

		// convert frequency to midi note
		Note = (int)Round((log(LocSndPar.Freq) - log(440.0)) / log(2.0) * 12 + 69, 0);
		Phrase = (LocSndPar.Vol * 256 + Note) * 256 + 144;

		midiOutShortMsg(hMidi, Phrase);
		Sleep((int)(LocSndPar.Dura*(1 / LocSndPar.Tempo + 0.0001)));
		Phrase = (LocSndPar.Vol * 256 + Note) * 256 + 128;

		midiOutShortMsg(hMidi, Phrase);
		midiOutClose(hMidi);
		gTexit++;
	}
	CloseHandle(gSThread);
	gSThread = NULL;
	return 0;
}
