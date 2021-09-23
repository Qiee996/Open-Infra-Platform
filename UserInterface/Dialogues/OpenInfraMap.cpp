/*! \verbatim
 *  \copyright	Copyright (c) 2021 Julian Amann. All rights reserved.
 *	\author		Julian Amann <julian.amann@tum.de> (https://www.cms.bgu.tum.de/en/team/amann)
 *	\brief		This file is part of the BlueFramework.
 *	\endverbatim
 */

#include "../UserInterface/Dialogues/OpenInfraMap.h"
#include <QVariant>
#include <QQuickItem>
#include "BlueFramework/Engine/ResourceManagment/download.h"

#define BOOST_SPIRIT_THREADSAFE

OpenInfraPlatform::UserInterface::OpenInfraMap::OpenInfraMap(QWidget* parent) :
	view_(nullptr),
	container_(nullptr)
{
	view_ = new QQuickView();
	container_ = QWidget::createWindowContainer(view_, parent);
	view_->setSource(QUrl("main.qml"));
	container_->setFocusPolicy(Qt::TabFocus);
	view_->setResizeMode(QQuickView::SizeRootObjectToView);
}

OpenInfraPlatform::UserInterface::OpenInfraMap::~OpenInfraMap()
{
	if (view_)
		delete view_;
	if (container_)
		delete container_;
}

QWidget* OpenInfraPlatform::UserInterface::OpenInfraMap::widget()
{
	return container_;
}

void OpenInfraPlatform::UserInterface::OpenInfraMap::show()
{
	container_->setVisible(true);
}

void OpenInfraPlatform::UserInterface::OpenInfraMap::hide()
{
	container_->setVisible(false);
}

QGeoCoordinate OpenInfraPlatform::UserInterface::OpenInfraMap::position() const
{
	return qvariant_cast<QGeoCoordinate>(view_->rootObject()->property("position"));
}

void OpenInfraPlatform::UserInterface::OpenInfraMap::position(const QGeoCoordinate& value)
{
	if (!view_->rootObject()->setProperty("position", QVariant::fromValue(value))) throw std::logic_error("setProperty failed");
}

int OpenInfraPlatform::UserInterface::OpenInfraMap::zoom() const
{
	return view_->rootObject()->property("zoomLevel").toInt();
}

void OpenInfraPlatform::UserInterface::OpenInfraMap::zoom(int value)
{
	if(!view_->rootObject()->setProperty("zoomLevel", QVariant::fromValue(value))) throw std::logic_error("setProperty failed");
}

int OpenInfraPlatform::UserInterface::OpenInfraMap::minimalZoom() const
{
	return view_->rootObject()->property("minZoomLevel").toInt();
}

int OpenInfraPlatform::UserInterface::OpenInfraMap::maximalZoom() const
{
	return view_->rootObject()->property("maxZoomLevel").toInt();
}

//taken from the javascript of http://www.deine-berge.de/umrechner_koordinaten.php
buw::Vector2d pod2wgs(const buw::Vector2d& pod)
{
	// Geographische Länge lp und Breite bp im Potsdam Datum
	double lp = pod.y();
	double bp = pod.x();

	// Quellsystem Potsdam Datum
	//  Grosse Halbachse a und Abplattung fq
	constexpr double a = 6378137.000 - 739.845;
	constexpr double fq = 3.35281066e-3 - 1.003748e-05;

	// Zielsystem WGS84 Datum
	//  Abplattung f
	constexpr double f = 3.35281066e-3;

	// Parameter fuer datum shift
	constexpr double dx = 587;
	constexpr double dy = 16;
	constexpr double dz = 393;

	// Quadrat der ersten numerischen Exzentrizitaet in Quell- und Zielsystem
	constexpr double e2q = (2 * fq - fq * fq);
	constexpr double e2 = (2 * f - f * f);

	// Breite und Laenge in Radianten
	constexpr double pi = M_PI;
	double b1 = bp * (pi / 180);
	double l1 = lp * (pi / 180);

	// Querkruemmungshalbmesser nd
	double nd = a / sqrt(1 - e2q * sin(b1) * sin(b1));

	// Kartesische Koordinaten des Quellsystems Potsdam
	double xp = nd * cos(b1) * cos(l1);
	double yp = nd * cos(b1) * sin(l1);
	double zp = (1 - e2q) * nd * sin(b1);

	// Kartesische Koordinaten des Zielsystems (datum shift) WGS84
	double x = xp + dx;
	double y = yp + dy;
	double z = zp + dz;

	// Berechnung von Breite und Laenge im Zielsystem
	double rb = sqrt(x * x + y * y);
	double b2 = (180 / pi) * atan((z / rb) / (1 - e2));

	double l2 = (180 / pi) * atan2(y, x);

	return buw::Vector2d(b2, l2);
}

