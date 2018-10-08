/***************************************************************************
 *   Copyright (C)2013 by Miguel Angel Rodriguez Jodar                     *
 *   miguel.angel@zxprojects.com | http://www.zxprojects.com               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <semaphore.h>
#endif

#ifdef SUPPORT_PLAY_CMD
#include "SDL.h"
#endif

#define PROGRAM_VERSION "v 1.0.3 (May-20-2013)"

#define MIN_SAMPLES_BTW_BITS 25
#define MAX_SAMPLES_BTW_BITS 100
#define MIN_SAMPLES_BTW_EDGES 1
#define MAX_SAMPLES_BTW_EDGES 15

#define MIN_NUM_EDGES_0 7
#define MAX_NUM_EDGES_0 12
#define MIN_NUM_EDGES_1 17
#define MAX_NUM_EDGES_1 22

#define EOF_AUDIO -1

#define START_VARS 0x4009
#define VERSN (0x4009-START_VARS)
#define D_FILE (0x400c-START_VARS)
#define VARS (0x4010-START_VARS)
#define E_LINE (0x4014-START_VARS)
#define MARGIN (0x4028-START_VARS)
#define NXTLIN (0x4029-START_VARS)

int w2p = 1;
int p2w = 0;
int tzx = 0;
char ifilename[1024];
char ofilename[1024];
int showscr = 0;
int showbasic = 0;
int play = 0;
int showprogress = 1;
int preprocess = 1;
int debug = 0;
int debuglevel = 0;

int le = 1;
unsigned short nchan;
unsigned short bps;
unsigned sfreq;
unsigned audiolen;

short *audio;
short *oaudio;
short threh, threl;
int thretotal = 500;

unsigned char pname[128];
unsigned char *pfile;
unsigned ipfile=0;

char cset[65]=" ??????????\"`$:?()><=+-*/;,.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *tokens[256]=
{
	" ", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "\"", "`", "$", ":",
	"?", "(", ")", ">", "<", "=", "+", "-", "*", "/", ";", ",",  ".", "0", "1",
	"2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D",  "E", "F", "G",
	"H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S",  "T", "U", "V",
	"W", "X", "Y", "Z", "RND", "INKEY$", "PI",
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", 
	"?",
	" ", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "\"", "`", "$", ":",
	"?", "(", ")", ">", "<", "=", "+", "-", "*", "/", ";", ",",  ".", "0", "1",
	"2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D",  "E", "F", "G",
	"H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S",  "T", "U", "V",
	"W", "X", "Y", "Z",
	"\"\"", "AT ", "TAB ", "?", "CODE ", "VAL ", "LEN ", "SIN ", "COS ", "TAN ",
	"ASN ", "ACS ", "ATN ", "LN ", "EXP ", "INT ", "SQR ", "SGN ", "ABS ",
	"PEEK ", "USR ", "STR$ ", "CHR$ ", "NOT ", "**", " OR ", " AND ", "<=", ">=",
	"<>", " THEN", " TO ", " STEP ", " LPRINT ", " LLIST ", " STOP ", " SLOW ", 
	" FAST ", " NEW ", " SCROLL ", " CONT ", " DIM ", " REM ", " FOR ", " GOTO ",
	" GOSUB ", " INPUT ", " LOAD ", " LIST ", " LET ", " PAUSE ", " NEXT ",
	" POKE ", " PRINT ", " PLOT ", " RUN ", " SAVE ", " RAND ", " IF ", " CLS ",
	" UNPLOT ", " CLEAR ", " RETURN ", " COPY "
};

unsigned char wavehdr[]={'R','I','F','F',0,0,0,0,'W','A','V','E','f','m','t',' ',
                         0x10,0,0,0,
						 0x1,0,
						 0x1,0,
						 0x44,0xac,0,0,
						 0x88,0x58,0x01,0,
						 0x2,0,
						 0x10,0,
						 'd','a','t','a',
						 0,0,0,0
						};

#define SZERO 0,0
#define SHIGH 0xff,0x7f
#define SLOW 0x00,0x80 

