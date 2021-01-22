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

/* -*-c++-*- IfcPlusPlus - www.ifcplusplus.com  - Copyright (C) 2011 Fabian Gerold
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 *
 * - modified by Michael Kern, September 2014
*/

#pragma once
#ifndef GEOMETRYINPUTDATA_H
#define GEOMETRYINPUTDATA_H

#include <vector>
#include <array>
#include <memory>

#include "CarveHeaders.h"
#include "namespace.h"
#include "EXPRESS/EXPRESS.h"

/**********************************************************************************************/
OIP_NAMESPACE_OPENINFRAPLATFORM_CORE_IFCGEOMETRYCONVERTER_BEGIN


//! \brief Class to hold input data of one IFC geometric representation item.
class ItemData {
public:
	std::vector<std::shared_ptr<carve::input::PolyhedronData>>	closed_polyhedrons;
	std::vector<std::shared_ptr<carve::input::PolyhedronData>>	open_polyhedrons;
	std::vector<std::shared_ptr<carve::input::PolyhedronData>>	open_or_closed_polyhedrons;
	std::vector<std::shared_ptr<carve::input::PolylineSetData>> polylines;
	std::vector<std::shared_ptr<carve::mesh::MeshSet<3>>>		meshsets;
	void createMeshSetsFromClosedPolyhedrons();

	void append(const std::shared_ptr<ItemData>& other)
	{
		std::copy(other->closed_polyhedrons.begin(),         other->closed_polyhedrons.end(),         std::back_inserter(this->closed_polyhedrons));
		std::copy(other->open_polyhedrons.begin(),           other->open_polyhedrons.end(),           std::back_inserter(this->open_polyhedrons));
		std::copy(other->open_or_closed_polyhedrons.begin(), other->open_or_closed_polyhedrons.end(), std::back_inserter(this->open_or_closed_polyhedrons));
		std::copy(other->polylines.begin(),                  other->polylines.end(),                  std::back_inserter(this->polylines));
		std::copy(other->meshsets.begin(),                   other->meshsets.end(),                   std::back_inserter(this->meshsets));
	}
};

template<
	class IfcEntityTypesT
>
class ShapeInputDataT {
public:
	ShapeInputDataT()
	{
	}

	~ShapeInputDataT()
	{
	}

	oip::EXPRESSReference<typename IfcEntityTypesT::IfcProduct>			ifc_product;
	oip::EXPRESSReference<typename IfcEntityTypesT::IfcRepresentation>	representation;
	std::vector<std::shared_ptr<ItemData>>								vec_item_data;
};

OIP_NAMESPACE_OPENINFRAPLATFORM_CORE_IFCGEOMETRYCONVERTER_END


#endif
