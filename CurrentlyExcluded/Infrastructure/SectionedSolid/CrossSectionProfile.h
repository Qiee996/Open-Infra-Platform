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

#ifndef __CROSSSECTIONPROFILE_H_ED9B3EE6762342F2879261C6DDBD40A0__
#define __CROSSSECTIONPROFILE_H_ED9B3EE6762342F2879261C6DDBD40A0__

#include <OpenInfraPlatform/Infrastructure/OIPInfrastructure.h>
#include <OpenInfraPlatform/Infrastructure/namespace.h>

#include <BlueFramework/Core/Math/Vector.h>
#include <BlueFramework/Rasterizer/vertex.h>

#include <memory>
#include <vector>

namespace OpenInfraPlatform {
	namespace IFC4X1 {
		class IfcArbitraryClosedProfileDef;
		class IfcAsymmetricIShapeProfileDef;
		class IfcCircleProfileDef;
		class IfcRectangleProfileDef;
	}
}

OIP_NAMESPACE_OPENINFRAPLATFORM_INFRASTRUCTURE_BEGIN

namespace SectionedSolid {

	class BLUEINFRASTRUCTURE_API CrossSectionProfile {
	public:
		struct Vertex {
			Vertex(buw::Vector2d const& position, buw::Vector2d const& normal) : position(position), normal(normal) {
			}

			buw::Vector2d position;
			buw::Vector2d normal;
		};

		explicit CrossSectionProfile(std::shared_ptr<IFC4X1::IfcArbitraryClosedProfileDef> csp);
		explicit CrossSectionProfile(std::shared_ptr<IFC4X1::IfcAsymmetricIShapeProfileDef> csp);
		explicit CrossSectionProfile(std::shared_ptr<IFC4X1::IfcCircleProfileDef> csp);
		explicit CrossSectionProfile(std::shared_ptr<IFC4X1::IfcRectangleProfileDef> csp);
		virtual ~CrossSectionProfile();

		// the first outmost element is the outer curve, the rest the inner curves (may be non-existent)
		std::vector<std::vector<std::vector<Vertex>>> segments;
	};

} // namespace SectionedSolid

OIP_NAMESPACE_OPENINFRAPLATFORM_INFRASTRUCTURE_END

#endif // __CROSSSECTIONPROFILE_H_ED9B3EE6762342F2879261C6DDBD40A0__
