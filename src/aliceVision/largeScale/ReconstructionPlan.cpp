// This file is part of the AliceVision project.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ReconstructionPlan.hpp"
#include <aliceVision/structures/Rgb.hpp>
#include <aliceVision/common/common.hpp>
#include <aliceVision/common/fileIO.hpp>
#include <aliceVision/delaunayCut/DelaunayGraphCut.hpp>
#include <aliceVision/delaunayCut/meshPostProcessing.hpp>
#include <aliceVision/largeScale/VoxelsGrid.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

ReconstructionPlan::ReconstructionPlan(Voxel& dimmensions, Point3d* space, MultiViewParams* _mp, PreMatchCams* _pc,
                                       std::string _spaceRootDir)
    : VoxelsGrid(dimmensions, space, _mp, _pc, _spaceRootDir)
{
    nVoxelsTracks = getNVoxelsTracks();
}

ReconstructionPlan::~ReconstructionPlan()
{
    delete nVoxelsTracks;
}

StaticVector<int>* ReconstructionPlan::voxelsIdsIntersectingHexah(Point3d* hexah)
{
    StaticVector<int>* ids = new StaticVector<int>(voxels->size() / 8);

    for(int i = 0; i < voxels->size() / 8; i++)
    {
        if(intersectsHexahedronHexahedron(&(*voxels)[i * 8], hexah))
        {
            ids->push_back(i);
        }
    }

    return ids;
}

unsigned long ReconstructionPlan::getNTracks(const Voxel& LU, const Voxel& RD)
{
    unsigned long n = 0;
    Voxel v;
    for(v.x = LU.x; v.x <= RD.x; v.x++)
    {
        for(v.y = LU.y; v.y <= RD.y; v.y++)
        {
            for(v.z = LU.z; v.z <= RD.z; v.z++)
            {
                n += (*nVoxelsTracks)[getIdForVoxel(v)];
            }
        }
    }
    return n;
}

bool ReconstructionPlan::divideBox(Voxel& LU1o, Voxel& RD1o, Voxel& LU2o, Voxel& RD2o, const Voxel& LUi,
                                   const Voxel& RDi, unsigned long maxTracks)
{
    unsigned long n = getNTracks(LUi, RDi);
    if(n < maxTracks)
    {
        return false;
    }

    Voxel LUt, RDt;

    unsigned long nx1, nx2, ny1, ny2, nz1, nz2;
    nx1 = 0;
    nx2 = 0;
    ny1 = 0;
    ny2 = 0;
    nz1 = 0;
    nz2 = 0;

    if(RDi.x - LUi.x > 0)
    {
        LUt = LUi;
        RDt = RDi;
        RDt.x = LUi.x + (RDi.x - LUi.x) / 2;
        nx1 = getNTracks(LUt, RDt);
        LUt = LUi;
        RDt = RDi;
        LUt.x = LUi.x + (RDi.x - LUi.x) / 2 + 1;
        nx2 = getNTracks(LUt, RDt);
    }

    if(RDi.y - LUi.y > 0)
    {
        LUt = LUi;
        RDt = RDi;
        RDt.y = LUi.y + (RDi.y - LUi.y) / 2;
        ny1 = getNTracks(LUt, RDt);
        LUt = LUi;
        RDt = RDi;
        LUt.y = LUi.y + (RDi.y - LUi.y) / 2 + 1;
        ny2 = getNTracks(LUt, RDt);
    }

    if(RDi.z - LUi.z > 0)
    {
        LUt = LUi;
        RDt = RDi;
        RDt.z = LUi.z + (RDi.z - LUi.z) / 2;
        nz1 = getNTracks(LUt, RDt);
        LUt = LUi;
        RDt = RDi;
        LUt.z = LUi.z + (RDi.z - LUi.z) / 2 + 1;
        nz2 = getNTracks(LUt, RDt);
    }

    if(RDi.x - LUi.x > 0)
    {
        if((abs(RDi.x - LUi.x) >= abs(RDi.y - LUi.y)) && (abs(RDi.x - LUi.x) >= abs(RDi.z - LUi.z)))
        {
            LU1o = LUi;
            RD1o = RDi;
            RD1o.x = LUi.x + (RDi.x - LUi.x) / 2;
            LU2o = LUi;
            RD2o = RDi;
            LU2o.x = LUi.x + (RDi.x - LUi.x) / 2 + 1;
            return true;
        }
    }

    if(RDi.y - LUi.y > 0)
    {
        if((abs(RDi.y - LUi.y) >= abs(RDi.x - LUi.x)) && (abs(RDi.y - LUi.y) >= abs(RDi.z - LUi.z)))
        {
            LU1o = LUi;
            RD1o = RDi;
            RD1o.y = LUi.y + (RDi.y - LUi.y) / 2;
            LU2o = LUi;
            RD2o = RDi;
            LU2o.y = LUi.y + (RDi.y - LUi.y) / 2 + 1;
            return true;
        }
    }

    if(RDi.z - LUi.z > 0)
    {
        if((abs(RDi.z - LUi.z) >= abs(RDi.x - LUi.x)) && (abs(RDi.z - LUi.z) >= abs(RDi.y - LUi.y)))
        {
            LU1o = LUi;
            RD1o = RDi;
            RD1o.z = LUi.z + (RDi.z - LUi.z) / 2;
            LU2o = LUi;
            RD2o = RDi;
            LU2o.z = LUi.z + (RDi.z - LUi.z) / 2 + 1;
            return true;
        }
    }

    printf("WARNING should not happen\n");

    return false;
}

