#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <float.h>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include "raytracing.h"

//temporary variables
//these are only used to illustrate
//a simple debug drawing. A ray
Vec3Df testRayOrigin;
Vec3Df testRayDestination;

struct BoundingBox;

Vec3Df sunVector;
rgb sunColor;

float pitchAngle = 0;
float yawAngle = 0;

std::vector<Vec3Df> meshPoints;

BoundingBox * mainBox;

// Specifies the distance of the sun from the origin
// This is usefull if the sun is drawn inside objects in the debug view
float sunDist = 4;

Vec3Df calculateSunVector();

rgb sunVectorToRgb(Vec3Df sunVector);

void calculateSun();

//use this function for any preprocessing of the mesh.
void init()
{
	//load the mesh file
	//please realize that not all OBJ files will successfully load.
	//Nonetheless, if they come from Blender, they should, if they
	//are exported as WavefrontOBJ.
	//PLEASE ADAPT THE LINE BELOW TO THE FULL PATH OF THE dodgeColorTest.obj
	//model, e.g., "C:/temp/myData/GraphicsIsFun/dodgeColorTest.obj",
	//otherwise the application will not load properly
    MyMesh.loadMesh("models/cube.obj", true);
	MyMesh.computeVertexNormals();
    meshPoints = getVerticePoints(MyMesh.vertices);
	//one first move: initialize the first light source
	//at least ONE light source has to be in the scene!!!
	//here, we set it to the current location of the camera

    calculateSun();
	mainBox = &BoundingBox(MyMesh.triangles);
	mainBox = split(mainBox, 500);
	
}

/**
 * True if the vector is the zero vector
 */
bool isZero(Vec3Df v) {
    return v[0] == 0 && v[1] == 0 && v[2] == 0;
}

void calculateSun() {
    sunVector = calculateSunVector();
    MyLightPositions.clear();
    MyLightPositions.push_back(sunVector);
    sunVector.normalize();
    sunColor = sunVectorToRgb(sunVector);
}

//return the color of your pixel.
Vec3Df performRayTracing(Ray ray)
{
	Vec3Df result;
	if (trace(ray, 0, result)) return result;
	return Vec3Df(0, 0, 0);
}

bool trace(Ray ray, int level, Vec3Df& result) {
	Intersection intersection;
	Vec3Df intersect;
	
	std::vector<Triangle *> triangles = intersectingTriangles(mainBox, ray);

	for (Triangle * triangle : triangles) {
        if (intersectionPoint(ray, meshPoints, (*triangle), intersect)) {
			float distance = (intersect - ray.origin).getLength();
			if (intersection.distance > distance) {
				intersection.distance = distance;
				intersection.triangle = triangle;
				intersection.intersect = intersect;
				intersection.normal =
					Vec3Df::crossProduct(MyMesh.vertices[(*triangle).v[1]].p - MyMesh.vertices[(*triangle).v[0]].p,
										MyMesh.vertices[(*triangle).v[2]].p - MyMesh.vertices[(*triangle).v[0]].p);
				intersection.normal.normalize();
			}
        }
    }

	if (intersection.distance == FLT_MAX) return false;

	result = shade(intersection, level);

	return true;
}

Vec3Df diffuse(const Vec3Df & vertexPos, Vec3Df & normal, Material* material, Vec3Df lightPos) {
	Vec3Df diffuse = Vec3Df(0,0,0);
	normal.normalize();
	lightPos.normalize();

    Vec3Df lightVector = (lightPos - vertexPos); // Get the light position relative from the vertex position
    lightVector.normalize(); // And normalize it

	// Calculate the material colors with the sun color.
    Vec3Df materialWithSun = Vec3Df(
			(float) (sunColor.r * material->Kd().p[0]),
			(float) (sunColor.g * material->Kd().p[1]),
			(float) (sunColor.b * material->Kd().p[2])
	);

    // Calculate the diffusion (including color)
    diffuse += materialWithSun * fmax(Vec3Df::dotProduct(normal, lightPos), 0.0f);

	return diffuse;
}

Vec3Df blinnPhong(const Vec3Df & vertexPos, Vec3Df & normal, Material* material, Vec3Df lightPos) {
	Vec3Df specularity = Vec3Df(0,0,0);
	normal.normalize();
	lightPos.normalize();

    Vec3Df viewVector = (vertexPos - lightPos); // Get the view vector
    viewVector.normalize(); // And normalize it

    Vec3Df lightVector = (lightPos - vertexPos); // Get the light vector (opposite direction of viewvector.
    lightVector.normalize(); // And normalize it

    // Calculate the halfway vector
    Vec3Df halfwayVector = viewVector + lightVector;
    halfwayVector.normalize();

    float specTerm = (std::max)(Vec3Df::dotProduct(halfwayVector, normal), 0.0f);
    specTerm = pow(specTerm, material->Ns());

	Vec3Df materialWithSun = Vec3Df(
			(float) (sunColor.r * material->Ks().p[0]),
			(float) (sunColor.g * material->Ks().p[1]),
			(float) (sunColor.b * material->Ks().p[2])
	);

    specularity += materialWithSun * specTerm;

	return specularity;
}