//taken from the javascript of http://www.deine-berge.de/umrechner_koordinaten.php
buw::Vector2d gk2wgs(const buw::Vector2d& gk)
{
	double rw = gk.x();
	double hw = gk.y();

	//  Potsdam Datum / Bessel Ellipsoid
	// Grosse Halbachse a und Abplattung f
	constexpr double a = 6377397.155;
	constexpr double f = 3.34277321e-3; // = 1/299.1528128 
	constexpr double pi = M_PI;

	// Polkruemmungshalbmesser c
	constexpr double c = a / (1 - f);

	// Quadrat der zweiten numerischen Exzentrizitaet
	constexpr double ex2 = (2 * f - f * f) / ((1 - f) * (1 - f));
	constexpr double ex4 = ex2 * ex2;
	constexpr double ex6 = ex4 * ex2;
	constexpr double ex8 = ex4 * ex4;

	// Koeffizienten zur Berechnung der geographischen Breite aus gegebener
	// Meridianbogenlaenge
	constexpr double e0 = c * (pi / 180) * (1 - 3 * ex2 / 4 + 45 * ex4 / 64 - 175 * ex6 / 256 + 11025 * ex8 / 16384);
	constexpr double f2 = (180 / pi) * (3 * ex2 / 8 - 3 * ex4 / 16 + 213 * ex6 / 2048 - 255 * ex8 / 4096);
	constexpr double f4 = (180 / pi) * (21 * ex4 / 256 - 21 * ex6 / 256 + 533 * ex8 / 8192);
	constexpr double f6 = (180 / pi) * (151 * ex6 / 6144 - 453 * ex8 / 12288);

	// Geographische Breite bf zur Meridianbogenlaenge gf = hw
	double sigma = hw / e0;
	double sigmr = sigma * pi / 180;
	double bf = sigma + f2 * sin(2 * sigmr) + f4 * sin(4 * sigmr) + f6 * sin(6 * sigmr);

	// Breite bf in Radianten
	double br = bf * pi / 180;
	double tan1 = tan(br);
	double tan2 = tan1 * tan1;
	double tan4 = tan2 * tan2;

	double cos1 = cos(br);
	double cos2 = cos1 * cos1;

	double etasq = ex2 * cos2;

	// Querkruemmungshalbmesser nd
	double nd = c / sqrt(1 + etasq);
	double nd2 = nd * nd;
	double nd4 = nd2 * nd2;
	double nd6 = nd4 * nd2;
	double nd3 = nd2 * nd;
	double nd5 = nd4 * nd;

	//  Laengendifferenz dl zum Bezugsmeridian lh
	double kz = static_cast<int>(rw / 1e6);
	double lh = kz * 3;
	double dy = rw - (kz * 1e6 + 500000);
	double dy2 = dy * dy;
	double dy4 = dy2 * dy2;
	double dy3 = dy2 * dy;
	double dy5 = dy4 * dy;
	double dy6 = dy3 * dy3;

	double b2 = -tan1 * (1 + etasq) / (2 * nd2);
	double b4 = tan1 * (5 + 3 * tan2 + 6 * etasq * (1 - tan2)) / (24 * nd4);
	double b6 = -tan1 * (61 + 90 * tan2 + 45 * tan4) / (720 * nd6);

	double l1 = 1 / (nd * cos1);
	double l3 = -(1 + 2 * tan2 + etasq) / (6 * nd3 * cos1);
	double l5 = (5 + 28 * tan2 + 24 * tan4) / (120 * nd5 * cos1);

	// Geographischer Breite bp und Laenge lp als Funktion von Rechts- und Hochwert
	double lat = bf + (180 / pi) * (b2 * dy2 + b4 * dy4 + b6 * dy6);
	double lng = lh + (180 / pi) * (l1 * dy + l3 * dy3 + l5 * dy5);

	return pod2wgs(buw::Vector2d(lat, lng));
}

void OpenInfraPlatform::UserInterface::OpenInfraMap::reposition(const buw::Vector3d& vector, GeoCoordinateSystem coordinateSystem)
{
	QGeoCoordinate coord;
	switch (coordinateSystem)
	{
	case GeoCoordinateSystem::WGS84:
		coord = QGeoCoordinate(vector.x(), vector.y());
		break;
	case GeoCoordinateSystem::GaussKrueger: {
		buw::Vector2d wgs = gk2wgs(buw::Vector2d(vector.x(), vector.y()));
		coord = QGeoCoordinate(wgs.x(), wgs.y());
		BLUE_LOG(trace) << 
			"converted Gauss-Krueger(" << std::fixed << vector.x() << ", " << 
			std::fixed << vector.y() << ") to WGS84(" << std::fixed << wgs.x() << ", " << std::fixed << wgs.y() << ")";
		break;
	}
	default:
		throw std::out_of_range("unknown GeoCoordinateSystem");
	}
	//check coordinate
	if (!coord.isValid()) 
	{ 
		BLUE_LOG(error) <<
			"invalid GeoCoordinate";
		throw std::logic_error("invalid GeoCoordinate");
	}
	BLUE_LOG(trace) << "Before position";
	this->position(coord);
	BLUE_LOG(trace) << "After position";

	BLUE_LOG(trace) << "Before zoom";
	this->zoom(this->maximalZoom() - (this->maximalZoom() - this->minimalZoom()) / 6);
	BLUE_LOG(trace) << "After zoom";

	BLUE_LOG(trace) << "Leaving reposition";
}
