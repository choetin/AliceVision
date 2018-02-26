// This file is part of the AliceVision project.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <aliceVision/structures/Rgb.hpp>
#include <aliceVision/structures/StaticVector.hpp>
#include <aliceVision/common/ImagesCache.hpp>
#include <aliceVision/common/PreMatchCams.hpp>
#include <aliceVision/depthMap/DepthSimMap.hpp>
#include <aliceVision/depthMap/RcTc.hpp>
#include <aliceVision/depthMap/cuda/PlaneSweepingCuda.hpp>

namespace aliceVision {
namespace depthMap {

class SemiGlobalMatchingParams
{
public:
    common::MultiViewParams* mp;
    common::PreMatchCams* pc;
    RcTc* prt;
    PlaneSweepingCuda* cps;
    common::ImagesCache* ic;
    bool visualizeDepthMaps;
    bool visualizePartialDepthMaps;
    bool doSmooth;
    // int   s_wsh;
    // float s_gammaC;
    // float s_gammaP;
    // int   wsh;
    // float gammaC;
    // float gammaP;
    bool doRefine;
    int ndepthsToRefine;
    unsigned char P1;
    unsigned char P2;
    unsigned char P3;
    int maxDepthsToStore;
    int maxDepthsToSweep;
    int rcTcDepthsHalfLimit;
    int rcDepthsCompStep;
    bool refineUseTcOrPixSize;
    bool useSeedsToCompDepthsToSweep;
    float seedsRangePercentile;
    float seedsRangeInflate;
    bool saveDepthsToSweepToTxtForVis;
    int modalsMapDistLimit;
    int minNumOfConsistentCams;
    int minObjectThickness;
    float maxTcRcPixSizeInVoxRatio;
    int nSGGCIters;
    bool doSGMoptimizeVolume;
    bool doRefineRc;
    std::string SGMoutDirName;
    std::string SGMtmpDirName;
    bool useSilhouetteMaskCodedByColor;
    rgb silhouetteMaskColor;

    SemiGlobalMatchingParams(common::MultiViewParams* _mp, common::PreMatchCams* _pc, PlaneSweepingCuda* _cps);
    ~SemiGlobalMatchingParams(void);

    DepthSimMap* getDepthSimMapFromBestIdVal(int w, int h, StaticVector<IdValue>* volumeBestIdVal, int scale,
                                                int step, int rc, int zborder, StaticVector<float>* planesDepths);

    std::string getREFINE_photo_depthMapFileName(IndexT viewId, int scale, int step);
    std::string getREFINE_photo_simMapFileName(IndexT viewId, int scale, int step);
    std::string getREFINE_opt_depthMapFileName(IndexT viewId, int scale, int step);
    std::string getREFINE_opt_simMapFileName(IndexT viewId, int scale, int step);

    std::string getSGMTmpDir();
    std::string getSGM_idDepthMapFileName(IndexT viewId, int scale, int step);
    std::string getSGM_depthMapFileName(IndexT viewId, int scale, int step);
    std::string getSGM_simMapFileName(IndexT viewId, int scale, int step);
    std::string getSGM_tcamsFileName(IndexT viewId);
    std::string getSGM_depthsFileName(IndexT viewId);

};

} // namespace depthMap
} // namespace aliceVision