Material getMat(int index) {
	int matIndex = MyMesh.triangleMaterials[index];
	return MyMesh.materials[matIndex];
}

Vec3Df shade(Intersection intersection, int level) {
    Vec3Df refl = Vec3Df(0,0,0);
	Vec3Df refr = Vec3Df(0,0,0);
	Vec3Df result = Vec3Df(0,0,0);

	Material material = MyMesh.materials[MyMesh.triangleMaterials[intersection.triangle - &*MyMesh.triangles.begin()]];

	/* Start of shading block */
    if (material.has_Ka()) {
//        result += material.Ka();
        //TODO This makes it white ALWAYS, unsure why
    }

    if (material.has_Kd()) {
		result += diffuse(intersection.intersect, intersection.normal, &material, sunVector);
	}

	if (material.has_Ks()) {
		result += blinnPhong(intersection.intersect, intersection.normal, &material, sunVector);
	}
    /* End of shading block */

  /*  if(level<2) // && reflects
    {
        //calculate reflection vector
       refl = trace(hit, Vec3Df(0,0,0)reflection, level +1);
    }

    else if(level<2) // && refracts
    {
        //calculate refraction vector
       refr = trace(hit, Vec3Df(0,0,0)refraction, level+1);
    } */

	for (int i = 0; i < 3; i++) {
		result.p[i] = max(0.0f, min(1.0f, result.p[i]));
	}

    return result;
}

std::vector<Vec3Df> getVerticePoints(const std::vector<Vertex> &vertices) {
	std::vector<Vec3Df> points;
	for(int i=0; i<vertices.size(); ++i){
        points.push_back(vertices[i].p);
	}
	return points;
}

bool intersectionPoint(Ray ray, const std::vector<Vec3Df> &vertices, const Triangle &triangle, Vec3Df& result) {
	Vec3Df o = ray.origin;
	Vec3Df q = ray.dest - o;

	Vertex va = vertices[triangle.v[0]];
	Vertex vb = vertices[triangle.v[1]];
	Vertex vc = vertices[triangle.v[2]];

	Vec3Df a = va.p - o;
	Vec3Df b = vb.p - o;
	Vec3Df c = vc.p - o;

	Vec3Df cq = Vec3Df::crossProduct(q, c);

	float x = Vec3Df::dotProduct(b, cq);
	if (x < FLT_EPSILON) return false;
	float y = -Vec3Df::dotProduct(a, cq);
	if (y < FLT_EPSILON) return false;
	float z = Vec3Df::dotProduct(q, Vec3Df::crossProduct(a, b));
	if (z < FLT_EPSILON) return false;

	float d = 1.0f / (x + y + z);

	result = Vec3Df(x*d, y*d, z*d);

	return true;
}

void yourDebugDraw()
{
	float lightPosition[4] = {sunVector[0], sunVector[1], sunVector[2], 0};
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	rgb sunColor = sunVectorToRgb(sunVector);

	float lightColor[4] = {sunColor.r, sunColor.g, sunColor.b, 0.5};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);

	float specularColor[4] = {1, 1, 1, 0.5};
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);

	//draw open gl debug stuff
	//this function is called every frame

	//let's draw the mesh
	MyMesh.draw();

	//let's draw the lights in the scene as points
	glPushAttrib(GL_ALL_ATTRIB_BITS); //store all GL attributes
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);
	glPointSize(10);
	glBegin(GL_POINTS);
	for (int i=0;i<MyLightPositions.size();++i)
		glVertex3fv(MyLightPositions[i].pointer());
	glEnd();
	glPopAttrib();//restore all GL attributes
	//The Attrib commands maintain the state.
	//e.g., even though inside the two calls, we set
	//the color to white, it will be reset to the previous
	//state after the pop.


	//as an example: we draw the test ray, which is set by the keyboard function
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
	glColor3f(0,1,1);
	glVertex3f(testRayOrigin[0], testRayOrigin[1], testRayOrigin[2]);
	glColor3f(0,0,1);
	glVertex3f(testRayDestination[0], testRayDestination[1], testRayDestination[2]);
	glEnd();
	glPointSize(10);
	glBegin(GL_POINTS);
	glVertex3fv(MyLightPositions[0].pointer());
	glEnd();
	glPopAttrib();

	//draw whatever else you want...
	////glutSolidSphere(1,10,10);
	////allows you to draw a sphere at the origin.
	////using a glTranslate, it can be shifted to whereever you want
	////if you produce a sphere renderer, this
	////triangulated sphere is nice for the preview
}

