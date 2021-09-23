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

#pragma once
#ifndef OpenInfraPlatform_PointCloudProcessing_PointCloudProcessing_PointCloudSection_87B20647_825A_4D6A_AA34_62736D1B2174_h
#define OpenInfraPlatform_PointCloudProcessing_PointCloudProcessing_PointCloudSection_87B20647_825A_4D6A_AA34_62736D1B2174_h

#include "namespace.h"
#include "PointCloudProcessing.h"

#include <BlueFramework/Core/Math/vector.h>
#include <BlueFramework/Core/memory.h>

#include <ccPointCloud.h>
#include <ReferenceCloud.h>

#include <tuple>

namespace OpenInfraPlatform {
	namespace PointCloudProcessing {

		// Forward declarations
		class PointCloud;

		class BLUEINFRASTRUCTURE_API PointCloudSection : public CCLib::ReferenceCloud {
		public:
			void setLength(double length);
			const double getLength() const;

			PointCloudSection(GenericIndexedCloudPersist* associatedCloud);

			PointCloudSection(PointCloudSection &other);

			virtual ~PointCloudSection();

			// Function which flags points considered as duplicate in this projection as duplicates in the associated cloud.
			int flagDuplicatePoints(const double minDistance);

			int computeLocalDensity(CCLib::GeometricalAnalysisTools::Density metric, ScalarType kernelRadius, buw::ReferenceCounted<CCLib::GenericProgressCallback> callback = nullptr);

			int computePercentiles(float kernelRadius);

			CCVector3 computeCenterOfMass();

			CCVector3 computeCenter();

			CCVector3 computeMedianCenter();

			Eigen::Matrix3d getOrientation();

			std::vector<std::pair<size_t, size_t>> computePairs(buw::PairComputationDescription desc, buw::ReferenceCounted<PointCloudSection> nextSection = nullptr);

			void resetPairs();

			std::vector<std::pair<size_t, size_t>> getPairs();

			void getAxisAlignedBoundingBox(CCVector3 &min, CCVector3 &max);

			void getObjectOrientedBoundingBox(CCVector3 &min, CCVector3 &max);

			template<typename F> void for_each(const F& function)
			{
				for(size_t i = 0; i < size(); i++)
					function(i);
			}

		private:
			// Creates a point cloud where all points are projected on the plane represented by this section.
			buw::ReferenceCounted<PointCloud> createPointCloud2D();

			buw::ReferenceCounted<PointCloud> createPointCloud3D();

			
		public:
			CCLib::DgmOctree::CellCode cellCode_;

		private:
			double length_;

			Eigen::Vector3d mainAxis_;

			std::vector<std::pair<size_t, size_t>> pairs_;
		};
	}
}

namespace buw {	
	using OpenInfraPlatform::PointCloudProcessing::PointCloudSection;
}
#endif
