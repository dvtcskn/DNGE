
#include "pch.h"
#include "Core/Camera.h"
#include "Engine/AbstractEngine.h"

FVector2 sCamera::ConvertScreenToWorld(const FVector2& ps) const
{
    auto Dimension = GPU::GetBackBufferDimension();
    float m_zoom = 1.0f;
    FVector2 m_center = FVector2::Zero();

    float w = float(Dimension.Width);
    float h = float(Dimension.Height);
    float u = ps.X / w;
    float v = (h - ps.Y) / h;

    float ratio = w / h;
    FVector2 extents(ratio * 25.0f, 25.0f);
    extents *= m_zoom;

    FVector2 lower = m_center - extents;
    FVector2 upper = m_center + extents;

    FVector2 pw;
    pw.X = (1.0f - u) * lower.X + u * upper.X;
    pw.Y = (1.0f - v) * lower.Y + v * upper.Y;
    return pw;
}

FVector2 sCamera::ConvertWorldToScreen(const FVector2& pw) const
{
    auto Dimension = GPU::GetBackBufferDimension();
    float m_zoom = 1.0f;
    FVector2 m_center = FVector2::Zero();

    float w = float(Dimension.Width);
    float h = float(Dimension.Height);
    float ratio = w / h;
    FVector2 extents(ratio * 25.0f, 25.0f);
    extents *= m_zoom;

    FVector2 lower = m_center - extents;
    FVector2 upper = m_center + extents;

    float u = (pw.X - lower.X) / (upper.X - lower.X);
    float v = (pw.Y - lower.Y) / (upper.Y - lower.Y);

    FVector2 ps;
    ps.X = u * w;
    ps.Y = (1.0f - v) * h;
    return ps;
}