unsigned char space_BTW_bits[]={SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,
                               SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,
                               SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,
                               SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,
                               SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,
                               SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,
                               SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO,SZERO};
unsigned char bit_zero[]={SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                          SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                          SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                          SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                          SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                          SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                          SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                          SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW};
unsigned char bit_one[]={SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                         SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                         SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                         SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                         SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                         SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                         SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,
                         SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW, 
                         SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,SHIGH,
                         SLOW,SLOW,SLOW,SLOW,SLOW,SLOW,SLOW};

#define LAUDIOBUFFER 16384
unsigned char *SDLaudio;
volatile unsigned SDLplaypos;
volatile int playback_finished;

int check_endian (void)
{
	union
	{
		unsigned int n;
		unsigned char c[4];
	} v;
	
	v.n = 0x12345678;
	if (v.c[0]==0x12)
		return 0;
	else
		return 1;
}

unsigned short to16bitle (unsigned short n)
{
	if (le)
		return n;
	else
		return (n&0xFF)<<8 | (n>>8);
}

unsigned to32bitle (unsigned n)
{
	if (le)
		return n;
	else
		return (n&0xFF)<<24 | (n&0x0000FF00)<<8 | (n&0x00FF0000)>>8 | (n&0xFF000000)>>24;
}

#ifdef SUPPORT_PLAY_CMD
#ifdef WIN32
HANDLE hSemConsumer, hSemProducer;

void CreateSem (void)
{
	hSemConsumer = CreateSemaphore(NULL,0,1,NULL);
	hSemProducer = CreateSemaphore(NULL,1,1,NULL);
}

void SleepConsumer (void)
{
	WaitForSingleObject(hSemConsumer,INFINITE);
}

void SleepProducer (void)
{
	WaitForSingleObject(hSemProducer,INFINITE);
}

void WakeupConsumer (void)
{
	ReleaseSemaphore(hSemConsumer, 1, NULL);
}

void WakeupProducer (void)
{
	ReleaseSemaphore(hSemProducer, 1, NULL);
}

void DestroySem (void)
{
	CloseHandle (hSemProducer);
	CloseHandle (hSemConsumer);
}

#else
sem_t SemConsumer, SemProducer;

void CreateSem (void)
{
	sem_init (&SemConsumer,0,0);
	sem_init (&SemProducer,0,1);
}

void SleepConsumer (void)
{
	sem_wait (&SemConsumer);
}

void SleepProducer (void)
{
	sem_wait (&SemProducer);
}

void WakeupConsumer (void)
{
	sem_post (&SemConsumer);
}

void WakeupProducer (void)
{
	sem_post (&SemProducer);
}

void DestroySem (void)
{
	sem_destroy (&SemProducer);
	sem_destroy (&SemConsumer);
}

#endif
#endif  /* SUPPORT_PLAY_CMD */
void SaveRAW (char *n, void *s, unsigned len)
{
	FILE *f;
	
	f = fopen (n, "wb");
	fwrite (s, 1, len, f);
	fclose (f);
}

int GetWAVParms (char *cab)
{
	unsigned short ispcm;
	
	if (strncmp(cab,"RIFF",4)!=0 || strncmp(cab+8,"WAVEfmt",7)!=0 || strncmp(cab+36,"data",4)!=0)
	{
		printf ("File is not WAV format\n");
		return 1;
	}
	
	ispcm = to16bitle ( *(unsigned short *)(cab+20));
	if (ispcm != 1)
	{
		printf ("WAVE file is not a PCM file\n");
		return 1;
	}
	
	nchan = to16bitle ( *(unsigned short *)(cab+22));
	if (nchan != 1 && nchan != 2)
	{
		printf ("WAVE file must be mono or stereo\n");
		return 1;
	}
	
	sfreq = to32bitle ( *(unsigned *)(cab+24));
	if (sfreq != 44100)
	{
		printf ("WAVE file must have a 44100 Hz sampling frequency\n");
		return 1;
	}
	
	bps = to16bitle ( *(unsigned short *)(cab+34));
	if (bps !=16)
	{
		printf ("WAVE file muyst be 16 bits audio sample\n");
		return 1;
	}
	
	audiolen = to32bitle ( *(unsigned *)(cab+40));
	
	return 0;
}