StaticVector<Point3d>* ReconstructionPlan::computeReconstructionPlanBinSearch(unsigned long maxTracks)
{
    Voxel actHexahLU = Voxel(0, 0, 0);
    Voxel actHexahRD = voxelDim - Voxel(1, 1, 1);

    /*
    printf("------------\n");
    printf("actHexahLU %i %i %i\n",actHexahLU.x,actHexahLU.y,actHexahLU.z);
    printf("actHexahRD %i %i %i\n",actHexahRD.x,actHexahRD.y,actHexahRD.z);
    */

    StaticVector<Point3d>* hexahsToReconstruct = new StaticVector<Point3d>(nVoxelsTracks->size() * 8);

    StaticVector<Voxel>* toDivideLU = new StaticVector<Voxel>(10 * nVoxelsTracks->size());
    StaticVector<Voxel>* toDivideRD = new StaticVector<Voxel>(10 * nVoxelsTracks->size());
    toDivideLU->push_back(actHexahLU);
    toDivideRD->push_back(actHexahRD);

    while(toDivideLU->size() > 0)
    {
        actHexahLU = toDivideLU->pop();
        actHexahRD = toDivideRD->pop();

        /*
        printf("------------\n");
        printf("actHexahLU %i %i %i\n",actHexahLU.x,actHexahLU.y,actHexahLU.z);
        printf("actHexahRD %i %i %i\n",actHexahRD.x,actHexahRD.y,actHexahRD.z);
        */

        Voxel LU1, RD1, LU2, RD2;
        if(divideBox(LU1, RD1, LU2, RD2, actHexahLU, actHexahRD, maxTracks))
        {
            toDivideLU->push_back(LU1);
            toDivideRD->push_back(RD1);
            toDivideLU->push_back(LU2);
            toDivideRD->push_back(RD2);

            /*
            printf("------------\n");
            printf("actHexahLU %i %i %i\n",actHexahLU.x,actHexahLU.y,actHexahLU.z);
            printf("actHexahRD %i %i %i\n",actHexahRD.x,actHexahRD.y,actHexahRD.z);
            printf("LU1 %i %i %i\n",LU1.x,LU1.y,LU1.z);
            printf("RD1 %i %i %i\n",RD1.x,RD1.y,RD1.z);
            printf("LU2 %i %i %i\n",LU2.x,LU2.y,LU2.z);
            printf("RD2 %i %i %i\n",RD2.x,RD2.y,RD2.z);
            */
        }
        else
        {
            Point3d hexah[8], hexahinf[8];

            /*
            printf("------------\n");
            printf("actHexahLU %i %i %i\n",actHexahLU.x,actHexahLU.y,actHexahLU.z);
            printf("actHexahRD %i %i %i\n",actHexahRD.x,actHexahRD.y,actHexahRD.z);
            */

            getHexah(hexah, actHexahLU, actHexahRD);
            inflateHexahedron(hexah, hexahinf, 1.05);
            for(int k = 0; k < 8; k++)
            {
                hexahsToReconstruct->push_back(hexahinf[k]);
            }
        }
    }

    delete toDivideLU;
    delete toDivideRD;

    return hexahsToReconstruct;
}

