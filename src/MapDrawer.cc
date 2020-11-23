/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/

#include "MapDrawer.h"
#include "MapPoint.h"
#include "KeyFrame.h"
#include <pangolin/pangolin.h>
#include <mutex>

namespace ORB_SLAM2
{


MapDrawer::MapDrawer(Map* pMap, const string &strSettingPath):mpMap(pMap)
{
    
    cv::FileStorage fSettings(strSettingPath, cv::FileStorage::READ);

    mKeyFrameSize = fSettings["Viewer.KeyFrameSize"];
    mKeyFrameLineWidth = fSettings["Viewer.KeyFrameLineWidth"];
    mGraphLineWidth = fSettings["Viewer.GraphLineWidth"];
    mPointSize = fSettings["Viewer.PointSize"];
    mCameraSize = fSettings["Viewer.CameraSize"];
    mCameraLineWidth = fSettings["Viewer.CameraLineWidth"];

}

void MapDrawer::DrawMapPoints()
{
    
    const vector<MapPoint*> &vpMPs = mpMap->GetAllMapPoints();
    const vector<MapPoint*> &vpRefMPs = mpMap->GetReferenceMapPoints();

    set<MapPoint*> spRefMPs(vpRefMPs.begin(), vpRefMPs.end());
    if(vpMPs.empty())
        return;

#ifdef HAVE_GLES_2

    float color[] = {0.0f, 0.0f, 0.0f, 1.0f};  //white   set a_color,  this can not modify color
    std::vector<float> vec_points;
    float points1_[] = {}; 
    uint length = 0;
    for(size_t i=0, iend=vpMPs.size(); i<iend;i++)
    {
        if(vpMPs[i]->isBad() || spRefMPs.count(vpMPs[i]))
            continue;
        cv::Mat pos = vpMPs[i]->GetWorldPos();
        vec_points.push_back(pos.at<float>(0));
        vec_points.push_back(pos.at<float>(1));
        vec_points.push_back(pos.at<float>(2));
    }

    if (vec_points.size() > 0) {
        glVertexPointer(3, GL_FLOAT, 0, vec_points.data());
        length = vec_points.size() / 3;
    }
    else {
        glVertexPointer(3, GL_FLOAT, 0, points1_);
        length = 0;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    pangolin::glEngine().SetColor(0.0,1.0,0.0,1.0);   // set color     green

    glColorPointer(4, GL_FLOAT, 0, color);
    glDrawArrays(GL_POINTS,0,length);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
#else
    glPointSize(mPointSize);
    glBegin(GL_POINTS);
    glColor3f(0.0,0.0,0.0);

    for(size_t i=0, iend=vpMPs.size(); i<iend;i++)
    {
        if(vpMPs[i]->isBad() || spRefMPs.count(vpMPs[i]))
            continue;
        cv::Mat pos = vpMPs[i]->GetWorldPos();
        glVertex3f(pos.at<float>(0),pos.at<float>(1),pos.at<float>(2));
    }
    glEnd();
#endif

#ifdef HAVE_GLES_2
    float color_2[] = {1.0f, 0.0f, 0.0f, 1.0f};
    float points2[] = {};
    std::vector<float> vec_points_2; 
    for(set<MapPoint*>::iterator sit=spRefMPs.begin(), send=spRefMPs.end(); sit!=send; sit++)
    {
        if((*sit)->isBad())
            continue;
        cv::Mat pos = (*sit)->GetWorldPos();
        vec_points_2.push_back(pos.at<float>(0));
        vec_points_2.push_back(pos.at<float>(1));
        vec_points_2.push_back(pos.at<float>(2));
    } 
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    pangolin::glEngine().SetColor(1.0,0.0,0.0,1.0);   //set color    red
    if (vec_points_2.size() > 0) {
        glVertexPointer(3, GL_FLOAT, 0, vec_points_2.data());
        length = vec_points_2.size() / 3;
    }
    else {
        glVertexPointer(3, GL_FLOAT, 0, points2);
        length = 0;
    }
    
    glColorPointer(4, GL_FLOAT, 0, color_2);
    glDrawArrays(GL_POINTS,0,length);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // //  debug
    // pangolin::glEngine().currentmatrix->pop();

#else
    glPointSize(mPointSize);
    glBegin(GL_POINTS);
    glColor3f(1.0,0.0,0.0);
    for(set<MapPoint*>::iterator sit=spRefMPs.begin(), send=spRefMPs.end(); sit!=send; sit++)
    {
        if((*sit)->isBad())
            continue;
        cv::Mat pos = (*sit)->GetWorldPos();
        glVertex3f(pos.at<float>(0),pos.at<float>(1),pos.at<float>(2));

    }
    glEnd();
#endif
}

void MapDrawer::DrawKeyFrames(const bool bDrawKF, const bool bDrawGraph)
{
    
    const float &w = mKeyFrameSize;
    const float h = w*0.75;
    const float z = w*0.6;

    const vector<KeyFrame*> vpKFs = mpMap->GetAllKeyFrames();

    if(bDrawKF)
    {
        for(size_t i=0; i<vpKFs.size(); i++)
        {
            KeyFrame* pKF = vpKFs[i];
            cv::Mat Twc = pKF->GetPoseInverse().t();

#ifdef HAVE_GLES_2

    pangolin::glEngine().currentmatrix->push(pangolin::glEngine().currentmatrix->top());
    pangolin::OpenGlMatrix TT;
    memcpy(TT.m, Twc.ptr<GLfloat>(0), sizeof(float) * 16 );
    TT.Multiply();

    glLineWidth(mKeyFrameLineWidth);

    GLfloat color[] = {0.0f,0.0f,1.0f};
    GLfloat vertex[48] = { 0 };
    vertex[0] = 0;
    vertex[1] = 0;
    vertex[2] = 0;
    vertex[3] = w;
    vertex[4] = h;
    vertex[5] = z;
    vertex[6] = 0;
    vertex[7] = 0;
    vertex[8] = 0;
    vertex[9] = w;
    vertex[10] = -h;
    vertex[11] = z;
    vertex[12] = 0;
    vertex[13] = 0;
    vertex[14] = 0;
    vertex[15] = -w;
    vertex[16] = -h;
    vertex[17] = z;
    vertex[18] = 0;
    vertex[19] = 0;
    vertex[20] = 0;
    vertex[21] = -w;
    vertex[22] = h;
    vertex[23] = z;
    vertex[24] = w;
    vertex[25] = h;
    vertex[26] = z;
    vertex[27] = w;
    vertex[28] = -h;
    vertex[29] = z;

    vertex[30] = -w;
    vertex[31] = h;
    vertex[32] = z;
    vertex[33] = -w;
    vertex[34] = -h;
    vertex[35] = z;

    vertex[36] = -w;
    vertex[37] = h;
    vertex[38] = z;
    vertex[39] = w;
    vertex[40] = h;
    vertex[41] = z;

    vertex[42] = -w;
    vertex[43] = -h;
    vertex[44] = z;
    vertex[45] = w;
    vertex[46] = -h;
    vertex[47] = z;

    pangolin::glEngine().SetColor(0.0,0.0,1.0,1.0);   //set color   bule
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertex);
    glColorPointer(3, GL_FLOAT, 0, color);
    glDrawArrays(GL_LINES, 0, 16);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    pangolin::glEngine().currentmatrix->pop();
    pangolin::glEngine().UpdateMatrices();

#else
            glPushMatrix();
            glMultMatrixf(Twc.ptr<GLfloat>(0));

            glLineWidth(mKeyFrameLineWidth);
            glColor3f(0.0f,0.0f,1.0f);
            glBegin(GL_LINES);
            glVertex3f(0,0,0);
            glVertex3f(w,h,z);
            glVertex3f(0,0,0);
            glVertex3f(w,-h,z);
            glVertex3f(0,0,0);
            glVertex3f(-w,-h,z);
            glVertex3f(0,0,0);
            glVertex3f(-w,h,z);

            glVertex3f(w,h,z);
            glVertex3f(w,-h,z);

            glVertex3f(-w,h,z);
            glVertex3f(-w,-h,z);

            glVertex3f(-w,h,z);
            glVertex3f(w,h,z);

            glVertex3f(-w,-h,z);
            glVertex3f(w,-h,z);
            glEnd();

            glPopMatrix();
#endif
        }
    }

    if(bDrawGraph)
    {
        glLineWidth(mGraphLineWidth);
#ifdef HAVE_GLES_2
    GLfloat Color2[] = {1.0f, 0.0f, 0.0f, 1.0f};
    std::vector<GLfloat> vertex2;
    GLfloat vertex2_[] = { };
              

    for(size_t i=0; i<vpKFs.size(); i++)
    {
        // Covisibility Graph
        const vector<KeyFrame*> vCovKFs = vpKFs[i]->GetCovisiblesByWeight(100);
        cv::Mat Ow = vpKFs[i]->GetCameraCenter();
        if(!vCovKFs.empty())
        {
            for(vector<KeyFrame*>::const_iterator vit=vCovKFs.begin(), vend=vCovKFs.end(); vit!=vend; vit++)
            {
                if((*vit)->mnId<vpKFs[i]->mnId)
                    continue;
                cv::Mat Ow2 = (*vit)->GetCameraCenter();
                vertex2.push_back(Ow.at<float>(0));
                vertex2.push_back(Ow.at<float>(1));
                vertex2.push_back(Ow.at<float>(2));
                vertex2.push_back(Ow2.at<float>(0));
                vertex2.push_back(Ow2.at<float>(1));
                vertex2.push_back(Ow2.at<float>(2));
            }
        }

        // Spanning tree
        KeyFrame* pParent = vpKFs[i]->GetParent();
        if(pParent)
        {
            cv::Mat Owp = pParent->GetCameraCenter();
            vertex2.push_back(Ow.at<float>(0));
            vertex2.push_back(Ow.at<float>(1));
            vertex2.push_back(Ow.at<float>(2));
            vertex2.push_back(Owp.at<float>(0));
            vertex2.push_back(Owp.at<float>(1));
            vertex2.push_back(Owp.at<float>(2));
        }

        // Loops
        set<KeyFrame*> sLoopKFs = vpKFs[i]->GetLoopEdges();
        for(set<KeyFrame*>::iterator sit=sLoopKFs.begin(), send=sLoopKFs.end(); sit!=send; sit++)
        {
            if((*sit)->mnId<vpKFs[i]->mnId)
                continue;
            cv::Mat Owl = (*sit)->GetCameraCenter();
            vertex2.push_back(Ow.at<float>(0));
            vertex2.push_back(Ow.at<float>(1));
            vertex2.push_back(Ow.at<float>(2));
            vertex2.push_back(Owl.at<float>(0));
            vertex2.push_back(Owl.at<float>(1));
            vertex2.push_back(Owl.at<float>(2));
        }
    }


    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    if (vertex2.size() > 0)
        glVertexPointer(3, GL_FLOAT, 0, vertex2.data());
    else
        glVertexPointer(3, GL_FLOAT, 0, vertex2_);

    pangolin::glEngine().SetColor(0.0,1.0,0.0,1.0);   //set color    GREEN
    glColorPointer(4, GL_FLOAT, 0, Color2);
    glDrawArrays(GL_LINES, 0, vertex2.size() / 3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
#else
        glColor4f(0.0f,1.0f,0.0f,0.6f);
        glBegin(GL_LINES);

        for(size_t i=0; i<vpKFs.size(); i++)
        {
            // Covisibility Graph
            const vector<KeyFrame*> vCovKFs = vpKFs[i]->GetCovisiblesByWeight(100);
            cv::Mat Ow = vpKFs[i]->GetCameraCenter();
            if(!vCovKFs.empty())
            {
                for(vector<KeyFrame*>::const_iterator vit=vCovKFs.begin(), vend=vCovKFs.end(); vit!=vend; vit++)
                {
                    if((*vit)->mnId<vpKFs[i]->mnId)
                        continue;
                    cv::Mat Ow2 = (*vit)->GetCameraCenter();
                    glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                    glVertex3f(Ow2.at<float>(0),Ow2.at<float>(1),Ow2.at<float>(2));
                }
            }

            // Spanning tree
            KeyFrame* pParent = vpKFs[i]->GetParent();
            if(pParent)
            {
                cv::Mat Owp = pParent->GetCameraCenter();
                glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                glVertex3f(Owp.at<float>(0),Owp.at<float>(1),Owp.at<float>(2));
            }

            // Loops
            set<KeyFrame*> sLoopKFs = vpKFs[i]->GetLoopEdges();
            for(set<KeyFrame*>::iterator sit=sLoopKFs.begin(), send=sLoopKFs.end(); sit!=send; sit++)
            {
                if((*sit)->mnId<vpKFs[i]->mnId)
                    continue;
                cv::Mat Owl = (*sit)->GetCameraCenter();
                glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                glVertex3f(Owl.at<float>(0),Owl.at<float>(1),Owl.at<float>(2));
            }
        }

        glEnd();
#endif
    }
}

void MapDrawer::DrawCurrentCamera(pangolin::OpenGlMatrix &Twc)
{
    const float &w = mCameraSize;
    const float h = w*0.75;
    const float z = w*0.6;

    
    

#ifdef HAVE_GLES
    pangolin::glEngine().currentmatrix->push(pangolin::glEngine().currentmatrix->top());
    Twc.Multiply();
#else
        glPushMatrix();
        glMultMatrixd(Twc.m);
#endif

        glLineWidth(mCameraLineWidth);

#ifdef HAVE_GLES_2

    GLfloat color[] = {0.0f,1.0f,0.0f};
    GLfloat vertex[48] = { 0 };
    vertex[0] = 0;
    vertex[1] = 0;
    vertex[2] = 0;
    vertex[3] = w;
    vertex[4] = h;
    vertex[5] = z;
    vertex[6] = 0;
    vertex[7] = 0;
    vertex[8] = 0;
    vertex[9] = w;
    vertex[10] = -h;
    vertex[11] = z;
    vertex[12] = 0;
    vertex[13] = 0;
    vertex[14] = 0;
    vertex[15] = -w;
    vertex[16] = -h;
    vertex[17] = z;
    vertex[18] = 0;
    vertex[19] = 0;
    vertex[20] = 0;
    vertex[21] = -w;
    vertex[22] = h;
    vertex[23] = z;
    vertex[24] = w;
    vertex[25] = h;
    vertex[26] = z;
    vertex[27] = w;
    vertex[28] = -h;
    vertex[29] = z;

    vertex[30] = -w;
    vertex[31] = h;
    vertex[32] = z;
    vertex[33] = -w;
    vertex[34] = -h;
    vertex[35] = z;

    vertex[36] = -w;
    vertex[37] = h;
    vertex[38] = z;
    vertex[39] = w;
    vertex[40] = h;
    vertex[41] = z;

    vertex[42] = -w;
    vertex[43] = -h;
    vertex[44] = z;
    vertex[45] = w;
    vertex[46] = -h;
    vertex[47] = z;
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    pangolin::glEngine().SetColor(0.0,1.0,0.0,1.0);   //set color    GREEN
    glVertexPointer(3, GL_FLOAT, 0, vertex);
    glColorPointer(3, GL_FLOAT, 0, color);
    glDrawArrays(GL_LINES, 0, 16);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    pangolin::glEngine().currentmatrix->pop();
    pangolin::glEngine().UpdateMatrices();
#else
    
    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(w,h,z);
    glVertex3f(0,0,0);
    glVertex3f(w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,h,z);

    glVertex3f(w,h,z);
    glVertex3f(w,-h,z);

    glVertex3f(-w,h,z);
    glVertex3f(-w,-h,z);

    glVertex3f(-w,h,z);
    glVertex3f(w,h,z);

    glVertex3f(-w,-h,z);
    glVertex3f(w,-h,z);
    glEnd();

    glPopMatrix();
#endif
}


void MapDrawer::SetCurrentCameraPose(const cv::Mat &Tcw)
{
    unique_lock<mutex> lock(mMutexCamera);
    mCameraPose = Tcw.clone();
}

void MapDrawer::GetCurrentOpenGLCameraMatrix(pangolin::OpenGlMatrix &M)
{
    if(!mCameraPose.empty())
    {
        cv::Mat Rwc(3,3,CV_32F);
        cv::Mat twc(3,1,CV_32F);
        {
            unique_lock<mutex> lock(mMutexCamera);
            Rwc = mCameraPose.rowRange(0,3).colRange(0,3).t();
            twc = -Rwc*mCameraPose.rowRange(0,3).col(3);
        }

        M.m[0] = Rwc.at<float>(0,0);
        M.m[1] = Rwc.at<float>(1,0);
        M.m[2] = Rwc.at<float>(2,0);
        M.m[3]  = 0.0;

        M.m[4] = Rwc.at<float>(0,1);
        M.m[5] = Rwc.at<float>(1,1);
        M.m[6] = Rwc.at<float>(2,1);
        M.m[7]  = 0.0;

        M.m[8] = Rwc.at<float>(0,2);
        M.m[9] = Rwc.at<float>(1,2);
        M.m[10] = Rwc.at<float>(2,2);
        M.m[11]  = 0.0;

        M.m[12] = twc.at<float>(0);
        M.m[13] = twc.at<float>(1);
        M.m[14] = twc.at<float>(2);
        M.m[15]  = 1.0;
    }
    else
        M.SetIdentity();
}

} //namespace ORB_SLAM
