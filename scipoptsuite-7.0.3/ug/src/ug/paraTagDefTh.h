/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*             This file is part of the program and software framework       */
/*                  UG --- Ubquity Generator Framework                       */
/*                                                                           */
/*    Copyright (C) 2002-2020 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  UG is distributed under the terms of the ZIB Academic Licence.           */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with UG; see the file COPYING. If not email to scip@zib.de.        */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file    paraTagDefPth.h
 * @brief   Tag definitions for Pthreads communication.
 * @author  Yuji Shinano
 *
 *
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/


#ifndef __PARA_TAG_DEF_TH_H__
#define __PARA_TAG_DEF_TH_H__
#include "paraTagDef.h"


namespace UG
{
static const int TAG_TH_FIRST            = TAG_LAST + 1;
//----------------------------------------------------------------
static const int TagParaInstance          = TAG_TH_FIRST + 0;
static const int TagSolverDiffParamSet    = TAG_TH_FIRST + 1;
//---------------------------------------------------------------
static const int TAG_TH_LAST             = TAG_TH_FIRST + 1;
static const int N_TH_TAGS               = TAG_TH_LAST - TAG_FIRST + 1;
}

#endif // __PARA_TAG_DEF_TH_H__
