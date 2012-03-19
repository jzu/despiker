/****************************************************************************
 * Despiker
 *
 * Copyright (C) Jean Zundel <jzu@free.fr> 2012 
 *
 * Removes crackles provoked by signal spikes, for example snare overloads,
 * by selectively applying a low-pass filter to half-waves reaching a given
 * treshold using a convolution matrix - leaving other parts untouched.
 *
 * Plugin structure inherited from amp.c:
 * This file has poor memory protection. Failures during malloc() will
 * not recover nicely. 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "ladspa.h"


typedef struct {
  LADSPA_Data * m_pfInputBuffer1;
  LADSPA_Data * m_pfOutputBuffer1;
} Despiker;

#define DESPIKER_INPUT1  0
#define DESPIKER_OUTPUT1 1


/* Yeuch */

#ifdef WIN32
#define _WINDOWS_DLL_EXPORT_ __declspec(dllexport)
int bIsFirstTime = 1; 
void _init(); // forward declaration
#else
#define _WINDOWS_DLL_EXPORT_ 
#endif


/* Plugin-specific defines */

#define TRESHOLD 0.9
#define HLF_CNVL 10
#define LEN_CNVL (2*HLF_CNVL+1)        /* 21 */


/* Convolution matrix */

LADSPA_Data matrix [LEN_CNVL] = {      /* Mean should be 1 */
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
  8, 
  1, 1, 1, 1, 1, 1, 0, 0, 0, 0 
};

LADSPA_Data *cnvl = matrix + HLF_CNVL + 1;


/*****************************************************************************
 * Apply a convolution matrix to all points in the marked area 
 *****************************************************************************/

void convolve (LADSPA_Data  *in,          // Input buffer
               LADSPA_Data  *out,         // Output buffer
               unsigned long start,
               unsigned long stop,
               unsigned long smplcnt) {

  unsigned long i;
  long c;

  LADSPA_Data sum;

  if ((start - HLF_CNVL < 0) ||           // Fast, clean, cheap: if the very
      (stop + HLF_CNVL + 1 > smplcnt))    // first/last samples of the track
    for (i = start ; i < stop ; i++)      // are clipping, zero out this part
      out [i] = 0;
  else
    for (i = start; i < stop ; i++) {
      sum = 0;
      for (c = -HLF_CNVL ; c <= HLF_CNVL; c++)
        sum += in [i+c] * cnvl [c];
      out [i] = sum / LEN_CNVL;
    }
}


/*****************************************************************************
 * Copy input to output, scan half-waves for spikes, call convolve()
 *****************************************************************************/

void runDespiker (LADSPA_Handle Instance,
                  unsigned long SampleCount) {
  

  LADSPA_Data  *in;
  LADSPA_Data  *out;
  Despiker     *psDespiker;
  unsigned long i;

  unsigned long wave_start = 0;
  int           clipping   = 0;

  psDespiker = (Despiker *) Instance;

  in  = psDespiker->m_pfInputBuffer1;
  out = psDespiker->m_pfOutputBuffer1;

  for (i = 0; i < SampleCount; i++) {
    out [i] = in [i];
    if ((clipping) && 
        (in [i] * in [i+1] < 0)) {        // Rewrite former half-wave
      convolve (in,              
                out, 
                wave_start, 
                i, 
                SampleCount);
      clipping = 0;
      continue;
    }
    if (fabs (in [i]) >= TRESHOLD) {      // Uh oh
      clipping = 1;
      continue;
    }
    if ((!clipping) && 
        (in [i] * in [i+1] < 0)) {        // Keep track of wave crossing 0
      wave_start = i;
    }
  }
}


/*****************************************************************************/

LADSPA_Handle instantiateDespiker (const LADSPA_Descriptor * Descriptor,
                                   unsigned long             SampleRate) {
  return malloc (sizeof (Despiker));
}

/*****************************************************************************/

void connectPortToDespiker (LADSPA_Handle Instance,
                            unsigned long Port,
                            LADSPA_Data * DataLocation) {

  Despiker * psDespiker;

  psDespiker = (Despiker *) Instance;
  switch (Port) {
    case DESPIKER_INPUT1:
      psDespiker->m_pfInputBuffer1 = DataLocation;
      break;
    case DESPIKER_OUTPUT1:
      psDespiker->m_pfOutputBuffer1 = DataLocation;
      break;
  }
}

