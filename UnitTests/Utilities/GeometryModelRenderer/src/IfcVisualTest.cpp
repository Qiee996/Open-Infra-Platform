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
#include <namespace.h>
#include <IfcVisualTest.h>
#include <boost/filesystem.hpp>

using namespace testing;

void IfcVisualTest::SetUp()
{
	rendererIfc = buw::makeReferenceCounted<IfcGeometryModelRenderer>(renderSystem_);

	// make a Test directory for test-specific data
	boost::filesystem::create_directory(filePath("Test"));
}

void IfcVisualTest::TearDown()
{
	rendererIfc.reset();
}

buw::Image4b IfcVisualTest::CaptureImage()
{
	return rendererIfc->captureImage();
}

boost::filesystem::path IfcVisualTest::filePath(const std::string& relPath) const
{
	return executablePath()
		.concat("\\UnitTests\\Schemas\\")
		.concat(Schema())
		.concat("\\")
		.concat(TestName())
		.concat("\\")
		.concat(relPath);
}