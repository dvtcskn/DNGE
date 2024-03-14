#pragma once

#ifndef USING_DIRECTX_HEADERS
#define USING_DIRECTX_HEADERS
#endif

//#ifndef D3DX12_NO_STATE_OBJECT_HELPERS
//#define D3DX12_NO_STATE_OBJECT_HELPERS
//#endif
//#ifndef D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
//#define D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
//#endif

//#ifdef USING_DIRECTX_HEADERS
//#undef USING_DIRECTX_HEADERS
//#endif

#ifdef USING_DIRECTX_HEADERS
#include <directx/d3d12.h>
#include <directx/d3d12video.h>
#include <directx/dxcore.h>
#include <directx/d3dx12.h>
#include <dxguids/dxguids.h>
#else
#include <d3d12.h>
#include <d3d12video.h>
#include <dxcore.h>
#include <d3dx12.h>
#endif

