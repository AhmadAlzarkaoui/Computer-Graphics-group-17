#include "BoundingBox.h" 
#include <limits>
#include <algorithm>

BoundingBox::BoundingBox() {
	init(Vec3Df(0, 0, 0), Vec3Df(0, 0, 0));
}

BoundingBox::BoundingBox(const Mesh& mesh) {
	std::vector<Vertex> vertices = mesh.vertices;
	float lowestX = std::numeric_limits<float>::max();
	float lowestY = lowestX;
	float lowestZ = lowestX;

	float highestX = std::numeric_limits<float>::min();
	float highestY = highestX;
	float highestZ = highestX;

	for (std::vector<Vertex>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
		Vec3Df vertex = (*it).p;
		lowestX = std::min(lowestX, vertex[0]);
		highestX = std::max(highestX, vertex[0]);

		lowestY = std::min(lowestY, vertex[1]);
		highestY = std::max(highestY, vertex[1]);

		lowestZ = std::min(lowestZ, vertex[2]);
		highestZ = std::max(highestZ, vertex[2]);		
	}

	Vec3Df origin = Vec3Df(lowestX, lowestY, lowestZ);
	Vec3Df farCorner = Vec3Df(highestX, highestY, highestZ);
	init(origin, farCorner - origin);
}

BoundingBox::BoundingBox(const Vec3Df& origin, const Vec3Df& dimensions) {
	init(origin, dimensions);
}

void BoundingBox::init(const Vec3Df& origin, const Vec3Df& dimensions) {
	this->origin = origin;
	this->dimensions = dimensions;
}

std::vector<Vec3Df> BoundingBox::getVertices() {
	std::vector<Vec3Df> vertices;
	vertices.push_back(origin);
	vertices.push_back(Vec3Df(origin[0], origin[1], origin[2] + dimensions[2]));
	vertices.push_back(Vec3Df(origin[0], origin[1] + dimensions[1], origin[2]));
	vertices.push_back(Vec3Df(origin[0], origin[1] + dimensions[1], origin[2] + dimensions[2]));
	vertices.push_back(Vec3Df(origin[0] + dimensions[0], origin[1], origin[2]));
	vertices.push_back(Vec3Df(origin[0] + dimensions[0], origin[1], origin[2] + dimensions[2]));
	vertices.push_back(Vec3Df(origin[0] + dimensions[0], origin[1] + dimensions[1], origin[2]));
	vertices.push_back(origin + dimensions);
	return vertices;
}

std::vector<int> BoundingBox::getDrawingIndices() {
	std::vector<int> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(4);

	indices.push_back(6);
	indices.push_back(4);
	indices.push_back(2);

	indices.push_back(6);
	indices.push_back(2);
	indices.push_back(3);

	indices.push_back(7);
	indices.push_back(6);
	indices.push_back(3);

	indices.push_back(5);
	indices.push_back(4);
	indices.push_back(6);

	indices.push_back(7);
	indices.push_back(5);
	indices.push_back(6);

	indices.push_back(0);
	indices.push_back(4);
	indices.push_back(1);

	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(1);

	indices.push_back(7);
	indices.push_back(3);
	indices.push_back(1);

	indices.push_back(1);
	indices.push_back(5);
	indices.push_back(7);
	return indices;
}