void ReconstructionPlan::getHexahedronForID(float dist, int id, Point3d* out)
{
    inflateHexahedron(&(*voxels)[id * 8], out, dist);
}

void reconstructSpaceAccordingToVoxelsArray(const std::string& voxelsArrayFileName, LargeScale* ls,
                                            bool doComputeColoredMeshes)
{
    StaticVector<Point3d>* voxelsArray = loadArrayFromFile<Point3d>(voxelsArrayFileName);

    ReconstructionPlan* rp =
        new ReconstructionPlan(ls->dimensions, &ls->space[0], ls->mp, ls->pc, ls->spaceVoxelsFolderName);

    StaticVector<Point3d>* hexahsToExcludeFromResultingMesh = new StaticVector<Point3d>(voxelsArray->size());
    for(int i = 0; i < voxelsArray->size() / 8; i++)
    {
        printf("RECONSTRUCTING %i-th VOXEL OF %i \n", i, voxelsArray->size() / 8);

        const std::string folderName = ls->getReconstructionVoxelFolder(i);
        bfs::create_directory(folderName);

        const std::string meshBinFilepath = folderName + "mesh.bin";
        if(!FileExists(meshBinFilepath))
        {
            StaticVector<int>* voxelsIds = rp->voxelsIdsIntersectingHexah(&(*voxelsArray)[i * 8]);
            DelaunayGraphCut delaunayGC(ls->mp, ls->pc);
            Point3d* hexah = &(*voxelsArray)[i * 8];
            delaunayGC.reconstructVoxel(hexah, voxelsIds, folderName, ls->getSpaceCamsTracksDir(), false,
                                  hexahsToExcludeFromResultingMesh, (VoxelsGrid*)rp, ls->getSpaceSteps());
            delete voxelsIds;

            // Save mesh as .bin and .obj
            Mesh* mesh = delaunayGC.createMesh();
            StaticVector<StaticVector<int>*>* ptsCams = delaunayGC.createPtsCams();
            StaticVector<int> usedCams = delaunayGC.getSortedUsedCams();

            meshPostProcessing(mesh, ptsCams, usedCams, *ls->mp, *ls->pc, ls->mp->mip->mvDir, hexahsToExcludeFromResultingMesh, hexah);
            mesh->saveToBin(folderName + "mesh.bin");
            mesh->saveToObj(folderName + "mesh.obj");

            saveArrayOfArraysToFile<int>(folderName + "meshPtsCamsFromDGC.bin", ptsCams);
            deleteArrayOfArrays<int>(&ptsCams);

            delete mesh;
        }

        /*
        if(doComputeColoredMeshes)
        {
            std::string resultFolderName = folderName + "/";
            computeColoredMesh(resultFolderName, ls);
        }
        */

        Point3d hexahThin[8];
        inflateHexahedron(&(*voxelsArray)[i * 8], hexahThin, 0.9);
        for(int k = 0; k < 8; k++)
        {
            hexahsToExcludeFromResultingMesh->push_back(hexahThin[k]);
        }
    }
    delete hexahsToExcludeFromResultingMesh;

    delete rp;
    delete voxelsArray;
}


