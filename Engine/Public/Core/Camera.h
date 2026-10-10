/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coþkun.
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ---------------------------------------------------------------------------------------
*/

#pragma once

#include "Engine/ClassBody.h"
#include "Math/CoreMath.h"
#include <iostream>

struct sCameraSceneBuffer
{
    FMatrix ViewProjMatrix;
    FMatrix ViewMatrix;
    FMatrix InverseViewMatrix;
    FMatrix PreviousViewProjMatrix;
    FMatrix ReprojectMatrix;

    sCameraSceneBuffer()
        : ViewProjMatrix(FMatrix::Identity())
        , ViewMatrix(FMatrix::Identity())
        , InverseViewMatrix(FMatrix::Identity())
        , PreviousViewProjMatrix(FMatrix::Identity())
        , ReprojectMatrix(FMatrix::Identity())
    {}
};

class ICamera
{
    sBaseClassBody(sClassDefaultProtectedConstructor, ICamera)
public:
    virtual bool IsOrthographic() const = 0;
    virtual float GetFOV() const = 0;
    virtual float GetAspectRatio() const = 0;
    virtual float GetNearClip() const = 0;
    virtual float GetFarClip() const = 0;
    virtual bool IsZReversed() const = 0;
    virtual float GetClearDepth() const = 0;
    virtual bool IsZInfinite() const = 0;

    virtual FVector GetPosition() const = 0;
    virtual FVector GetRotation() const = 0;
    virtual FVector GetFocus() const = 0;

    virtual FMatrix GetWorldMatrix() const = 0;
    virtual float GetSceneScaling() const = 0;
    virtual FMatrix GetViewMatrix() const = 0;
    virtual FMatrix GetInverseViewMatrix() const = 0;
    virtual FMatrix GetProjMatrix() const = 0;
    virtual FMatrix GetViewProjMatrix() const = 0;
    virtual FMatrix GetPrevViewProjMatrix() const = 0;
    virtual FMatrix GetJitteredProjMatrix() const = 0;
    virtual FMatrix GetJitteredPrevProjMatrix() const = 0;
    virtual FMatrix GetReprojectionMatrix() const = 0;
    virtual FVector GetEye() const = 0;

    virtual bool IsJitterEnabled() const = 0;
    virtual void SetEnableJitter(bool bEnable) = 0;
    virtual FVector2 GetJitter() const = 0;
    virtual FVector2 GetPreviousJitter() const = 0;
    virtual void SetJitter(const FVector2& Jitter) = 0;
    virtual bool IsJitterCallBackSet() const = 0;
    virtual void SetJitterCallBack(std::function<bool(FVector2&)> Callback) = 0;
};

class sCamera final : public ICamera
{
    sClassBody(sClassConstructor, sCamera, ICamera)
public:
    sCamera()
        : Super()
        , ReverseZ(true)
        , InfiniteZ(true)
        , VerticalFOV(0.0f)
        , AspectRatio(0.0f)
        , NearClip(0.0f)
        , FarClip(0.0f)
        , Eye(FVector::Zero())
        , LookAt(FVector::Zero())
        , ViewMatrix(FMatrix::Identity())
        , CameraWorld(FMatrix::Identity())
        , ViewProjMatrix(FMatrix::Identity())
        , PreviousViewProjMatrix(FMatrix::Identity())
        , ProjMatrix(FMatrix::Identity())
        , ReprojectMatrix(FMatrix::Identity())
        , WorldMatrix(FMatrix::Identity())
        , bIsOrthographic(false)
        , sceneScaling(1.0f)
        , bIsJitterEnabled(false)
        , Jitter(FVector2())
        , PreviousJitter(FVector2())
        , JitterCallback(nullptr)
        , JitteredProjMatrix(FMatrix::Identity())
        , PreviousJitteredProjMatrix(FMatrix::Identity())
    {
        SetPerspectiveMatrix((float)dPI_OVER_4, 9.0f / 16.0f, 0.1f, 4000.0f);
        SetWorldScale(1.0f);
    }

public:
    inline ~sCamera() = default;