int LoadWAV (char *name)
{
	FILE *f;
	char cabecera[44];
	unsigned leido;
	
	f = fopen (name, "rb");
	if (!f)
	{
		printf ("File <%s> cannot be read\n", name);
		return 1;
	}
	
	fread (cabecera, 1, 44, f);
	if (GetWAVParms (cabecera))
		return 1;
		
	oaudio = malloc (audiolen);
	if (!oaudio)
	{
		printf ("Not enough memory. Too big WAV??\n");
		return 1;
	}
	
	leido = fread (oaudio, 1, audiolen, f);
	if (leido<audiolen)
	{
		printf ("Cannot read full audio file. Read %d bytes\n", leido);
		return 1;
	}
	fclose (f);
	
	return 0;
}

void Stereo2Mono (void)
{
	int temp;
	unsigned i;
	
	if (nchan==1)
	{
		audio = oaudio;
		audiolen = audiolen / 2;
		return;
	}
	
	audio = malloc (audiolen/2);
	for (i=0;i<(audiolen/4);i++)
	{
		temp = oaudio[i*2]+oaudio[i*2+1];
		if (temp>32767)
			temp = 32767;
		else if (temp<-32768)
			temp = -32768;
		audio[i] = temp;
	}
	audiolen = audiolen / 4;
	free (oaudio);
}

void RemoveDC (short *s, unsigned len)
{
	unsigned pri,fin;
	short peakp, peakn;
	int dc,temp;
	unsigned i;
	
	// Estimate DC component measuring middle section of sample wave
	pri=len/2-len/10;
	fin=len/2+len/10;
	
	peakp=s[pri];
	peakn=s[pri];
	for (i=pri;i<fin;i++)
	{
		if (s[i]>peakp)
			peakp=s[i];
		if (s[i]<peakn)
			peakn=s[i];
	}
	
	dc = (peakp + peakn)/2;
	// Apply DC offset to entire audio signal
	for (i=0;i<len;i++)
	{
		temp = s[i] - dc;
		if (temp>32767)
			temp = 32767;
		else if (temp<-32768)
			temp = -32768;
		s[i] = temp;
	}
		
	threh = 32767/10;
	threl = -32768/10;
}

void Squarize (short *s, unsigned len)
{
	short signal = 0;
	unsigned i;
	
	for (i=0;i<len;i++)
	{
		if (s[i]>=threh)
			signal = 32767;
		else if (s[i]<=threl)
			signal = -32768;
		
		s[i] = signal;
	}
}

int FindNextEdge (short *s, unsigned len, unsigned *pos)
{
	short v;
	int lpulso = 0;
		
	v = s[*pos];
	while (*pos<len && abs(s[*pos]-v)<thretotal)
	{
		(*pos)++;
		lpulso++;
	}
	if (*pos==len)
		lpulso = EOF_AUDIO;
		
	return lpulso;
}