StaticVector<StaticVector<int>*>* loadLargeScalePtsCams(const std::vector<std::string>& recsDirs)
{
    StaticVector<StaticVector<int>*>* ptsCamsFromDct = new StaticVector<StaticVector<int>*>();
    for(int i = 0; i < recsDirs.size(); ++i)
    {
        std::string folderName = recsDirs[i];

        std::string filePtsCamsFromDCTName = folderName + "meshPtsCamsFromDGC.bin";
        if(!FileExists(filePtsCamsFromDCTName))
            throw std::runtime_error("Missing file: " + filePtsCamsFromDCTName);
        StaticVector<StaticVector<int>*>* ptsCamsFromDcti = loadArrayOfArraysFromFile<int>(filePtsCamsFromDCTName);
        ptsCamsFromDct->resizeAdd(ptsCamsFromDcti->size());
        for(int i = 0; i < ptsCamsFromDcti->size(); i++)
        {
            ptsCamsFromDct->push_back((*ptsCamsFromDcti)[i]);
        }
        delete ptsCamsFromDcti; //!!!NOT DELETE ARRAYOFARRAYS
    }
    return ptsCamsFromDct;
}

StaticVector<rgb>* getTrisColorsRgb(Mesh* me, StaticVector<rgb>* ptsColors)
{
    StaticVector<rgb>* trisColors = new StaticVector<rgb>(me->tris->size());
    trisColors->resize(me->tris->size());
    for(int i = 0; i < me->tris->size(); i++)
    {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        for(int j = 0; j < 3; j++)
        {
            r += (float)(*ptsColors)[(*me->tris)[i].i[j]].r;
            g += (float)(*ptsColors)[(*me->tris)[i].i[j]].g;
            b += (float)(*ptsColors)[(*me->tris)[i].i[j]].b;
        }
        (*trisColors)[i].r = (unsigned char)(r / 3.0f);
        (*trisColors)[i].g = (unsigned char)(g / 3.0f);
        (*trisColors)[i].b = (unsigned char)(b / 3.0f);
    }
    return trisColors;
}

Mesh* joinMeshes(const std::vector<std::string>& recsDirs, StaticVector<Point3d>* voxelsArray,
                    LargeScale* ls)
{
    ReconstructionPlan rp(ls->dimensions, &ls->space[0], ls->mp, ls->pc, ls->spaceVoxelsFolderName);

    if(ls->mp->verbose)
        printf("Detecting size of merged mesh\n");
    int npts = 0;
    int ntris = 0;
    for(int i = 0; i < recsDirs.size(); i++)
    {
        std::string folderName = recsDirs[i];

        std::string fileName = folderName + "mesh.bin";
        if(FileExists(fileName))
        {
            Mesh* mei = new Mesh();
            mei->loadFromBin(fileName);
            npts += mei->pts->size();
            ntris += mei->tris->size();

            printf("npts %i %i \n", npts, mei->pts->size());
            printf("ntris %i %i \n", ntris, mei->tris->size());

            delete mei;
        }
    }

    if(ls->mp->verbose)
        printf("Creating mesh\n");
    Mesh* me = new Mesh();
    me->pts = new StaticVector<Point3d>(npts);
    me->tris = new StaticVector<Mesh::triangle>(ntris);

    StaticVector<rgb>* trisCols = new StaticVector<rgb>(ntris);
    StaticVector<rgb>* ptsCols = new StaticVector<rgb>(npts);

    if(ls->mp->verbose)
        printf("Merging part to one mesh (not connecting them!!!)\n");
    for(int i = 0; i < recsDirs.size(); i++)
    {
        if(ls->mp->verbose)
            printf("Merging part %i\n", i);
        std::string folderName = recsDirs[i];

        std::string fileName = folderName + "mesh.bin";
        if(FileExists(fileName))
        {
            Mesh* mei = new Mesh();
            mei->loadFromBin(fileName);

            // to remove artefacts on the border
            Point3d hexah[8];
            float inflateFactor = 0.96;
            inflateHexahedron(&(*voxelsArray)[i * 8], hexah, inflateFactor);
            mei->removeTrianglesOutsideHexahedron(hexah);

            if(ls->mp->verbose)
                printf("Adding mesh part %i to mesh\n", i);
            me->addMesh(mei);

            if(ls->mp->verbose)
                printf("Merging colors of part %i\n", i);
            fileName = folderName + "meshAvImgCol.ply.ptsColors";
            if(FileExists(fileName))
            {
                StaticVector<rgb>* ptsColsi = loadArrayFromFile<rgb>(fileName);
                StaticVector<rgb>* trisColsi = getTrisColorsRgb(mei, ptsColsi);

                for(int j = 0; j < trisColsi->size(); j++)
                {
                    trisCols->push_back((*trisColsi)[j]);
                }
                for(int j = 0; j < ptsColsi->size(); j++)
                {
                    ptsCols->push_back((*ptsColsi)[j]);
                }
                delete ptsColsi;
                delete trisColsi;
            }

            delete mei;
        }
    }

    // int gridLevel = ls->mp->mip->_ini.get<int>("LargeScale.gridLevel0", 0);

    if(ls->mp->verbose)
        printf("Deleting\n");
    delete ptsCols;
    delete trisCols;

    //if(ls->mp->verbose)
    //    printf("Creating QS\n");
    // createQSfileFromMesh(ls->spaceFolderName + "reconstructedSpaceLevelJoinedMesh"+num2str(gridLevel)+".bin",
    // ls->spaceFolderName+"meshAvImgCol.ply.ptsColors", ls->spaceFolderName +
    // "reconstructedSpaceLevelJoinedMesh"+num2str(gridLevel)+".qs");
#ifdef QSPLAT
    createQSfileFromMesh(spaceBinFileName, spacePtsColsBinFileName, outDir + "mesh.qs");
#endif

    return me;
}

