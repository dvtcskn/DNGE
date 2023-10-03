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
    virtual FMatrix GetReprojectionMatrix() const = 0;
    virtual FVector GetEye() const = 0;
};

class sCamera final : public ICamera
{
    sClassBody(sClassConstructor, sCamera, ICamera)
public:
    sCamera()
        : Super()
        , m_ReverseZ(true)
        , m_InfiniteZ(true)
        , m_VerticalFOV(0.0f)
        , m_AspectRatio(0.0f)
        , m_NearClip(0.0f)
        , m_FarClip(0.0f)
        , m_vEye(FVector::Zero())
        , m_vLookAt(FVector::Zero())
        , m_ViewMatrix(FMatrix::Identity())
        , m_mCameraWorld(FMatrix::Identity())
        , m_ViewProjMatrix(FMatrix::Identity())
        , m_ProjMatrix(FMatrix::Identity())
        , m_PreviousViewProjMatrix(FMatrix::Identity())
        , m_ReprojectMatrix(FMatrix::Identity())
        , WorldMatrix(FMatrix::Identity())
        , bIsOrthographic(false)
        , sceneScaling(1.0f)
    {
        SetPerspectiveMatrix((float)dPI_OVER_4, 9.0f / 16.0f, 0.01f, 4000.0f);
        SetWorldScale(1.0f);
    }

public:
    inline ~sCamera() = default;