int ReadAudio (short *s, unsigned len)
{
	int lpulso;
	int estado = 0;
	unsigned pos = 0;
	unsigned char byteleido = 0;
	int contpulsos = 0;
	int contbits = 0;
	int min;
	float secs;
	
	while (estado!=99)
	{
		switch (estado)
		{
			case 0:
				lpulso = FindNextEdge (s, len, &pos);

				secs = pos/44100.0;
				min = secs/60;
				secs = secs - min*60;

				if (debug && debuglevel>=2) printf ("State 0: [%2d:%6.3f]. Next edge at %d samples\n", min, secs, lpulso);
				if (lpulso==EOF_AUDIO)
					estado=99;
				else if (lpulso<MIN_SAMPLES_BTW_EDGES || lpulso>MAX_SAMPLES_BTW_BITS)
					estado=0;
				else
				{
					estado=1;
					contpulsos=1;
				}
				break;
				
			case 1:
				lpulso = FindNextEdge (s, len, &pos);

				secs = pos/44100.0;
				min = secs/60;
				secs = secs - min*60;

				if (debug && debuglevel>=2) printf ("State 1: [%2d:%6.3f]. Next edge at %d samples\n", min, secs, lpulso);
				if (lpulso==EOF_AUDIO)
					estado=99;
				else if (lpulso>MIN_SAMPLES_BTW_BITS)
					estado=2;
				else if (lpulso<MIN_SAMPLES_BTW_EDGES)
					estado=99;
				else if (lpulso>=MIN_SAMPLES_BTW_EDGES && lpulso<=MAX_SAMPLES_BTW_EDGES)
					contpulsos++;
				break;
				
			case 2:
				if (contpulsos>=MIN_NUM_EDGES_0 && contpulsos<=MAX_NUM_EDGES_0)
				{
					if (debug && debuglevel>=2) printf ("State 2: [%2d:%6.3f]. Edges found: %d. Bit 0\n", min, secs, contpulsos);
					byteleido = (byteleido<<1);
					contbits++;
					estado=3;
				}
				else if (contpulsos>=MIN_NUM_EDGES_1 && contpulsos<=MAX_NUM_EDGES_1)
				{
					if (debug && debuglevel>=2) printf ("State 2: [%2d:%6.3f]. Edges found: %d. Bit 1\n", min, secs, contpulsos);
					byteleido = (byteleido<<1) | 1;
					contbits++;
					estado=3;
				}
				else
				{
					if (debug && debuglevel>=1) printf ("State 2: [%2d:%6.3f]. Edges found: %d. Bit discarded\n", min, secs, contpulsos);
					estado=0;
				}
				break;
				
			case 3:
				contpulsos = 0;
				if (contbits==8)
				{
					if (debug && debuglevel>=2) printf ("State 3: [%2d:%6.3f]. Byte read: %.2X\n", min, secs, byteleido);
					pfile[ipfile]=byteleido;
					byteleido=0;
					ipfile++;
					contbits=0;
				}
				estado=0;
				break;
		}
	}
				
	if (lpulso==EOF_AUDIO)
	{
		if (contbits!=0)
			pfile[ipfile++]=0x80;
		return 0;
	}
	else
	{
		printf ("Read error at [%2d:%6.3f]\n", min, secs);
		return 1;
	}
}

void RenderScreen (unsigned char *p)
{
	int nlin=0;
	int column=0;
	int i=1;
	
	printf ("ZX81 Screen: \n");
	printf ("+--------------------------------+\n");
	putchar ('|');
	while (nlin<24)
	{
		if ((p[i]&0x40)==0 && column<32)
		{
			putchar (cset[p[i]&0x3f]);
			column++;
		}
		else
		{
			for (;column<32;column++)
				putchar(' ');
			printf ("|\n");
			column=0;
			nlin++;
			if (nlin<24)
				putchar ('|');
		}
		i++;
	}
	printf ("+--------------------------------+\n\n");
}

void RenderBASIC (unsigned char *p, int len)
{
	int i=0,j;
	int nuevalinea=1;
	int numlinea;
	int rawbytemode = 0;

	printf ("  BASIC listing  \n");
	printf ("------------------------------------------------\n");	
	do
	{
		if (nuevalinea)
		{
			nuevalinea = 0;
			numlinea = p[i]*256+p[i+1];
			printf ("%4d", numlinea);
			i = i + 4;
		}
		else if (p[i]==0x7e)  // number
		{
			i = i + 6;
		}
		else if (p[i]==0x76)  // ENTER
		{
			nuevalinea = 1;
			printf ("\n");
			rawbytemode = 0;
			i++;
		}
		else
		{
			if (!rawbytemode)
				printf ("%s", tokens[p[i]]);
			else
				printf ("[%02.2X]", p[i]);
			if (p[i]==234)  // REM
			{
				for (j=i+1;p[j]!=0x76;j++)
					if (p[j]>=67 && p[j]<=127)  // possible MC here
						rawbytemode = 1;
			}
			i++;
		}
	}
	while (i<len);
	printf ("------------------------------------------------\n\n");
}