/*****************************************************************************/

void cleanupDespiker (LADSPA_Handle Instance) {

  free (Instance);
}

/*****************************************************************************/

LADSPA_Descriptor * g_psDespikerDescr = NULL;

/*****************************************************************************/
/* _init() is called automatically when the plugin library is first
   loaded. */

void _init() {

  char ** pcPortNames;
  LADSPA_PortDescriptor * piPortDescriptors;
  LADSPA_PortRangeHint * psPortRangeHints;

  g_psDespikerDescr
    = (LADSPA_Descriptor *) malloc (sizeof (LADSPA_Descriptor));

  if (g_psDespikerDescr) {
  
    g_psDespikerDescr->UniqueID         = 4741;
    g_psDespikerDescr->Label            = strdup ("Despiker");
    g_psDespikerDescr->Properties
      = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    g_psDespikerDescr->Name             = strdup ("Despiker");
    g_psDespikerDescr->Maker            = strdup ("Jean Zundel");
    g_psDespikerDescr->Copyright        = strdup ("GPL v3");
    g_psDespikerDescr->PortCount        = 2;
    piPortDescriptors
      = (LADSPA_PortDescriptor *) calloc (2, sizeof (LADSPA_PortDescriptor));
    g_psDespikerDescr->PortDescriptors
      = (const LADSPA_PortDescriptor *) piPortDescriptors;
    piPortDescriptors [DESPIKER_INPUT1]
      = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors [DESPIKER_OUTPUT1]
      = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    pcPortNames
      = (char **) calloc (2, sizeof(char *));
    g_psDespikerDescr->PortNames 
      = (const char **) pcPortNames;
    pcPortNames [DESPIKER_INPUT1]  = strdup ("Input");
    pcPortNames [DESPIKER_OUTPUT1] = strdup ("Output");
    psPortRangeHints 
      = ((LADSPA_PortRangeHint *) calloc (2, sizeof (LADSPA_PortRangeHint)));
    g_psDespikerDescr->PortRangeHints
      = (const LADSPA_PortRangeHint *) psPortRangeHints;
    psPortRangeHints [DESPIKER_INPUT1].HintDescriptor  = 0;
    psPortRangeHints [DESPIKER_OUTPUT1].HintDescriptor = 0;
    g_psDespikerDescr->instantiate  = instantiateDespiker;
    g_psDespikerDescr->connect_port = connectPortToDespiker;
    g_psDespikerDescr->activate     = NULL;
    g_psDespikerDescr->run          = runDespiker;
    g_psDespikerDescr->run_adding   = NULL;
    g_psDespikerDescr->deactivate   = NULL;
    g_psDespikerDescr->cleanup      = cleanupDespiker;
  }
}

/*****************************************************************************/

void deleteDescriptor (LADSPA_Descriptor * psDescriptor) {

  unsigned long lIndex;

  if (psDescriptor) {
    free ((char *)psDescriptor->Label);
    free ((char *)psDescriptor->Name);
    free ((char *)psDescriptor->Maker);
    free ((char *)psDescriptor->Copyright);
    free ((LADSPA_PortDescriptor *)psDescriptor->PortDescriptors);
    for (lIndex = 0; lIndex < psDescriptor->PortCount; lIndex++)
      free ((char *)(psDescriptor->PortNames[lIndex]));
    free ((char **)psDescriptor->PortNames);
    free ((LADSPA_PortRangeHint *)psDescriptor->PortRangeHints);
    free (psDescriptor);
  }
}

/*****************************************************************************/
/* _fini() is called automatically when the library is unloaded. */

void _fini() {

  deleteDescriptor (g_psDespikerDescr);
}

/*****************************************************************************/

_WINDOWS_DLL_EXPORT_
const LADSPA_Descriptor *ladspa_descriptor (unsigned long Index) {

#ifdef WIN32
  if (bIsFirstTime) {
    _init();
    bIsFirstTime = 0;
  }
#endif

  if (Index == 0)
    return g_psDespikerDescr;
  else
    return NULL;
}