Mesh* joinMeshes(int gl, LargeScale* ls)
{
    ReconstructionPlan* rp =
        new ReconstructionPlan(ls->dimensions, &ls->space[0], ls->mp, ls->pc, ls->spaceVoxelsFolderName);
    std::string param = "LargeScale:gridLevel" + num2str(gl);
    int gridLevel = ls->mp->mip->_ini.get<int>(param.c_str(), gl * 300);

    std::string optimalReconstructionPlanFileName =
        ls->spaceFolderName + "optimalReconstructionPlan" + num2str(gridLevel) + ".bin";
    StaticVector<SortedId>* optimalReconstructionPlan = loadArrayFromFile<SortedId>(optimalReconstructionPlanFileName);

    auto subFolderName = ls->mp->mip->_ini.get<std::string>("LargeScale.subFolderName", "");
    if(subFolderName.empty())
    {
        if(ls->mp->mip->_ini.get<bool>("global.LabatutCFG09", false))
        {
            subFolderName = "LabatutCFG09";
        }
        if(ls->mp->mip->_ini.get<bool>("global.JancosekCVPR11", true))
        {
            subFolderName = "JancosekCVPR11";
        }
    }
    subFolderName = subFolderName + "/";

    StaticVector<Point3d>* voxelsArray = new StaticVector<Point3d>(optimalReconstructionPlan->size() * 8);
    std::vector<std::string> recsDirs;
    for(int i = 0; i < optimalReconstructionPlan->size(); i++)
    {
        int id = (*optimalReconstructionPlan)[i].id;
        float inflateFactor = (*optimalReconstructionPlan)[i].value;
        std::string folderName = ls->spaceFolderName + "reconstructedSpacePart" + num2strFourDecimal(id) + "/";
        folderName +=  "GL_" + num2str(gridLevel) + "_IF_" + num2str((int)inflateFactor) + "/";

        Point3d hexah[8];
        rp->getHexahedronForID(inflateFactor, id, hexah);
        for(int k = 0; k < 8; k++)
        {
            voxelsArray->push_back(hexah[k]);
        }

        recsDirs.push_back(folderName);
    }
    delete optimalReconstructionPlan;
    delete rp;

    Mesh* me = joinMeshes(recsDirs, voxelsArray, ls);
    delete voxelsArray;

    return me;
}

Mesh* joinMeshes(const std::string& voxelsArrayFileName, LargeScale* ls)
{
    StaticVector<Point3d>* voxelsArray = loadArrayFromFile<Point3d>(voxelsArrayFileName);
    std::vector<std::string> recsDirs = ls->getRecsDirs(voxelsArray);

    Mesh* me = joinMeshes(recsDirs, voxelsArray, ls);
    delete voxelsArray;

    return me;
}