void ParseSavePFile (unsigned char *p, unsigned len)
{
	int i=0;
	int videop;
	int vars,endsave;
	int nxtlin;
	
	do
	{
		pname[i]=cset[p[i]&0x3f];
		i++;
	}
	while (!(p[i-1]&0x80));
	pname[i]='\0';	
	printf ("ZX81 filename            : [%s]\n", pname);
	
	p = &p[i];
	printf ("File type                : ");
	if (p[VERSN]==0)
		printf ("Cassette\n");
	else
		printf ("Lambda\n");
		
	videop = to16bitle ( *(unsigned short *)(p+D_FILE));
	vars = to16bitle ( *(unsigned short *)(p+VARS));
	endsave = to16bitle ( *(unsigned short *)(p+E_LINE));

	printf ("Video frame buffer at    : %4.4X ", videop);
	if (vars-videop == 793)
		printf ("(Video buffer expanded)\n");
	else
		printf ("(Video buffer collapsed)\n");

	printf ("BASIC variables begin at : %04.4X\n", vars);
	printf ("End of save area         : %04.4X\n", endsave);
	
	printf ("Video mode               : ");
	if (p[MARGIN]==55)
		printf ("PAL 50Hz\n");
	else if (p[MARGIN]==31)
		printf ("NTSC 60Hz\n");
	else
		printf ("Unknown (%d lines for top/bottom border)\n", p[MARGIN]);
		
	nxtlin = to16bitle ( *(unsigned short *)(p+NXTLIN));
	printf ("Next BASIC line          : %04.4X ", nxtlin);
    if (nxtlin>=videop)
        printf ("(doesn't seem to autostart)\n");
    else
        printf ("(autostarts after loading)\n");
	
	printf ("\n");

    videop = videop - 0x4009;
	if (showscr)
		RenderScreen (p+videop);

	if (showbasic)
		RenderBASIC (p+0x74, videop-0x74);
		
	if (w2p)
	{
		if (!ofilename[0])
		{	
			strcpy (ofilename, pname);	
			strcat (ofilename, ".P");
		}		
		len = len - strlen(pname);
		SaveRAW (ofilename, p, len);
	}
}

int DoWAV2P (void)
{
	int res;
	
	if (LoadWAV (ifilename))
		return 1;
	
	pfile = malloc(65536);
	ipfile=0;
		
	Stereo2Mono();
	if (preprocess)
	{
		RemoveDC (audio, audiolen);
		Squarize (audio, audiolen);
	}
	res = ReadAudio (audio, audiolen);
	if (res)
		return 1;
	ParseSavePFile (pfile, ipfile);
	//SaveRAW ("salida.raw", audio, audiolen*2);
	return 0;
}

int GenerateWAV (unsigned char *p, int len)
{
	FILE *f;
    int datalen=0;
    unsigned i;
    unsigned char dato;
    int bit;
    int data2write;

    if (!ofilename[0])
    {
    	strcpy (ofilename, ifilename);
   		strcat (ofilename, ".wav");
    }
    
    f = fopen (ofilename, "wb+");
    if (!f)
    {
   		printf ("File <%s> couldn't be created\n", ofilename);
		return 1;
    }
   
    fwrite (wavehdr, 1, 44, f);

    for (i=0;i<10;i++)
    {
   		datalen += fwrite (space_BTW_bits, 1, sizeof(space_BTW_bits), f);
    }
   
    for (i=0;i<ipfile;i++)
    {
   		dato = p[i];
   		for (bit=0;bit<8;bit++)
   		{
   			if (dato&0x80)
   				datalen += fwrite (bit_one, 1, sizeof(bit_one), f);
			else
				datalen += fwrite (bit_zero, 1, sizeof(bit_zero), f);
			dato = dato<<1;
			datalen += fwrite (space_BTW_bits, 1, sizeof(space_BTW_bits), f);
   		}
    }

    for (i=0;i<4;i++)
    {
   		datalen += fwrite (space_BTW_bits, 1, sizeof(space_BTW_bits), f);
    }
    
	fseek (f, 40, SEEK_SET);
	data2write = to32bitle (datalen);
	fwrite (&data2write, 1, 4, f);
	
	fseek (f, 4, SEEK_SET);
	data2write = to32bitle (datalen+36);
	fwrite (&data2write, 1, 4, f);
	
	fclose (f);

	return 0;
}