    inline void Update()
    {
        PreviousJitter = Jitter;
        PreviousJitteredProjMatrix = JitteredProjMatrix;
        PreviousViewProjMatrix = ViewProjMatrix;

        if (bIsJitterEnabled)
        {
            if (JitterCallback)
            {
                bool bSuccess = JitterCallback(Jitter);
                if (!bSuccess)
                    SetEnableJitter(false);
            }
            FMatrix JitterMat = FMatrix(TMatrix3x3<float>::Identity(), FVector(Jitter.X, Jitter.Y, 0.0f));
            JitteredProjMatrix = ProjMatrix * JitterMat;
            ViewProjMatrix = WorldMatrix * ViewMatrix * JitteredProjMatrix;
        }
        else
        {
            ViewProjMatrix = WorldMatrix * ViewMatrix * ProjMatrix;
        }
        
        ReprojectMatrix = Invert(GetViewProjMatrix()) * PreviousViewProjMatrix;
    }

private:
    inline void UpdatePerspectiveProjMatrix()
    {
        if (bIsOrthographic)
            return;

        float Y = 1.0f / std::tanf(VerticalFOV * 0.5f);
        float X = Y * AspectRatio;

        float Q1, Q2;

        // ReverseZ puts far plane at Z=0 and near plane at Z=1.  This is never a bad idea, and it's
        // actually a great idea with F32 depth buffers to redistribute precision more evenly across
        // the entire range.  It requires clearing Z to 0.0f and using a GREATER variant depth test.
        // Some care must also be done to properly reconstruct linear W in a pixel shader from hyperbolic Z.
        if (ReverseZ)
        {
            if (InfiniteZ)
            {
                Q1 = 0.0f;
                Q2 = NearClip;
            }
            else
            {
                Q1 = NearClip / (FarClip - NearClip);
                Q2 = Q1 * FarClip;
            }
        }
        else
        {
            if (InfiniteZ)
            {
                Q1 = -1.0f;
                Q2 = -NearClip;
            }
            else
            {
                Q1 = FarClip / (NearClip - FarClip);
                Q2 = Q1 * NearClip;
            }
        }

        SetProjMatrix(FMatrix(FVector4(X, 0.0f, 0.0f, 0.0f),
            FVector4(0.0f, Y, 0.0f, 0.0f),
            FVector4(0.0f, 0.0f, Q1, -1.0f),
            FVector4(0.0f, 0.0f, Q2, 0.0f)
        ));

        /*SetProjMatrix(Matrix4(
            Vector4(X, 0.0f, 0.0f, 0.0f),
            Vector4(0.0f, Y, 0.0f, 0.0f),
            Vector4(0.0f, 0.0f, 1.0f, 1.0f),
            Vector4(0.0f, 0.0f, -0.01f, 0.0f)
        ));*/
    }

public:
    inline virtual bool IsOrthographic() const override { return bIsOrthographic; }
    inline virtual float GetFOV() const override { return VerticalFOV; }
    inline void SetFOV(float verticalFovInRadians)
    {
        VerticalFOV = verticalFovInRadians;
        UpdatePerspectiveProjMatrix();
    }
    inline virtual float GetAspectRatio() const override { return AspectRatio; }
    inline void SetAspectRatio(float heightOverWidth)
    {
        AspectRatio = heightOverWidth;
        UpdatePerspectiveProjMatrix();
    }
    inline virtual float GetNearClip() const override { return NearClip; }
    inline virtual float GetFarClip() const override { return FarClip; }
    inline void SetZRange(float nearZ, float farZ)
    {
        NearClip = nearZ;
        FarClip = farZ;
        UpdatePerspectiveProjMatrix();
    }

    inline virtual bool IsZReversed() const override { return ReverseZ; }
    inline void SetReverseZ(bool Value)
    {
        ReverseZ = Value;
        UpdatePerspectiveProjMatrix();
    }
    inline virtual float GetClearDepth() const override { return ReverseZ ? 0.0f : 1.0f; }

    inline virtual bool IsZInfinite() const override { return InfiniteZ; }
    inline void SetInfiniteZ(bool Value)
    {
        InfiniteZ = Value;
        UpdatePerspectiveProjMatrix();
    }

