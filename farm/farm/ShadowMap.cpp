#include "stdafx.h"
#include "ShadowMap.h"
#include "d3dHelper.h"

ShadowMap::ShadowMap(ID3D12Device* pDevice, UINT width, UINT height) :
	m_width(width),
	m_height(height),
	m_pDevice(pDevice),
	m_viewport(D3D12_VIEWPORT{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f }),
	m_sissorRect(D3D12_RECT{0, 0, (int)width, (int)height})
{
	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = m_width;
	desc.Height = m_height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE option;
	option.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	option.DepthStencil.Depth = 1.0f;
	option.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&option,
			IID_PPV_ARGS(&m_shadowMap)));
}

void ShadowMap::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDsv)
{
	m_cpuSrvHandle = cpuSrv;
	m_gpuSrvHandle = gpuSrv;
	m_cpuDsvHandle = cpuDsv;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	m_pDevice->CreateShaderResourceView(m_shadowMap.Get(), &srvDesc, m_cpuSrvHandle);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_pDevice->CreateDepthStencilView(m_shadowMap.Get(), &dsvDesc, m_cpuDsvHandle);
}