int GenerateTZX (unsigned char *p, int len)
{
	FILE *f;
    unsigned data2write;
    unsigned char tzxhdr[]={'Z','X','T','a','p','e','!',0x1a,1,20};
    unsigned char tzxid19[]={0x19,
                             0,0,0,0,   // len + 0x58
                             0xe8,0x3,  // pause after this block (1000ms)
                             0,0,0,0,
                             0,
                             0,
                             0,0,0,0,  // len*8
                             0x12,
                             2,
                             3,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x51,0x12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                             3,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x8,0x2,0x12,0x2,0x51,0x12
                            };

    data2write = to32bitle(len+0x58);
    *( (unsigned int *)(tzxid19+1)) = data2write;
    data2write = to32bitle(len*8);
    *( (unsigned int *)(tzxid19+0xd)) = data2write;

    if (!ofilename[0])
    {
    	strcpy (ofilename, ifilename);
   		strcat (ofilename, ".tzx");
    }
    
    f = fopen (ofilename, "wb");
    if (!f)
    {
   		printf ("File <%s> couldn't be created\n", ofilename);
		return 1;
    }

    fwrite (tzxhdr, 1, sizeof(tzxhdr), f);
    fwrite (tzxid19, 1, sizeof(tzxid19), f);
    fwrite (p, 1, len, f);

    fclose(f);
    return 0;
}

#ifdef SUPPORT_PLAY_CMD
void WriteSDLAudio (unsigned char *buff, int len)
{
	if (playback_finished)
		return;

	if (SDLplaypos>=LAUDIOBUFFER)
	{
//printf ("Producer: len=%d, SDLplaypos=%d\n", len, SDLplaypos);
		SDL_PauseAudio(0);
		SleepProducer();
	}
	
	SDL_LockAudio();
	memcpy (SDLaudio+SDLplaypos, buff, len);
	SDLplaypos += len;
	if (SDLplaypos>=LAUDIOBUFFER)
		WakeupConsumer();
	SDL_UnlockAudio();
}

void SDL_AudioCallback (void *userdata, Uint8 *stream, int len)
{
	if (SDLplaypos<len)
	{
//printf ("Consumer: len=%d, SDLplaypos=%d\n", len, SDLplaypos);
		if (!playback_finished)
			SleepConsumer();
		else
		{
			memcpy (stream, SDLaudio, SDLplaypos);
			memset (stream+SDLplaypos, 0, len-SDLplaypos);
			SDLplaypos = len;
			SDL_PauseAudio(1);
		}
	}

	memcpy (stream, SDLaudio, len);
	memcpy (SDLaudio, SDLaudio+len, SDLplaypos-len);
	SDLplaypos = SDLplaypos-len;
	if (SDLplaypos<LAUDIOBUFFER)
		WakeupProducer();
}

int InitAudio (void)
{
	SDL_AudioSpec parms;
	int res;
	
	parms.freq=44100;
	parms.format=AUDIO_S16LSB;
	parms.channels=1;
	parms.samples=LAUDIOBUFFER/2;
	parms.callback=SDL_AudioCallback;
	parms.userdata=NULL;
	
	res=SDL_OpenAudio (&parms, NULL);
	if (res<0)
	{
		fprintf (stderr, "Unable to open SDL audio device: %s\n", SDL_GetError() );
		return 1;
	}
	SDL_PauseAudio(1);
	return 0;
}

void CloseAudio (void)
{
	SDL_audiostatus estado;

	playback_finished = 1;
	do
	{
		SDL_PauseAudio(1);
		estado = SDL_GetAudioStatus();
	}
    while (estado==SDL_AUDIO_PLAYING || playback_finished==2);	
	SDL_CloseAudio();	
}