    // Call this function once per frame and after you've changed any state.  This
    // regenerates all matrices.  Calling it more or less than once per frame will break
    // temporal effects and cause unpredictable results.
    inline void Update()
    {
        m_PreviousViewProjMatrix = m_ViewProjMatrix;

        m_ViewProjMatrix = m_ViewMatrix * m_ProjMatrix * WorldMatrix;
        
        m_ReprojectMatrix = Invert(GetViewProjMatrix()) * m_PreviousViewProjMatrix;
    }

private:
    inline void UpdatePerspectiveProjMatrix()
    {
        if (bIsOrthographic)
            return;

        float Y = 1.0f / std::tanf(m_VerticalFOV * 0.5f);
        float X = Y * m_AspectRatio;

        float Q1, Q2;

        // ReverseZ puts far plane at Z=0 and near plane at Z=1.  This is never a bad idea, and it's
        // actually a great idea with F32 depth buffers to redistribute precision more evenly across
        // the entire range.  It requires clearing Z to 0.0f and using a GREATER variant depth test.
        // Some care must also be done to properly reconstruct linear W in a pixel shader from hyperbolic Z.
        if (m_ReverseZ)
        {
            if (m_InfiniteZ)
            {
                Q1 = 0.0f;
                Q2 = m_NearClip;
            }
            else
            {
                Q1 = m_NearClip / (m_FarClip - m_NearClip);
                Q2 = Q1 * m_FarClip;
            }
        }
        else
        {
            if (m_InfiniteZ)
            {
                Q1 = -1.0f;
                Q2 = -m_NearClip;
            }
            else
            {
                Q1 = m_FarClip / (m_NearClip - m_FarClip);
                Q2 = Q1 * m_NearClip;
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
    inline virtual float GetFOV() const override { return m_VerticalFOV; }
    inline void SetFOV(float verticalFovInRadians)
    {
        m_VerticalFOV = verticalFovInRadians;
        UpdatePerspectiveProjMatrix();
    }
    inline virtual float GetAspectRatio() const override { return m_AspectRatio; }
    inline void SetAspectRatio(float heightOverWidth)
    {
        m_AspectRatio = heightOverWidth;
        UpdatePerspectiveProjMatrix();
    }
    inline virtual float GetNearClip() const override { return m_NearClip; }
    inline virtual float GetFarClip() const override { return m_FarClip; }
    inline void SetZRange(float nearZ, float farZ)
    {
        m_NearClip = nearZ;
        m_FarClip = farZ;
        UpdatePerspectiveProjMatrix();
    }

    inline virtual bool IsZReversed() const override { return m_ReverseZ; }
    inline void SetReverseZ(bool Value)
    {
        m_ReverseZ = Value;
        UpdatePerspectiveProjMatrix();
    }
    inline virtual float GetClearDepth() const override { return m_ReverseZ ? 0.0f : 1.0f; }

    inline virtual bool IsZInfinite() const override { return m_InfiniteZ; }
    inline void SetInfiniteZ(bool Value)
    {
        m_InfiniteZ = Value;
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
        m_VerticalFOV = verticalFovRadians;
        m_AspectRatio = aspectHeightOverWidth;
        m_NearClip = nearZClip;
        m_FarClip = farZClip;

        bIsOrthographic = false;

        UpdatePerspectiveProjMatrix();

        m_PreviousViewProjMatrix = m_ViewProjMatrix;
    }

    inline void SetOrthographic(std::size_t Width, std::size_t Height)
    {
        bIsOrthographic = true;

        m_ProjMatrix = GetOrthographicTransform((int)Width, (int)Height);

        m_ViewProjMatrix = m_ProjMatrix;
        m_PreviousViewProjMatrix = m_ViewProjMatrix;
    }

    inline void SetViewParams(FVector vEyePt, FVector vLookatPt)
    {
        using namespace DirectX;

        m_vEye = vEyePt;
        m_vLookAt = vLookatPt;

        // Calc the view matrix
        m_ViewMatrix = XMMatrixLookAtLH(m_vEye, m_vLookAt, g_XMIdentityR1);

        m_mCameraWorld = XMMatrixInverse(nullptr, m_ViewMatrix);

        // The axis basis vectors and camera position are stored inside the 
        // position matrix in the 4 rows of the camera's world matrix.
        // To figure out the yaw/pitch of the camera, we just need the Z basis vector
        XMFLOAT3 zBasis;
        XMStoreFloat3(&zBasis, m_mCameraWorld.r[2]);

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
        m_vEye = vPosDeltaWorld;

        // Update the lookAt position based on the eye position
        m_vLookAt = m_vEye + vWorldAhead;

        // Update the view matrix
        m_ViewMatrix = XMMatrixLookAtLH(m_vEye, m_vLookAt, vWorldUp);

        m_mCameraWorld = XMMatrixInverse(nullptr, m_ViewMatrix);
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
        m_vEye += vPosDeltaWorld;

        // Update the lookAt position based on the eye position
        m_vLookAt = m_vEye + vWorldAhead;

        // Update the view matrix
        m_ViewMatrix = XMMatrixLookAtLH(m_vEye, m_vLookAt, vWorldUp);

        m_mCameraWorld = XMMatrixInverse(nullptr, m_ViewMatrix);
    }

    FVector2 ConvertScreenToWorld(const FVector2& ps) const;
    FVector2 ConvertWorldToScreen(const FVector2& pw) const;

    inline virtual FVector GetPosition() const override { return m_vEye; }
    inline virtual FVector GetRotation() const override { return Rotation; }
    inline virtual FVector GetFocus() const override { return m_vLookAt; }

    inline virtual float GetSceneScaling() const override { return sceneScaling; }
    inline virtual FMatrix GetWorldMatrix() const override { return WorldMatrix; }
    inline virtual FMatrix GetViewMatrix() const override { return m_ViewMatrix; }
    inline virtual FMatrix GetInverseViewMatrix() const override { return m_mCameraWorld; }
    inline virtual FMatrix GetProjMatrix() const override { return m_ProjMatrix; }
    inline virtual FMatrix GetViewProjMatrix() const override { return m_ViewProjMatrix; }
    inline virtual FMatrix GetReprojectionMatrix() const override { return m_ReprojectMatrix; }
    inline virtual FVector GetEye() const override { return FVector(m_ViewMatrix.r[3].X, m_ViewMatrix.r[3].Y, m_ViewMatrix.r[3].Z); }

    inline void SetProjMatrix(const FMatrix& ProjMat) { m_ProjMatrix = ProjMat; }
    inline void SetViewMatrix(const FMatrix& InView)
    {
        m_ViewMatrix = InView; 
        m_mCameraWorld = DirectX::XMMatrixInverse(nullptr, m_ViewMatrix);
    }

private:
    float m_VerticalFOV;
    float m_AspectRatio;
    float m_NearClip;
    float m_FarClip;

    bool m_ReverseZ;  // Invert near and far clip distances so that Z=0 is the far plane
    bool m_InfiniteZ; // Move the far plane to infinity

    bool bIsOrthographic;

    FVector m_vEye;
    FVector m_vLookAt;
    FVector Rotation;
    FMatrix m_ViewMatrix;
    FMatrix m_mCameraWorld;

    FMatrix m_ViewProjMatrix; 
    FMatrix m_ProjMatrix;
    FMatrix m_PreviousViewProjMatrix;
    FMatrix m_ReprojectMatrix;

    FMatrix WorldMatrix;
    float sceneScaling;
};
