/*
  Audio functions
*/
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../zen.h"
#include "../../alib.h"
#include "../../aregex.h"
#include "../../atable.h"
#include "../../aexception.h"
#include "../../autil.h"
#include "../../aerror.h"

#include <SFML/Audio.h>

/********************************************************************
*                       Listener      Functions                     *
********************************************************************/

void listener_setglobalvolume(avm *vm)
{
    sfListener_SetGlobalVolume(getfloat(vm, 0));
}

void listener_getglobalvolume(avm *vm)
{
    word w;
    setf(&w, sfListener_GetGlobalVolume(getfloat(vm, 0)));
    returnv(vm, &w);
}

void listener_setposition(avm *vm)
{
    sfListener_SetPosition(getfloat(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void listener_getposition(avm *vm)
{
    word w;
    float x; float y; float z;
    sfListener_GetPosition(&x, &y, &z);
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    ewtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    word index, source;
    sets(&index, newstring(vm, "X"));
    setf(&source, x);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Y"));
    setf(&source, y);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Z"));
    setf(&source, z);
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    returnv(vm, &w);
}

void listener_settarget(avm *vm)
{
    sfListener_SetTarget(getfloat(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void listener_gettarget(avm *vm)
{
    word w;
    float x; float y; float z;
    sfListener_GetTarget(&x, &y, &z);
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    word index, source;
    sets(&index, newstring(vm, "X"));
    setf(&source, x);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Y"));
    setf(&source, y);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Z"));
    setf(&source, z);
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    returnv(vm, &w);
}


/********************************************************************
*                       Music         Functions                     *
********************************************************************/

void music_createfromfile(avm *vm)
{
    word w;
    seti(&w, sfMusic_CreateFromFile (getstring(vm, 0)));
    returnv(vm, &w);
}

void music_createfrommemory(avm *vm)
{
    word w;
    seti(&w, sfMusic_CreateFromMemory (getstring(vm, 0), getint(vm, 1)));
    returnv(vm, &w);
}

void music_destroy(avm *vm)
{
    sfMusic_Destroy (getint(vm, 0));
}

void music_setloop(avm *vm)
{
    sfMusic_SetLoop (getint(vm, 0), getint(vm, 1));
}

void music_getloop(avm *vm)
{
    word w;
    seti(&w, sfMusic_GetLoop(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_getduration(avm *vm)
{
    word w;
    setf(&w, sfMusic_GetDuration(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_play(avm *vm)
{
    sfMusic_Play(getint(vm, 0));
}

void music_pause(avm *vm)
{
    sfMusic_Pause(getint(vm, 0));
}

void music_stop(avm *vm)
{
    sfMusic_Stop(getint(vm, 0));
}

void music_getchannelcount(avm *vm)
{
    word w;
    seti(&w, sfMusic_GetChannelsCount(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_getsamplerate(avm *vm)
{
    word w;
    seti(&w, sfMusic_GetSampleRate(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_getstatus(avm *vm)
{
    word w;
    int x;
    switch (sfMusic_GetStatus(getint(vm, 0)))
    {
        case (sfStopped): x = 1; break;
        case (sfPaused): x = 2; break;
        case (sfPlaying): x = 3; break;
    }
    seti(&w, x);
    returnv(vm ,&w);
}

void music_getplayingoffset(avm *vm)
{
    word w;
    setf(&w, sfMusic_GetPlayingOffset(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_setpitch(avm *vm)
{
    sfMusic_SetPitch(getint(vm, 0), getfloat(vm, 1));
}

void music_setvolume(avm *vm)
{
    sfMusic_SetVolume(getint(vm, 0), getfloat(vm, 1));
}

void music_setposition(avm *vm)
{
    sfMusic_SetPosition(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2), getfloat(vm, 3));
}

void music_setrelativetolistener(avm *vm)
{
    sfMusic_SetRelativeToListener(getint(vm, 0), getint(vm, 1));
}

void music_setmindistance(avm *vm)
{
    sfMusic_SetMinDistance(getint(vm, 0), getfloat(vm, 1));
}

void music_setattenuation(avm *vm)
{
    sfMusic_SetAttenuation(getint(vm, 0), getfloat(vm, 1));
}

void music_getpitch(avm *vm)
{
    word w;
    setf(&w, sfMusic_GetPitch(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_getvolume(avm *vm)
{
    word w;
    setf(&w, sfMusic_GetVolume(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_getposition(avm *vm)
{
    word w;
    float x; float y; float z;
    sfMusic_GetPosition (getint(vm, 0), &x, &y, &z);
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    word index, source;
    sets(&index, newstring(vm, "X"));
    setf(&source, x);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Y"));
    setf(&source, y);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Z"));
    setf(&source, z);
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    returnv(vm, &w);
}

void music_isrelativetolistener(avm *vm)
{
    word w;
    seti(&w, sfMusic_IsRelativeToListener(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_getmindistance(avm *vm)
{
    word w;
    setf(&w, sfMusic_GetMinDistance(getint(vm, 0)));
    returnv(vm ,&w);
}

void music_getattenuation(avm *vm)
{
    word w;
    setf(&w, sfMusic_GetAttenuation(getint(vm, 0)));
    returnv(vm ,&w);
}

/********************************************************************
*                       Sound         Functions                     *
********************************************************************/

void sound_create(avm *vm)
{
    word w;
    seti(&w, sfSound_Create());
    returnv(vm ,&w);
}

void sound_destroy(avm *vm)
{
    sfSound_Destroy(getint(vm, 0));
}

void sound_play(avm *vm)
{
    sfSound_Play(getint(vm, 0));
}

void sound_pause(avm *vm)
{
    sfSound_Pause(getint(vm, 0));
}

void sound_stop(avm *vm)
{
    sfSound_Stop(getint(vm, 0));
}

void sound_setbuffer(avm *vm)
{
    sfSound_SetBuffer(getint(vm, 0), getint(vm, 1));
}

void sound_getbuffer(avm *vm)
{
    word w;
    seti(&w, sfSound_GetBuffer(getint(vm, 0)));
    returnv(vm, &w);
}

void sound_setloop(avm *vm)
{
    sfSound_SetLoop(getint(vm, 0), getint(vm, 1));
}

void sound_getloop(avm *vm)
{
    word w;
    seti(&w, sfSound_GetLoop(getint(vm, 0)));
    returnv(vm, &w);
}

void sound_getstatus(avm *vm)
{
    word w;
    int x;
    switch (sfSound_GetStatus(getint(vm, 0)))
    {
        case (sfStopped): x = 1; break;
        case (sfPaused): x = 2; break;
        case (sfPlaying): x = 3; break;
    }
    seti(&w, x);
    returnv(vm ,&w);
}

void sound_setpitch(avm *vm)
{
    sfSound_SetPitch(getint(vm, 0), getfloat(vm, 1));
}

void sound_setvolume(avm *vm)
{
    sfSound_SetVolume(getint(vm, 0), getfloat(vm, 1));
}

void sound_setposition(avm *vm)
{
    sfSound_SetPosition(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2), getfloat(vm, 3));
}

void sound_setrelativetolistener(avm *vm)
{
    sfSound_SetRelativeToListener(getint(vm, 0), getint(vm, 1));
}

void sound_setmindistance(avm *vm)
{
    sfSound_SetMinDistance(getint(vm, 0), getfloat(vm, 1));
}

void sound_setattenuation(avm *vm)
{
    sfSound_SetAttenuation(getint(vm, 0), getfloat(vm, 1));
}

void sound_setplayingoffset(avm *vm)
{
    sfSound_SetPlayingOffset(getint(vm, 0), getfloat(vm, 1));
}

void sound_getpitch(avm *vm)
{
    word w;
    setf(&w, sfSound_GetPitch(getint(vm, 0)));
    returnv(vm, &w);
}

void sound_getvolume(avm *vm)
{
    word w;
    setf(&w, sfSound_GetVolume(getint(vm, 0)));
    returnv(vm, &w);
}

void sound_getposition(avm *vm)
{
    word w;
    float x; float y; float z;
    sfSound_GetPosition (getint(vm, 0), &x, &y, &z);
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    word index, source;
    sets(&index, newstring(vm, "X"));
    setf(&source, x);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Y"));
    setf(&source, y);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Z"));
    setf(&source, z);
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    returnv(vm, &w);
}

void sound_isrelativetolistener(avm *vm)
{
    word w;
    seti(&w, sfSound_IsRelativeToListener(getint(vm, 0)));
    returnv(vm, &w);
}

void sound_getmindistance(avm *vm)
{
    word w;
    setf(&w, sfSound_GetMinDistance(getint(vm, 0)));
    returnv(vm, &w);
}

void sound_getattenuation(avm *vm)
{
    word w;
    setf(&w, sfSound_GetAttenuation(getint(vm, 0)));
    returnv(vm, &w);
}

void sound_getplayingoffset(avm *vm)
{
    word w;
    setf(&w, sfSound_GetPlayingOffset(getint(vm, 0)));
    returnv(vm, &w);
}

/********************************************************************
*                       SoundBuffer   Functions                     *
********************************************************************/

void soundbuffer_createfromfile(avm *vm)
{
    word w;
    seti(&w, sfSoundBuffer_CreateFromFile(getstring(vm, 0)));
    returnv(vm, &w);
}

void soundbuffer_createfrommemory(avm *vm)
{
    word w;
    seti(&w, sfSoundBuffer_CreateFromMemory(getstring(vm, 0), getint(vm, 1)));
    returnv(vm, &w);
}

void soundbuffer_createfromsamples(avm *vm)
{
    word w;
    seti(&w, sfSoundBuffer_CreateFromSamples(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3)));
    returnv(vm, &w);
}

void soundbuffer_destroy(avm *vm)
{
    sfSoundBuffer_Destroy(getint(vm, 0));
}

void soundbuffer_savetofile(avm *vm)
{
    sfSoundBuffer_SaveToFile(getint(vm, 0), getstring(vm, 1));
}

void soundbuffer_getsamples(avm *vm)
{
    word w;
    seti(&w, sfSoundBuffer_GetSamples(getint(vm, 0)));
    returnv(vm, &w);
}

void soundbuffer_getsamplescount(avm *vm)
{
    word w;
    seti(&w, sfSoundBuffer_GetSamplesCount(getint(vm, 0)));
    returnv(vm, &w);
}

void soundbuffer_getsamplesrate(avm *vm)
{
    word w;
    seti(&w, sfSoundBuffer_GetSampleRate(getint(vm, 0)));
    returnv(vm, &w);
}

void soundbuffer_getchannelscount(avm *vm)
{
    word w;
    seti(&w, sfSoundBuffer_GetChannelsCount(getint(vm, 0)));
    returnv(vm, &w);
}

void soundbuffer_getduration(avm *vm)
{
    word w;
    setf(&w, sfSoundBuffer_GetDuration(getint(vm, 0)));
    returnv(vm, &w);
}

/********************************************************************
*                 SoundBufferRecorder Functions                     *
********************************************************************/

void soundbufferrecorder_create(avm *vm)
{
    word w;
    seti(&w, sfSoundBufferRecorder_Create());
    returnv(vm, &w);
}

void soundbufferrecorder_destroy(avm *vm)
{
    sfSoundBufferRecorder_Destroy(getint(vm, 0));
}

void soundbufferrecorder_start(avm *vm)
{
    sfSoundBufferRecorder_Start(getint(vm, 0), getint(vm, 1));
}

void soundbufferrecorder_stop(avm *vm)
{
    sfSoundBufferRecorder_Stop(getint(vm, 0));
}

void soundbufferrecorder_getsamplerate(avm *vm)
{
    word w;
    seti(&w, sfSoundBufferRecorder_GetSampleRate(getint(vm, 0)));
    returnv(vm, &w);
}

void soundbufferrecorder_getbuffer(avm *vm)
{
    word w;
    seti(&w, sfSoundBufferRecorder_GetBuffer(getint(vm, 0)));
    returnv(vm, &w);
}

/********************************************************************
*                 Sound      Recorder Functions                     *
********************************************************************/

void SoundRecoder_Create(avm* vm)
{
    sfSoundRecorder_Create(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3));
}

void SoundRecorder_Destroy(avm* vm)
{
    sfSoundRecorder_Destroy(getint(vm, 0));
}

void SoundRecorder_Start(avm* vm)
{
    sfSoundRecorder_Start(getint(vm, 0), getint(vm, 1));
}

void SoundRecorder_Stop(avm* vm)
{
    sfSoundRecorder_Stop(getint(vm, 0));
}

void SoundRecorder_GetSampleRate(avm* vm)
{
    sfSoundRecorder_GetSampleRate(getint(vm, 0));
}

void SoundRecorder_CanCapture(avm* vm)
{
    word w;
    seti(&w, sfSoundRecorder_CanCapture());
    returnv(vm, &w);
}
/********************************************************************
*                       Exporting     Functions                     *
********************************************************************/

static fptrname audiolib[] =
{
    // SoundRecorder libs
    {SoundRecoder_Create, "$SoundRecoder"},
    {SoundRecorder_Destroy, "SoundRecorder_Destroy"},
    {SoundRecorder_Start, "SoundRecorder_Start"},
    {SoundRecorder_Stop, "SoundRecorder_Stop"},
    {SoundRecorder_GetSampleRate, "SoundRecorder_GetSampleRate"},
    {SoundRecorder_CanCapture, "SoundRecorder_CanCapture"},

    // SoundBufferRecorder libs
    {soundbufferrecorder_create, "$SoundBufferRecorder"},
    {soundbufferrecorder_destroy, "SoundBufferRecorder_Destroy"},
    {soundbufferrecorder_start, "SoundBufferRecorder_Start"},
    {soundbufferrecorder_stop, "SoundBufferRecorder_Stop"},
    {soundbufferrecorder_getsamplerate, "SoundBufferRecorder_GetSampleRate"},
    {soundbufferrecorder_getbuffer, "SoundBufferRecorder_GetBuffer"},

    // SoundBuffer libs
    {soundbuffer_createfromfile, "$SoundBuffer_FromFile"},
    {soundbuffer_createfrommemory, "$SoundBuffer_FromMemory"},
    {soundbuffer_destroy, "SoundBuffer_Destroy"},
    {soundbuffer_savetofile, "SoundBuffer_SaveToFile"},
    {soundbuffer_createfromsamples, "$SoundBuffer_FromSamples"},
    {soundbuffer_getsamples, "SoundBuffer_GetSamples"},
    {soundbuffer_getsamplescount, "SoundBuffer_GetSamplesCount"},
    {soundbuffer_getsamplesrate, "SoundBuffer_GetSamplesRate"},
    {soundbuffer_getchannelscount, "SoundBuffer_GetChannelsCount"},
    {soundbuffer_getduration, "SoundBuffer_GetDuration"},

    // Sound libs
    {sound_create, "$Sound"},
    {sound_destroy, "Sound_Destroy"},
    {sound_play, "Sound_Play"},
    {sound_pause, "Sound_Pause"},
    {sound_stop, "Sound_Stop"},
    {sound_setbuffer, "Sound_SetBuffer"},
    {sound_setbuffer, "Sound_GetBuffer"},
    {sound_setloop, "Sound_SetLoop"},
    {sound_getloop, "Sound_GetLoop"},
    {sound_getstatus, "Sound_GetStatus"},
    {sound_setpitch, "sound_setpitch"},
    {sound_setvolume, "sound_setvolume"},
    {sound_setposition, "Sound_SetPosition"},
    {sound_setrelativetolistener, "Sound_SetRelativeToListener"},
    {sound_setmindistance, "sound_setmindistance"},
    {sound_setattenuation, "sound_setattenuation"},
    {sound_setplayingoffset, "sound_setplayingoffset"},
    {sound_getpitch, "Sound_GetPitch"},
    {sound_getvolume, "Sound_getVolume"},
    {sound_getposition, "Sound_GetPosition"},
    {sound_isrelativetolistener, "Sound_IsRelativeToListener"},
    {sound_getmindistance, "Sound_GetMinDistance"},
    {sound_getattenuation, "Sound_GetAttenuation"},
    {sound_getplayingoffset, "Sound_GetPlayingOffset"},
    {sound_getposition, "sound_getposition"},

    // Listener Libs
    {listener_setglobalvolume, "Listener_SetGlobalVolume"},
    {listener_getglobalvolume, "Listener_GetGlobalVolume"},
    {listener_setposition, "Listener_SetPosition"},
    {listener_getposition, "Listener_GetPosition"},
    {listener_settarget, "Listener_SetTarget"},
    {listener_gettarget, "Listener_GetTarget"},

    // Music Libs
    {music_createfromfile, "$Music_FromFile"},
    {music_createfrommemory, "$Music_FromMemory"},
    {music_destroy, "Music_Destroy"},
    {music_getloop, "Music_GetLoop"},
    {music_getduration, "Music_GetDuration"},
    {music_play, "Music_Play"},
    {music_pause, "Music_Pause"},
    {music_getchannelcount, "Music_GetChannelCount"},
    {music_getsamplerate, "Music_GetSampleRate"},
    {music_getstatus, "Music_GetStatus"},
    {music_getplayingoffset, "Music_GetPlayingOffset"},
    {music_setpitch, "music_setpitch"},
    {music_setvolume, "music_setvolume"},
    {music_setposition, "music_setposition"},
    {music_setrelativetolistener, "Music_SetRelativeToListener"},
    {music_setmindistance, "music_setmindistance"},
    {music_setattenuation, "music_setattenuation"},
    {music_getpitch, "Music_GetPitch"},
    {music_getvolume, "Music_GetVolume"},
    {music_getposition, "music_getposition"},
    {music_isrelativetolistener, "Music_IsRelativeToListener"},
    {music_getmindistance, "Music_GetMinDistance"},
    {music_getattenuation, "Music_GetAttenuation"}
};

void openaudiolib()
{
	int i;
	for (i=0; i<sizeof(audiolib)/sizeof(fptrname); i++)
		zen_regfunc(audiolib[i].ptr, audiolib[i].name);
}
