#include "utilities/image.h"
#include <thread>
#include <iostream>
#include <vector>

#define MINRADIUS 4.0
#define MAXRADIUS 8.0
#define BOXSCALE 3.0
#define BOXRADIUS 3.0
#define WIDTH 3840
#define HEIGHT 2180

double distFunc(Vec3<double> pos)
{
    return pos.length() - 0.5f;
}

void boxFold(Vec3<double> & v, double & boundary)
{
    for (int a = 0; a < 3; a++) {
        if (v[a] > BOXRADIUS) v[a] = BOXRADIUS * 2 - v[a];
        else if (v[a]<-BOXRADIUS) v[a] = BOXRADIUS * -2 -v[a];
    }
}

void sphereFold(Vec3<double> & v, double & derivative)
{
    double zDot = v.dot(v);
    if (zDot < MINRADIUS) {
        v *= MAXRADIUS / MINRADIUS;
        derivative *= MAXRADIUS / MINRADIUS;
    } else if (zDot < MAXRADIUS) {
        v *= MAXRADIUS / zDot;
        derivative *= MAXRADIUS / zDot;
    }
    // if m<r           m = m/r^2
    // else if m<1 m = 1/m
}

double mandelBox(Vec3<double> z)
{
	Vec3<double> offset = z;
	double dr = 1.0;
	for (int n = 0; n < 12; n++) {
		boxFold(z,dr);       // Reflect
		sphereFold(z,dr);    // Sphere Inversion

 		
        z= BOXSCALE * z + offset;  // Scale & Translate
        dr = dr * abs(BOXSCALE) + 1.0;
		// sphereFold(z,dr);    // Sphere Inversion
	}
	double r = z.length();
	return r/abs(dr);
}

double mandelBulb(Vec3<double> pos) {
	Vec3<double> z = pos;
	double dr = 1;
    const double Power = 6.0f;
    const double Bailout = 200;
	double r = 0.0;
	for (int i = 0; i < 10000; i++) {
		r = z.length();
		if (r>Bailout) break;
		
		// convert to polar coordinates
		double theta = acos(z[2]/r);
		double phi = atan2(z[1], z[0]);
		dr =  pow( r, Power-1.0)*Power*dr + 1.0;
		
		// scale and rotate the point
		double zr = pow( r,Power);
		theta = theta*Power;
		phi = phi*Power;
		
		// convert back to cartesian coordinates
		// z = zr*Vec3<double>(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
        z = zr*Vec3<double>( cos(theta)*cos(phi), cos(theta)*sin(phi), sin(theta) );
		z+=pos;
	}
	return 0.5*log(r)*r/dr;
}

double currentDistanceEstimator(Vec3<double> pos)
{
    return mandelBox(pos);
}


void printProgress(double proggres) 
{
    std::cout << "\r" << "progress: " << (int)(proggres * 100)  << "%";
}

void generateImagePart(int startX, int startY, int endX, int endY, Color ** image, double * proggres)
{
    Vec3<double> cameraOrigin(0.5, 0.0, 15);

    Vec3<double> cameraTarget(0, 0.0, 0.0);

    Vec3<double> upDirection(0.0, 1.0, 0.0);

    Vec3<double> cameraDir = (cameraTarget - cameraOrigin).normalize();
    Vec3<double> cameraRight = upDirection.cross(cameraOrigin).normalize();
    Vec3<double> cameraUp = cameraDir.cross(cameraRight);
    Vec3<double> lightDir = Vec3<double>(0, 0.5, 1.0).normalize();

    for(int i=startY; i<endY; i++){
        for(int j=startX; j<endX; j++) {
            Vec3<double> rayDir = (cameraRight * (((double)j / WIDTH * 2 - 1) * WIDTH / (double)HEIGHT) + cameraUp * ((double)i / HEIGHT * 2 - 1) + cameraDir).normalize();
            const int MAX_ITER = 999999; // 100 is a safe number to use, it won't produce too many artifacts and still be quite fast
            const double MAX_DIST = 200.0; // Make sure you change this if you have objects farther than 20 units away from the camera
            const double EPSILON = 0.0125; // At this distance we are close enough to the object that we have essentially hit it

            double totalDist = 0.0;
            Vec3<double> pos = cameraOrigin;
            double dist = EPSILON;

            for (int t = 0; t < MAX_ITER; t++)
            {
                // Either we've hit the object or hit nothing at all, either way we should break out of the loop
                if (dist < EPSILON || totalDist > MAX_DIST)
                    break; // If you use windows and the shader isn't working properly, change this to continue;

                dist = currentDistanceEstimator(pos); // Evalulate the distance at the current point
                totalDist += dist;
                pos += dist * rayDir; // Advance the point forwards in the ray direction by the distance
            }
            if (dist < EPSILON) {
                Vec3<double> x(EPSILON, 0, 0);
                Vec3<double> y(0, EPSILON, 0);
                Vec3<double> z(0, 0, EPSILON);

                Vec3<double> normal = Vec3<double>(
                    currentDistanceEstimator(pos + x) - currentDistanceEstimator(pos - x),
                    currentDistanceEstimator(pos + y) - currentDistanceEstimator(pos - y),
                    currentDistanceEstimator(pos + z) - currentDistanceEstimator(pos - z)).normalize();

                double light = fmax(0.1, lightDir.dot(normal));
                image[i][j].color[2] = (unsigned char)((double)light * 255); ///red
                image[i][j].color[1] = (unsigned char)((double)light * 255); ///green
                image[i][j].color[0] = (unsigned char)((double)light * 255); ///blue
            }
        }
        *proggres = (double)(i + 1 - startY) / (double)(endY - startY);
    }
}

int main(int argc, char* argvc[]){
    if (argc != 2) return 2;
    int maxThreads = atoi(argvc[1]);
    std::cout << maxThreads << "\n";
    // unsigned char image[height][width][bytesPerPixel];
    Color ** image = new Color*[HEIGHT];
    for (int i = 0; i < HEIGHT; i++) {
        image[i] = new Color[WIDTH * bytesPerPixel];
    }
    char imageFileName[] = "bitmapImage.bmp";
    // int maxThreads = 8;
    std::vector<std::thread> threads;
    double progresses[maxThreads];
    for (int i = 0; i < maxThreads; i++)
    {
        std::cout << ((double)i / maxThreads) * HEIGHT << " " << WIDTH << " " << ((i + 1.0f) / maxThreads) * HEIGHT << " " << "\n";
        progresses[i] = 0.0f;
        threads.push_back(std::thread(&generateImagePart, 0, ((double)i / maxThreads) * HEIGHT, WIDTH, ((i + 1.0f) / maxThreads) * HEIGHT, image, &progresses[i]));
    }
    double proggres = 0;
    while (proggres != 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        proggres = 0;
        for (int i = 0; i < maxThreads; i++) {
            proggres += progresses[i];
        }
        proggres /= maxThreads;
        printProgress(proggres);
    }
    for (int i = 0; i < maxThreads; i++) {
        threads[i].join();
    }

    generateBitmapImage(image, HEIGHT, WIDTH, imageFileName);
    return 0;
}
