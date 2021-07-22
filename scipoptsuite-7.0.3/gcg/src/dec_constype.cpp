/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program                         */
/*          GCG --- Generic Column Generation                                */
/*                  a Dantzig-Wolfe decomposition based extension            */
/*                  of the branch-cut-and-price framework                    */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/* Copyright (C) 2010-2020 Operations Research, RWTH Aachen University       */
/*                         Zuse Institute Berlin (ZIB)                       */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU Lesser General Public License        */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU Lesser General Public License for more details.                       */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.*/
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   dec_constype.cpp
 * @ingroup DETECTORS
 * @brief  detector constype (put your description here)
 * @author Michael Bastubbe
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "dec_constype.h"
#include "cons_decomp.h"
#include "class_seeed.h"
#include "class_seeedpool.h"
#include "gcg.h"
#include "scip/cons_setppc.h"
#include "scip/scip.h"
#include "scip_misc.h"
#include "scip/clock.h"

#include <iostream>

/* constraint handler properties */
#define DEC_DETECTORNAME          "constype"       /**< name of detector */
#define DEC_DESC                  "detector constype" /**< description of detector*/
#define DEC_FREQCALLROUND         1           /** frequency the detector gets called in detection loop ,ie it is called in round r if and only if minCallRound <= r <= maxCallRound AND  (r - minCallRound) mod freqCallRound == 0 */
#define DEC_MAXCALLROUND          0           /** last round the detector gets called                              */
#define DEC_MINCALLROUND          0           /** first round the detector gets called                              */
#define DEC_FREQCALLROUNDORIGINAL 1           /** frequency the detector gets called in detection loop while detecting the original problem   */
#define DEC_MAXCALLROUNDORIGINAL  0     /** last round the detector gets called while detecting the original problem                            */
#define DEC_MINCALLROUNDORIGINAL  0           /** first round the detector gets called while detecting the original problem    */
#define DEC_PRIORITY              0           /**< priority of the constraint handler for separation */
#define DEC_DECCHAR               't'         /**< display character of detector */
#define DEC_ENABLED               FALSE        /**< should the detection be enabled */
#define DEC_ENABLEDORIGINAL       FALSE  /**< should the detection of the original problem be enabled */
#define DEC_ENABLEDFINISHING      FALSE       /**< should the finishing be enabled */
#define DEC_ENABLEDPOSTPROCESSING FALSE          /**< should the finishing be enabled */
#define DEC_SKIP                  FALSE       /**< should detector be skipped if other detectors found decompositions */
#define DEC_USEFULRECALL          FALSE       /**< is it useful to call this detector on a descendant of the propagated seeed */
#define DEC_LEGACYMODE            FALSE       /**< should (old) DETECTSTRUCTURE method also be used for detection */
/*
 * Data structures
 */

/** @todo fill in the necessary detector data */

/** detector handler data */
struct DEC_DetectorData
{
};

/*
 * Local methods
 */

/* put your local methods here, and declare them static */

/** method to enumerate all subsets */
std::vector< std::vector<int> > getSubsets(std::vector<int> set)
{
    std::vector< std::vector<int> > subset;
    std::vector<int> empty;
    subset.push_back( empty );

    for ( size_t i = 0; i < set.size(); ++i )
    {
        std::vector< std::vector<int> > subsetTemp = subset;

        for (size_t j = 0; j < subsetTemp.size(); ++j)
            subsetTemp[j].push_back( set[i] );

        for (size_t j = 0; j < subsetTemp.size(); ++j)
            subset.push_back( subsetTemp[j] );
    }
    return subset;
}

/*
 * detector callback methods
 */

/** destructor of detector to free user data (called when GCG is exiting) */
#define freeConstype NULL

/** destructor of detector to free detector data (called before the solving process begins) */
#if 0
static
DEC_DECL_EXITDETECTOR(exitConstype)
{ /*lint --e{715}*/

   SCIPerrorMessage("Exit function of detector <%s> not implemented!\n", DEC_DETECTORNAME);
   SCIPABORT();

   return SCIP_OKAY;
}
#else
#define exitConstype NULL
#endif