int PlayPFile (unsigned char *p, int len)
{
    int datalen=0;
    unsigned i,j;
    unsigned char dato;
    int bit;

	if (InitAudio())
		return 1;

	CreateSem();
	SDLaudio = malloc (LAUDIOBUFFER*2);
	if (!SDLaudio)
	{
		printf ("Cannot allocate audio buffer por playback\n");
		CloseAudio();
		return 1;
	}
	SDLplaypos = 0;
	
	printf ("Playing...\n");
	playback_finished = 0;

	for (i=0;i<10;i++)
    {
   		WriteSDLAudio (space_BTW_bits, sizeof(space_BTW_bits));
    }
   
    for (i=0;i<ipfile && !playback_finished;i++)
    {
		if (showprogress)
		{
			printf ("[");
			for (j=0;j<i*50/(ipfile-1);j++)
				putchar ('-');
			for (;j<50;j++)
				putchar (' ');
			printf ("]  \r");
			if ((i%16)==0)
				fflush (stdout);
		}

		dato = p[i];
   		for (bit=0;bit<8;bit++)
   		{
   			if (dato&0x80)
   				WriteSDLAudio (bit_one, sizeof(bit_one));
			else
				WriteSDLAudio (bit_zero, sizeof(bit_zero));
			dato = dato<<1;
			WriteSDLAudio (space_BTW_bits, sizeof(space_BTW_bits));
   		}
    }

    for (i=0;i<4;i++)
    {
   		WriteSDLAudio (space_BTW_bits, sizeof(space_BTW_bits));
    }

	playback_finished = 1;
	CloseAudio();	
    free (SDLaudio);
	DestroySem();

	printf ("\n");
	return 0;
}
#endif

int DoP2WAV (void)
{
	FILE *f;
	
	pfile=malloc(65536);
	f = fopen (ifilename, "rb");
	if (!f)
	{
		printf ("File <%s> cannot be read\n", ifilename);
		return 1;
	}
	if (strlen(ifilename)>2)
		ifilename[strlen(ifilename)-2]='\0';
	for (ipfile=0;ifilename[ipfile];ipfile++)
	{
		if (ifilename[ipfile]>='A' && ifilename[ipfile]<='Z')
			pfile[ipfile] = ifilename[ipfile]-'A'+38;
		else if (ifilename[ipfile]>='a' && ifilename[ipfile]<='z')
			pfile[ipfile] = ifilename[ipfile]-'a'+38;
		else if (ifilename[ipfile]>='0' && ifilename[ipfile]<='9')
			pfile[ipfile] = ifilename[ipfile]-'0'+28;
		else
			pfile[ipfile] = 22;  // '-'
	}		
	pfile[ipfile-1] |= 0x80;
	
	ipfile += fread (pfile+ipfile, 1, 65536-ipfile, f);
	fclose (f);
	
	ParseSavePFile (pfile, ipfile);
	
	if (p2w)
		return GenerateWAV (pfile, ipfile);
#ifdef SUPPORT_PLAY_CMD
	else if (play)
		return PlayPFile (pfile, ipfile);
#endif
	else if (tzx)
        return GenerateTZX (pfile, ipfile);

	return 0;
}

void funcion_atexit (void)
{
#ifdef SUPPORT_PLAY_CMD
	CloseAudio();
	SDL_Quit();
#endif
}

#ifdef WIN32
static BOOL WINAPI console_ctrl_handler(DWORD dwCtrlType)
{
  switch (dwCtrlType)
  {
  case CTRL_C_EVENT: // Ctrl+C
  case CTRL_BREAK_EVENT: // Ctrl+Break
  case CTRL_CLOSE_EVENT: // Closing the console window
	  playback_finished = 1;  // this is another thread. Don't make SDL calls here.
	return TRUE;
  }

  // Return TRUE if handled this message, further handler functions won't be called.
  // Return FALSE to pass this message to further handlers until default handler calls ExitProcess().
  return FALSE;
}
#endif

