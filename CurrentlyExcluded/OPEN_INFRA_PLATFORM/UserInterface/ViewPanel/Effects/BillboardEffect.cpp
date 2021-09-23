/*
Copyright (c) 2021 Technical University of Munich
Chair of Computational Modeling and Simulation.

TUM Open Infra Platform is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License Version 3
as published by the Free Software Foundation.

TUM Open Infra Platform is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "BillboardEffect.h"
#include "OpenInfraPlatform/UserInterface/ViewPanel/RenderResources.h"
#include <BlueFramework/Rasterizer/vertex.h>


OIP_NAMESPACE_OPENINFRAPLATFORM_UI_BEGIN

typedef buw::VertexPosition3Color4 VertexTypeBillboard;

BillboardEffect::BillboardEffect(
	buw::IRenderSystem * renderSystem,
	buw::ReferenceCounted<buw::IViewport> viewport,
	buw::ReferenceCounted<buw::ITexture2D> depthStencilMSAA,
	buw::ReferenceCounted<buw::IConstantBuffer> worldBuffer,
	buw::ReferenceCounted<buw::IConstantBuffer> viewportBuffer)
	: buw::Effect(renderSystem), viewport_(viewport), worldBuffer_(worldBuffer), depthStencilMSAA_(depthStencilMSAA), viewportBuffer_(viewportBuffer)
{

}

BillboardEffect::~BillboardEffect()
{
	worldBuffer_ = nullptr;
	settingsBuffer_ = nullptr;
	viewportBuffer_ = nullptr;
	vertexBuffer_ = nullptr;
	depthStencilMSAA_ = nullptr;
	pipelineState_ = nullptr;
	viewport_ = nullptr;
}

void BillboardEffect::loadShader()
{
	try {
		/*Retrieve the vertexLayout from the vertex type.*/
		auto vertexLayout = VertexTypeBillboard::getVertexLayout();

		std::string effectFilename = buw::Singleton<RenderResources>::instance().getResourceRootDir() + "/Shader/BillboardEffect.be";

		buw::blendStateDescription bsd;
		bsd.renderTarget[0].blendEnable = true;		
		bsd.renderTarget[0].srcBlend = buw::eBlendValue::SrcAlpha;
		bsd.renderTarget[0].destBlend = buw::eBlendValue::InvSrcAlpha;

		/*Create the pipeline state.*/
		buw::pipelineStateDescription psd;
		psd.effectFilename = effectFilename;
		psd.pipelineStateName = "main";
		psd.vertexLayout = vertexLayout;
		psd.primitiveTopology = buw::ePrimitiveTopology::PointList;
		psd.renderTargetFormats = { buw::eTextureFormat::R8G8B8A8_UnsignedNormalizedInt_SRGB };
		psd.useDepth = true;
		psd.useMSAA = true;
		psd.fillMode = buw::eFillMode::Solid;
		psd.cullMode = buw::eCullMode::None;
		psd.blendStateDesc = bsd;
		

		pipelineState_ = createPipelineState(psd);		
	}
	catch(...) {
		pipelineState_ = nullptr;
	}
}

void BillboardEffect::drawPointsWithUniformColor(const bool checked)
{
	settings_.bUseUniformColor = checked;
	updateSettingsBuffer();
}

void BillboardEffect::drawPointsWithUniformSize(const bool checked)
{
	settings_.bUseUniformPointSize = checked;
	updateSettingsBuffer();
}

void BillboardEffect::setPointSize(const float size)
{
	settings_.pointSize = size;
	updateSettingsBuffer();
}

