/*
	Copyright (c) 2018 Technical University of Munich
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

#pragma once
#ifndef IFCIMPORTERIMPL_H
#define IFCIMPORTERIMPL_H

#include <IfcGeometryConverter/IfcImporter.h>
#include "BlueFramework/Core/Diagnostics/log.h"

#ifdef OIP_MODULE_EARLYBINDING_IFC2X3
#include "EarlyBinding\IFC2X3\src\EMTIFC2X3EntityTypes.h"
#include "EarlyBinding\IFC2X3\src\IFC2X3Entities.h"
#endif

#ifdef OIP_MODULE_EARLYBINDING_IFC4
#include "EarlyBinding\IFC4\src\EMTIFC4EntityTypes.h"
#include "EarlyBinding\IFC4\src\IFC4Entities.h"
#endif

#ifdef OIP_MODULE_EARLYBINDING_IFC4X1
#include "EarlyBinding\IFC4X1\src\EMTIFC4X1EntityTypes.h"
#include "EarlyBinding\IFC4X1\src\IFC4X1Entities.h"
#endif

#ifdef OIP_MODULE_EARLYBINDING_IFC4X3_RC1
#include "EarlyBinding\IFC4X3_RC1\src\EMTIFC4X3_RC1EntityTypes.h"
#include "EarlyBinding\IFC4X3_RC1\src\IFC4X3_RC1Entities.h"
#endif

#include "namespace.h"
#include "PlacementConverterImpl.h"

OIP_NAMESPACE_OPENINFRAPLATFORM_CORE_IFCGEOMETRYCONVERTER_BEGIN

template <
	class IfcEntityTypesT
>
IfcImporterT<IfcEntityTypesT>::IfcImporterT<IfcEntityTypesT>()
{
	geomSettings = std::make_shared<GeometrySettings>();
	unitConverter = std::make_shared<UnitConverter<IfcEntityTypesT>>();
	georefConverter = std::make_shared<GeoreferencingConverterT<IfcEntityTypesT>>(geomSettings, unitConverter);
	repConverter = std::make_shared<RepresentationConverterT<IfcEntityTypesT>>(geomSettings, unitConverter, georefConverter);
}

template <
	class IfcEntityTypesT
>
std::shared_ptr<ShapeInputDataT<IfcEntityTypesT>> IfcImporterT<IfcEntityTypesT>::convertIfcProduct(
	const EXPRESSReference<typename IfcEntityTypesT::IfcProduct>& product
) const
{
	std::shared_ptr<ShapeInputDataT<IfcEntityTypesT>> productShape = std::make_shared<ShapeInputDataT<IfcEntityTypesT>>();
	try
	{
		carve::math::Matrix productPlacement(carve::math::Matrix::IDENT());

		// check if there's any global object placement for this product
		// if yes, then apply the placement
		if (product->ObjectPlacement) {
			std::vector<EXPRESSReference<typename IfcEntityTypesT::IfcObjectPlacement>> placementAlreadyApplied;
			productPlacement = repConverter->getPlacementConverter()->convertIfcObjectPlacement(
				product->ObjectPlacement,
				placementAlreadyApplied);
		}

		// go through all representations of the product
		if (product->Representation) {
			auto representation = product->Representation.get();
#ifdef _DEBUG
			BLUE_LOG(trace) << "Processing " << representation->getErrorLog();
#endif
			// so evaluate its geometry
			for (EXPRESSReference<typename IfcEntityTypesT::IfcRepresentation>& rep : representation->Representations) {
				// convert each shape of the represenation
#ifdef _DEBUG
				BLUE_LOG(trace) << "Processing " << rep->getErrorLog();
#endif
				repConverter->convertIfcRepresentation(rep, productPlacement, productShape);
#ifdef _DEBUG
				BLUE_LOG(trace) << "Processed " << rep->getErrorLog();
#endif
			}

			computeMeshsetsFromPolyhedrons(productShape);
#ifdef _DEBUG
			BLUE_LOG(trace) << "Processed " << representation->getErrorLog();
#endif
		}

		if (product.template isOfType<typename IfcEntityTypesT::IfcAlignment>()) {
			std::shared_ptr<ItemData> itemData(new ItemData());
			productShape->vec_item_data.push_back(itemData);
			repConverter->convertIfcGeometricRepresentationItem(
				product.template as<typename IfcEntityTypesT::IfcAlignment>()->
					Axis.as<typename IfcEntityTypesT::IfcGeometricRepresentationItem>(),
				productPlacement, itemData);
		}
	}
	catch (const oip::UnhandledException& ex)
	{
		BLUE_LOG(warning) << product->getErrorLog() + " -> unhandled error: " + std::string(ex.what());
	}
	catch (const oip::InconsistentGeometryException& ex)
	{
		BLUE_LOG(warning) << product->getErrorLog() + " -> geometry error: " + std::string(ex.what());
	}
	catch (const oip::InconsistentModellingException& ex)
	{
		BLUE_LOG(warning) << product->getErrorLog() + " -> modelling error: " + std::string(ex.what());
	}
	catch (const oip::ReferenceExpiredException& ex)
	{
		BLUE_LOG(warning) << product->getErrorLog() + " -> reference error: " + std::string(ex.what());
	}
	catch (const std::exception& ex)
	{
		BLUE_LOG(warning) << product->getErrorLog() + " -> general error: " + std::string(ex.what());
	}
	catch (...)
	{
		BLUE_LOG(warning) << product->getErrorLog() + " -> unknown error.";
		throw; // throw onwards
	}

	// return the shape data
	return productShape;
}


template <
	class IfcEntityTypesT
>
void IfcImporterT<IfcEntityTypesT>::computeMeshsetsFromPolyhedrons(
	std::shared_ptr<ShapeInputDataT<IfcEntityTypesT>> productShape
) const
{
	// now examine the opening data of the product representation
	std::vector<std::shared_ptr<ShapeInputDataT<IfcEntityTypesT>>> openingDatas;

	// check if the product is an ifcElement, if so, it may contain opening data
	if (productShape->ifc_product.template isOfType<typename IfcEntityTypesT::IfcElement>()) {
		// then collect opening data
		repConverter->convertOpenings(productShape->ifc_product.template as<typename IfcEntityTypesT::IfcElement>(), openingDatas);
	}

	// go through all shapes and convert them to meshsets
	for (auto& itemData : productShape->vec_item_data) {
		// convert closed polyhedrons to meshsets
		itemData->createMeshSetsFromClosedPolyhedrons();

		// if product is IfcElement, then subtract openings like windows, doors, etc.
		if (!openingDatas.empty()) {
			repConverter->subtractOpenings(
				productShape->ifc_product.template as<typename IfcEntityTypesT::IfcElement>(), 
				itemData, openingDatas);
		}

		// convert all open polyhedrons to meshsets
		for (auto& openPoly : itemData->open_polyhedrons) {

			if (openPoly->getVertexCount() < 3) { continue; }

			std::shared_ptr<carve::mesh::MeshSet<3>> openMeshset(openPoly->createMesh(carve::input::opts()));
			itemData->meshsets.push_back(openMeshset);
		}

		// convert all open or closed polyhedrons to meshsets
		for (auto& openClosedPoly : itemData->open_or_closed_polyhedrons) {

			if (openClosedPoly->getVertexCount() < 3) { continue; }

			std::shared_ptr<carve::mesh::MeshSet<3>> openMeshset(
				openClosedPoly->createMesh(carve::input::opts()));
			itemData->meshsets.push_back(openMeshset);
		}

		// simplify geometry of all meshsets
		for (auto& meshset : itemData->meshsets) {
			repConverter->getSolidConverter()->simplifyMesh(meshset);
		}
		// polylines are handled by rendering engine
	}
}

OIP_NAMESPACE_OPENINFRAPLATFORM_CORE_IFCGEOMETRYCONVERTER_END

#endif // IFCIMPORTERIMPL_H