/** detection initialization function of detector (called before solving is about to begin) */
#if 0
static
DEC_DECL_INITDETECTOR(initConstype)
{ /*lint --e{715}*/

   SCIPerrorMessage("Init function of detector <%s> not implemented!\n", DEC_DETECTORNAME);
   SCIPABORT();

   return SCIP_OKAY;
}
#else
#define initConstype NULL
#endif

/** detection function of detector */
//static DEC_DECL_DETECTSTRUCTURE(detectConstype)
//{ /*lint --e{715}*/
//   *result = SCIP_DIDNOTFIND;
//
//   SCIPerrorMessage("Detection function of detector <%s> not implemented!\n", DEC_DETECTORNAME)
//;   SCIPABORT(); /*lint --e{527}*/
//
//   return SCIP_OKAY;
//}

#define detectConstype NULL

static DEC_DECL_PROPAGATESEEED(propagateSeeedConstype)
{
  *result = SCIP_DIDNOTFIND;
  char decinfo[SCIP_MAXSTRLEN];

  SCIP_CLOCK* temporaryClock;
  SCIP_CALL_ABORT(SCIPcreateClock(scip, &temporaryClock) );
  SCIP_CALL_ABORT( SCIPstartClock(scip, temporaryClock) );

  SCIP_CONS* cons;

  int seeedCounter = 0;
  gcg::Seeed* seeedOrig;
  gcg::Seeed* seeed;

  std::vector<consType> foundConstypes(0);
  std::vector<int> constypesIndices(0);

  seeedOrig = seeedPropagationData->seeedToPropagate;



  for( int i = 0; i < seeedOrig->getNOpenconss(); ++i)
  {
      cons = seeedPropagationData->seeedpool->getConsForIndex(seeedOrig->getOpenconss()[i]);
      consType cT = GCGconsGetType(scip, cons);

      /** find constype or not */
      std::vector<consType>::const_iterator constypeIter = foundConstypes.begin();
      for(; constypeIter != foundConstypes.end(); ++constypeIter)
      {
    	  if(*constypeIter == cT)
    		  break;
      }

      if( constypeIter  == foundConstypes.end()  )
      {
         foundConstypes.push_back(GCGconsGetType(scip, cons) );
      }
  }

  for(size_t i = 0; i < foundConstypes.size(); ++i)
  {
      constypesIndices.push_back(i);
  }

  std::vector< std::vector<int> > subsetsOfConstypes = getSubsets(constypesIndices);

  SCIP_CALL( SCIPallocMemoryArray(scip, &(seeedPropagationData->newSeeeds), subsetsOfConstypes.size() - 1) );
  seeedPropagationData->nNewSeeeds = subsetsOfConstypes.size() - 1;

  for(size_t subset = 0; subset < subsetsOfConstypes.size(); ++subset)
  {
      if(subsetsOfConstypes[subset].size() == 0)
          continue;

      seeed = new gcg::Seeed(seeedOrig);
         /** set open cons that have type of the current subset to Master */
      for( int i = 0; i < seeed->getNOpenconss(); ++i)
      {
          for(size_t constypeId = 0; constypeId < subsetsOfConstypes[subset].size(); ++constypeId )
          {
              cons = seeedPropagationData->seeedpool->getConsForIndex(seeed->getOpenconss()[i]);
              if( GCGconsGetType(scip, cons) == foundConstypes[subsetsOfConstypes[subset][constypeId]] )
              {
                  seeed->bookAsMasterCons(seeed->getOpenconss()[i]);
              }
          }
      }
      seeed->flushBooked();
      (void) SCIPsnprintf(decinfo, SCIP_MAXSTRLEN, "constype");
      seeedPropagationData->newSeeeds[0]->addDetectorChainInfo(decinfo);

      seeedPropagationData->newSeeeds[seeedCounter] = seeed;
      seeedCounter++;
  }

  SCIP_CALL_ABORT( SCIPstopClock(scip, temporaryClock ) );
  for( int s = 0; s < seeedPropagationData->nNewSeeeds; ++s )
  {
     seeedPropagationData->newSeeeds[s]->addClockTime( SCIPgetClockTime( scip, temporaryClock )  );
  }
  SCIP_CALL_ABORT(SCIPfreeClock(scip, &temporaryClock) );

  *result = SCIP_SUCCESS;

   return SCIP_OKAY;
}