void usage (void)
{
	printf ("zx81putil [options] input_filename\n");
	printf ("  OPTIONS\n");
	printf ("   -o outputfile  : overrides default file name for output.\n");
	printf ("   -w2p           : converts a WAV file into a P file (default operation)\n");
	printf ("   -p2w           : converts a P file into a WAV file\n");
    printf ("   -tzx           : converts a P file into a TZX file\n");
#ifdef SUPPORT_PLAY_CMD
	printf ("   -play          : plays a P file thru the computer audio system\n");
	printf ("   -noprogress    : don't show progress bar during PLAY (def. is to show it)\n");
#endif
	printf ("   -scr           : shows the screen buffer contents as saved into the file\n");
	printf ("   -bas           : shows the BASIC listing as saved into the file\n");
	printf ("   input_filename : WAV or P file to convert/play (def. WAV file is assumed)\n");
	printf ("  ADVANCED OPTIONS\n");
	printf ("   -nopreprocess  : don't preprocess (DC removal and square shape) audio signal\n");
	printf ("   -distance N    : sets N as minimum difference between two adjacent samples\n");
	printf ("                    to consider them part of an edge. Default is 500\n");
	printf ("   -debug LEVEL   : enable debug messages. LEVEL must be positive number\n");
	printf ("\n");
}

void copyright (void)
{
	printf ("ZX81PUTIL. Utils for transfering ZX81 software to/from PC. " PROGRAM_VERSION "\n"
	"(C)2013 Miguel Angel Rodriguez Jodar (http://www.zxprojects.com)\n\n");
}

void copyright2 (void)
{
	printf (
    "This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n\n"

    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n\n"

    "You should have received a copy of the GNU General Public License\n"
    "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n");
}

int main (int argc, char *argv[])
{
	int i;
	char *opc;
	
	check_endian();
	
	copyright();
	
	if (argc<2)
	{
		copyright2();
		usage();
		return 1;
	}
#ifdef SUPPORT_PLAY_CMD
    if ( SDL_Init(SDL_INIT_AUDIO) < 0 )
    {
        fprintf(stderr, "Cannot init SDL: %s\n", 
	           SDL_GetError());
        exit(1);
    }
#endif
	atexit(funcion_atexit);
	
#ifdef WIN32
	SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
#endif

	ofilename[0]='\0';
	ifilename[0]='\0';

	for (i=1;i<argc;i++)
	{
		opc = argv[i];
		if (opc[0]!='-')
		{
			strcpy (ifilename, opc);
		}
		else
		{
			if (!strcmp (opc, "-o") && i<(argc-1) )
			{
				i++;
				strcpy (ofilename, argv[i]);
			}
			else if (!strcmp (opc, "-w2p"))
			{
				w2p = 1;
				p2w = 0;
				play = 0;
                tzx = 0;
			}
			else if (!strcmp (opc, "-p2w"))
			{
				p2w = 1;
				w2p = 0;
				play = 0;
                tzx = 0;
			}
#ifdef SUPPORT_PLAY_CMD
			else if (!strcmp (opc, "-play"))
			{
				p2w = 0;
				w2p = 0;
				play = 1;
                tzx = 0;
			}
#endif
			else if (!strcmp (opc, "-tzx"))
			{
				p2w = 0;
				w2p = 0;
				play = 0;
                tzx = 1;
			}
			else if (!strcmp (opc, "-scr"))
			{
				showscr = 1;
			}
			else if (!strcmp (opc, "-bas"))
			{
				showbasic = 1;
			}
			else if (!strcmp (opc, "-noprogress"))
			{
				showprogress = 0;
			}
			else if (!strcmp (opc, "-nopreprocess"))
			{
				preprocess = 0;
			}
			else if (!strcmp (opc, "-distance"))
			{
				i++;
				thretotal = atoi(argv[i]);
			}
			else if (!strcmp (opc, "-debug"))
			{
				debug = 1;
				i++;
				debuglevel = atoi(argv[i]);
			}
			else if (opc[0]=='-')
			{
				printf ("Unknown option: %s\n", opc);
				return 1;
			}
		}
	}

	if (w2p)
		return DoWAV2P();
	if (p2w || play || tzx)
		return DoP2WAV();

	return 0;
}	