const float ANGLE_STEP = (float) (2 * M_PI) / 72;
const hsv fullLight = {
	49,
	45,
	100
};

const hsv duskLight = {
	353,
	77,
	72
};

Vec3Df calculateSunVector() {
	return Vec3Df(cos(yawAngle) * sin(pitchAngle), cos(pitchAngle), -sin(yawAngle) * sin(pitchAngle)) * sunDist;
}

rgb sunVectorToRgb(Vec3Df sunVector) {
	sunVector.normalize();

	double angle = acos(Vec3Df::dotProduct(sunVector, Vec3Df(0, 1, 0)));

	double interpolation = angle / (M_PI * 0.5);

	double hueDiff = (duskLight.h - 360) - fullLight.h;
	double satDiff = duskLight.s - fullLight.s;
	double valueDiff = duskLight.v - fullLight.v;

	hsv currentLight = {
			(int) ((fullLight.h + (hueDiff * interpolation)) + 360) % 360,
			(fullLight.s + (satDiff * interpolation)) / 100,
			(fullLight.v + (valueDiff * interpolation)) / 100
	};
	return hsv2rgb(currentLight);
}

//yourKeyboardFunc is used to deal with keyboard input.
//t is the character that was pressed
//x,y is the mouse position in pixels
//rayOrigin, rayDestination is the ray that is going in the view direction UNDERNEATH your mouse position.
//
//A few keys are already reserved:
//'L' adds a light positioned at the camera location to the MyLightPositions vector
//'l' modifies the last added light to the current
//    camera position (by default, there is only one light, so move it with l)
//    ATTENTION These lights do NOT affect the real-time rendering.
//    You should use them for the raytracing.
//'r' calls the function performRaytracing on EVERY pixel, using the correct associated ray.
//    It then stores the result in an image "result.ppm".
//    Initially, this function is fast (performRaytracing simply returns
//    the target of the ray - see the code above), but once you replaced
//    this function and raytracing is in place, it might take a
//    while to complete...
void yourKeyboardFunc(char t, int x, int y, Ray ray)
{

	//here, as an example, I use the ray to fill in the values for my upper global ray variable
	//I use these variables in the debugDraw function to draw the corresponding ray.
	//try it: Press a key, move the camera, see the ray that was launched as a line.
	testRayOrigin=ray.origin;
	testRayDestination=ray.dest;

	// do here, whatever you want with the keyboard input t.
	if (t == 'w') {
		pitchAngle += ANGLE_STEP;
        calculateSun();
	} else if (t == 's') {
		pitchAngle -= ANGLE_STEP;
        calculateSun();
	} else if (t == 'a') {
		yawAngle += ANGLE_STEP;
        calculateSun();
	} else if (t == 'd') {
		yawAngle -= ANGLE_STEP;
        calculateSun();
	}
	//...

	std::cout<<t<<" pressed! The mouse was in location "<<x<<","<<y<<"!"<<std::endl;
}

Vec3Df intersectionWithPlane(const Vec3Df & planeNormal, Vec3Df & planePoint)
{
	Vec3Df dir = testRayDestination;
	Vec3Df origin = testRayOrigin;
	dir.normalize();
	origin.normalize();
	planePoint.normalize();

	float t = Vec3Df::dotProduct((planePoint - origin), planeNormal) / Vec3Df::dotProduct(dir, planeNormal);
	Vec3Df res = origin + t*dir;
	return res;
}

Vec3Df intersectionWithSphere(Ray ray, const float & radius)
{
	Vec3Df p1 = ray.origin;
	Vec3Df p2 = ray.dest;

	float A = p2[0] - p1[0];
	float B = p2[1] - p1[1];
	float C = p2[2] - p1[2];

	float a = (A*A) + (B*B) + (C*C);
	float b = 2 * ((A*p1[0]) + (B*p1[1]) + (C*p1[2]));
	float c = (p1[0] * p1[0]) + (p1[1] * p1[1]) + (p1[2] * p1[2]) - (radius*radius);

	float t = (-b + sqrtf((b*b) - (4 * a*c))) / (2 * a);

	Vec3Df res = p1 + t*(p2 - p1);
	return res;
}