#define finishSeeedConstype NULL
#define detectorPostprocessSeeedConstype NULL
static
DEC_DECL_SETPARAMAGGRESSIVE(setParamAggressiveConstype)
{
   char setstr[SCIP_MAXSTRLEN];
   const char* name = DECdetectorGetName(detector);

   (void) SCIPsnprintf(setstr, SCIP_MAXSTRLEN, "detection/detectors/%s/enabled", name);
   SCIP_CALL( SCIPsetBoolParam(scip, setstr, FALSE) );

   (void) SCIPsnprintf(setstr, SCIP_MAXSTRLEN, "detection/detectors/%s/origenabled", name);
   SCIP_CALL( SCIPsetBoolParam(scip, setstr, FALSE) );

   (void) SCIPsnprintf(setstr, SCIP_MAXSTRLEN, "detection/detectors/%s/finishingenabled", name);
   SCIP_CALL( SCIPsetBoolParam(scip, setstr, FALSE ) );

   return SCIP_OKAY;

}


static
DEC_DECL_SETPARAMDEFAULT(setParamDefaultConstype)
{

   return SCIP_OKAY;

}

static
DEC_DECL_SETPARAMFAST(setParamFastConstype)
{
   char setstr[SCIP_MAXSTRLEN];

   const char* name = DECdetectorGetName(detector);

   (void) SCIPsnprintf(setstr, SCIP_MAXSTRLEN, "detection/detectors/%s/enabled", name);
   SCIP_CALL( SCIPsetBoolParam(scip, setstr, FALSE) );

   (void) SCIPsnprintf(setstr, SCIP_MAXSTRLEN, "detection/detectors/%s/origenabled", name);
   SCIP_CALL( SCIPsetBoolParam(scip, setstr, FALSE) );

   (void) SCIPsnprintf(setstr, SCIP_MAXSTRLEN, "detection/detectors/%s/finishingenabled", name);
   SCIP_CALL( SCIPsetBoolParam(scip, setstr, FALSE ) );


   return SCIP_OKAY;

}



/*
 * detector specific interface methods
 */

/** creates the handler for constype detector and includes it in SCIP */
SCIP_RETCODE SCIPincludeDetectorConstype(SCIP* scip /**< SCIP data structure */
)
{
   DEC_DETECTORDATA* detectordata;

   /**@todo create constype detector data here*/
   detectordata = NULL;

   SCIP_CALL(
      DECincludeDetector(scip, DEC_DETECTORNAME, DEC_DECCHAR, DEC_DESC, DEC_FREQCALLROUND, DEC_MAXCALLROUND,
         DEC_MINCALLROUND, DEC_FREQCALLROUNDORIGINAL, DEC_MAXCALLROUNDORIGINAL, DEC_MINCALLROUNDORIGINAL, DEC_PRIORITY, DEC_ENABLED, DEC_ENABLEDORIGINAL, DEC_ENABLEDFINISHING, DEC_ENABLEDPOSTPROCESSING, DEC_SKIP, DEC_USEFULRECALL, DEC_LEGACYMODE, detectordata, detectConstype,
         freeConstype, initConstype, exitConstype, propagateSeeedConstype, NULL, NULL, finishSeeedConstype, detectorPostprocessSeeedConstype, setParamAggressiveConstype, setParamDefaultConstype, setParamFastConstype));

   /**@todo add constype detector parameters */

   return SCIP_OKAY;
}