    inline void SetWorldScale(double InSceneScaling, bool zAxisUp = false)
    {
        sceneScaling = (float)InSceneScaling;
        WorldMatrix = FMatrix::MakeScale(sceneScaling);

        if (zAxisUp)
        {
            WorldMatrix = FMatrix(DirectX::XMMatrixRotationX(-DirectX::XM_PI / 2.0f)) * WorldMatrix;
        }

        FVector sceneTranslation = FVector::Zero();
        {
            //WorldMatrix = FMatrix(DirectX::XMMatrixTranslation(sceneTranslation.X, sceneTranslation.Y, sceneTranslation.Z)) * WorldMatrix;
        }
    }

public:
    inline void SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip)
    {
        VerticalFOV = verticalFovRadians;
        AspectRatio = aspectHeightOverWidth;
        NearClip = nearZClip;
        FarClip = farZClip;

        bIsOrthographic = false;

        UpdatePerspectiveProjMatrix();

        PreviousViewProjMatrix = ViewProjMatrix;
    }

    inline void SetOrthographic(std::size_t Width, std::size_t Height)
    {
        bIsOrthographic = true;

        ProjMatrix = GetOrthographicTransform((int)Width, (int)Height);

        ViewProjMatrix = ProjMatrix;
        PreviousViewProjMatrix = ViewProjMatrix;
    }

    inline void SetViewParams(FVector vEyePt, FVector vLookatPt)
    {
        using namespace DirectX;

        Eye = vEyePt;
        LookAt = vLookatPt;

        // Calc the view matrix
        ViewMatrix = XMMatrixLookAtLH(Eye, LookAt, g_XMIdentityR1);

        CameraWorld = XMMatrixInverse(nullptr, ViewMatrix);

        // The axis basis vectors and camera position are stored inside the 
        // position matrix in the 4 rows of the camera's world matrix.
        // To figure out the yaw/pitch of the camera, we just need the Z basis vector
        XMFLOAT3 zBasis;
        XMStoreFloat3(&zBasis, CameraWorld.r[2]);

        const float fLen = sqrtf(zBasis.z * zBasis.z + zBasis.x * zBasis.x);

        Rotation.X = 0.0f;
        Rotation.Y = -atan2f(zBasis.y, fLen);
        Rotation.Z = atan2f(zBasis.x, zBasis.z);
    }

    inline void SetPosition(const FVector Position, const float Pitch = 0.0f, const float Yaw = 0.0f, const float Roll = 0.0f)
    {
        using namespace DirectX;

        // Make a rotation matrix based on the camera's yaw & pitch
        XMMATRIX mCameraRot = XMMatrixRotationRollPitchYaw(Pitch, Yaw, Roll);

        // Transform vectors based on camera's rotation matrix
        XMVECTOR vWorldUp = XMVector3TransformCoord(g_XMIdentityR1, mCameraRot);
        XMVECTOR vWorldAhead = XMVector3TransformCoord(g_XMIdentityR2, mCameraRot);

        XMVECTOR vPosDeltaWorld = XMVector3TransformCoord(Position, mCameraRot);

        // Move the eye position 
        Eye = vPosDeltaWorld;

        // Update the lookAt position based on the eye position
        LookAt = Eye + vWorldAhead;

        // Update the view matrix
        ViewMatrix = XMMatrixLookAtLH(Eye, LookAt, vWorldUp);

        CameraWorld = XMMatrixInverse(nullptr, ViewMatrix);
    }

    inline void SetTransform(const FVector& Position, const float Pitch, const float Yaw, const float Roll)
    {
        using namespace DirectX;

        Rotation.X = Roll;
        Rotation.Y = Pitch;
        Rotation.Z = Yaw;

        // Make a rotation matrix based on the camera's yaw & pitch
        XMMATRIX mCameraRot = XMMatrixRotationRollPitchYaw(Pitch, Yaw, Roll);

        // Transform vectors based on camera's rotation matrix
        XMVECTOR vWorldUp = XMVector3TransformCoord(g_XMIdentityR1, mCameraRot);
        XMVECTOR vWorldAhead = XMVector3TransformCoord(g_XMIdentityR2, mCameraRot);

        XMVECTOR vPosDeltaWorld = XMVector3TransformCoord(Position, mCameraRot);

        // Move the eye position 
        Eye += vPosDeltaWorld;

        // Update the lookAt position based on the eye position
        LookAt = Eye + vWorldAhead;

        // Update the view matrix
        ViewMatrix = XMMatrixLookAtLH(Eye, LookAt, vWorldUp);

        CameraWorld = XMMatrixInverse(nullptr, ViewMatrix);
    }

    inline FVector2 ConvertScreenToWorld(const float WorldWidth, const float WorldHeight, const FVector2& Screen) const
    {
        float Zoom = 1.0f;
        FVector2 Center = FVector2::Zero();

        float W = float(WorldWidth);
        float H = float(WorldHeight);
        float U = Screen.X / W;
        float V = (H - Screen.Y) / H;

        float Ratio = W / H;
        FVector2 Extents(Ratio * 25.0f, 25.0f);
        Extents *= Zoom;

        FVector2 Lower = Center - Extents;
        FVector2 Upper = Center + Extents;

        FVector2 World;
        World.X = (1.0f - U) * Lower.X + U * Upper.X;
        World.Y = (1.0f - V) * Lower.Y + V * Upper.Y;
        return World;
    }

    inline FVector2 ConvertWorldToScreen(const float ScreenWidth, const float ScreenHeight, const FVector2& World) const
    {
        float Zoom = 1.0f;
        FVector2 Center = FVector2::Zero();

        float W = float(ScreenWidth);
        float H = float(ScreenHeight);
        float Ratio = W / H;
        FVector2 Extents(Ratio * 25.0f, 25.0f);
        Extents *= Zoom;

        FVector2 Lower = Center - Extents;
        FVector2 Upper = Center + Extents;

        float U = (World.X - Lower.X) / (Upper.X - Lower.X);
        float V = (World.Y - Lower.Y) / (Upper.Y - Lower.Y);

        FVector2 Screen;
        Screen.X = U * W;
        Screen.Y = (1.0f - V) * H;
        return Screen;
    }


    inline virtual FVector GetPosition() const override { return Eye; }
    inline virtual FVector GetRotation() const override { return Rotation; }
    inline virtual FVector GetFocus() const override { return LookAt; }

    inline virtual float GetSceneScaling() const override { return sceneScaling; }
    inline virtual FMatrix GetWorldMatrix() const override { return WorldMatrix; }
    inline virtual FMatrix GetViewMatrix() const override { return ViewMatrix; }
    inline virtual FMatrix GetInverseViewMatrix() const override { return CameraWorld; }
    inline virtual FMatrix GetProjMatrix() const override { return ProjMatrix; }
    inline virtual FMatrix GetViewProjMatrix() const override { return ViewProjMatrix; }
    inline virtual FMatrix GetPrevViewProjMatrix() const override { return PreviousViewProjMatrix; }
    inline virtual FMatrix GetJitteredProjMatrix() const override { return JitteredProjMatrix; }
    inline virtual FMatrix GetJitteredPrevProjMatrix() const override { return PreviousJitteredProjMatrix; }
    inline virtual FMatrix GetReprojectionMatrix() const override { return ReprojectMatrix; }
    inline virtual FVector GetEye() const override { return FVector(ViewMatrix.r[3].X, ViewMatrix.r[3].Y, ViewMatrix.r[3].Z); }

    inline void SetProjMatrix(const FMatrix& ProjMat) { ProjMatrix = ProjMat; }
    inline void SetViewMatrix(const FMatrix& InView)
    {
        ViewMatrix = InView; 
        CameraWorld = DirectX::XMMatrixInverse(nullptr, ViewMatrix);
    }

    virtual bool IsJitterEnabled() const override
    {
        return bIsJitterEnabled;
    }

    virtual void SetEnableJitter(bool bEnable) override
    {
        bIsJitterEnabled = bEnable;
        if (!bIsJitterEnabled)
            JitterCallback = nullptr;
    }

    virtual FVector2 GetPreviousJitter() const override
    {
        return PreviousJitter;
    }

    virtual FVector2 GetJitter() const override
    {
        return Jitter;
    }

    virtual void SetJitter(const FVector2& NewJitter) override
    {
        Jitter = NewJitter;
    }

    virtual bool IsJitterCallBackSet() const override
    {
        return JitterCallback != nullptr;
    }
    virtual void SetJitterCallBack(std::function<bool(FVector2&)> Callback) override
    {
        JitterCallback = Callback;
    }

private:
    float VerticalFOV;
    float AspectRatio;
    float NearClip;
    float FarClip;

    bool ReverseZ;  // Invert near and far clip distances so that Z=0 is the far plane
    bool InfiniteZ; // Move the far plane to infinity

    bool bIsOrthographic;

    FVector Eye;
    FVector LookAt;
    FVector Rotation;
    FMatrix ViewMatrix;
    FMatrix CameraWorld;

    FMatrix ViewProjMatrix; 
    FMatrix ProjMatrix;
    FMatrix PreviousViewProjMatrix;
    FMatrix ReprojectMatrix;

    FMatrix JitteredProjMatrix;
    FMatrix PreviousJitteredProjMatrix;

    FMatrix WorldMatrix;
    float sceneScaling;

    bool bIsJitterEnabled;
    FVector2 Jitter;
    FVector2 PreviousJitter;
    std::function<bool(FVector2&)> JitterCallback;
};