void BillboardEffect::setProxyModel(buw::ReferenceCounted<OpenInfraPlatform::Infrastructure::ProxyModel> proxyModel, buw::Vector3d offset)
{
	buw::vertexBufferDescription vbd;
	vbd.vertexCount = proxyModel->getAccidentReportCount();

	vbd.vertexLayout = VertexTypeBillboard::getVertexLayout();
	
	std::vector<VertexTypeBillboard> vertices = std::vector<VertexTypeBillboard>(vbd.vertexCount);

#pragma omp parallel for
	for(int i = 0; i < proxyModel->getAccidentReportCount(); i++) {
		
		buw::Vector3f pos = (proxyModel->getAccidentReportByIndex(i).position + offset).cast<float>();
		buw::Vector4f col = buw::Vector4f(1, 0, 0, 1.0f);
		vertices[i] = VertexTypeBillboard(buw::Vector3f(pos.x(),pos.z(),pos.y()), col);
	}
	vbd.data = vertices.data();

	vertexBuffer_ = renderSystem()->createVertexBuffer(vbd);	
}

void BillboardEffect::v_init()
{
	loadShader();
	
	/*Create the settings buffer.*/
	settings_.positions[0] = buw::Vector4f(-0.5, 0.5, 0, 0);
	settings_.positions[1] = buw::Vector4f(0.5, 0.5, 0, 0);
	settings_.positions[2] = buw::Vector4f(-0.5, -0.5, 0, 0);
	settings_.positions[3] = buw::Vector4f(0.5, -0.5, 0, 0);
	settings_.bUseUniformColor = false;
	settings_.bUseUniformPointSize = false;
	settings_.pointSize = 3.0f;

	buw::constantBufferDescription cbd;
	cbd.sizeInBytes = sizeof(SettingsBuffer);
	cbd.data = &settings_;

	settingsBuffer_ = renderSystem()->createConstantBuffer(cbd);


	// load terrain texture
	auto img1 = buw::loadImage4b("Data/CarCrash.png");

	buw::texture2DDescription td1;
	td1.width = img1.getWidth();
	td1.height = img1.getHeight();
	td1.format = buw::eTextureFormat::R8G8B8A8_UnsignedNormalizedInt_SRGB;
	td1.data = img1.getData();

	texture_ = renderSystem()->createTexture2D(td1, buw::eTextureBindType::SRV);

	buw::samplerStateDescription samplerStateDesc;
	samplerStateDesc.minificationFilter = buw::eFilter::Linear;
	samplerStateDesc.magnificationFilter = buw::eFilter::Linear;
	samplerStateDesc.mipLevelFilter = buw::eFilter::None;
	samplerStateDesc.textureAddressModeU = buw::eTextureAddressMode::Clamp;
	samplerStateDesc.textureAddressModeV = buw::eTextureAddressMode::Clamp;
	samplerStateDesc.textureAddressModeW = buw::eTextureAddressMode::Clamp;
	sampler_ = renderSystem()->createSampler(samplerStateDesc);
}

void BillboardEffect::v_render()
{
	buw::ReferenceCounted<buw::ITexture2D> renderTarget = renderSystem()->getBackBufferTarget();
	setRenderTarget(renderTarget, depthStencilMSAA_);
	setViewport(viewport_);
	setPipelineState(pipelineState_);
	setConstantBuffer(worldBuffer_, "WorldBuffer");
	setConstantBuffer(viewportBuffer_, "ViewportBuffer");
	setConstantBuffer(settingsBuffer_, "SettingsBuffer");

	setSampler(buw::Singleton<RenderResources>::instance().getLinearSampler(), "sampler_");
	setTexture(texture_, "texDiffuseMap");

	setVertexBuffer(vertexBuffer_);
	draw(static_cast<UINT>(vertexBuffer_->getVertexCount()));
}

void BillboardEffect::updateSettingsBuffer()
{
	settings_.positions[0] = buw::Vector4f(-0.5, 0.5, 0, 0);
	settings_.positions[1] = buw::Vector4f(0.5, 0.5, 0, 0);
	settings_.positions[2] = buw::Vector4f(-0.5, -0.5, 0, 0);
	settings_.positions[3] = buw::Vector4f(0.5, -0.5, 0, 0);

	buw::constantBufferDescription cbd;
	cbd.sizeInBytes = sizeof(SettingsBuffer);
	cbd.data = &settings_;

	settingsBuffer_->uploadData(cbd);
}


OIP_NAMESPACE_OPENINFRAPLATFORM_UI_END
