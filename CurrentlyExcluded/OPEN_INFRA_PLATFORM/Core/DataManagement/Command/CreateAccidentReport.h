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

#include "OpenInfraPlatform/Infrastructure/Alignment/AlignmentModel.h"
#include "OpenInfraPlatform/Infrastructure/ProxyModel/ProxyModel.h"

#include <BlueFramework/Core/Math/vector.h>
#include <BlueFramework/Application/DataManagement/Command/ICommand.h>
#include <boost/noncopyable.hpp>
#include <vector>

namespace OpenInfraPlatform
{
	namespace DataManagement
	{
		namespace Command
		{
			class CreateAccidentReport : public buw::ICommand
			{
			public:
				 CreateAccidentReport(buw::ReferenceCounted<buw::IAlignment3D> alignment, const double station);
				virtual ~ CreateAccidentReport();

				virtual void execute();
				virtual void unexecute();

			private:
				int AccidentReportIndex_ = -1;
				buw::accidentReportDescription ca_;
				buw::ReferenceCounted<buw::IAlignment3D> alignment_;
			}; // end class CreateAccidentReport
		} // end namespace Action
	} // end namespace DataManagement
} // end namespace OpenInfraPlatform

namespace buw
{
	using OpenInfraPlatform::DataManagement::Command:: CreateAccidentReport;
}
