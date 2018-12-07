#include "utilities/image.h"
#include <thread>
#include <iostream>
#include <vector>

#define WIDTH 3640
#define HEIGHT 2180

float distFunc(Vec3<float> pos)
{
    return pos.length() - 0.5f;
}

float DE(Vec3<float> pos) {
	Vec3<float> z = pos;
	float dr = 1.1;
    const float Power = 2.f;
    const float Bailout = 200;
	float r = 0.0;
	for (int i = 0; i < 1000; i++) {
		r = z.length();
		if (r>Bailout) break;
		
		// convert to polar coordinates
		float theta = acos(z[2]/r);
		float phi = atan2(z[1], z[0]);
		dr =  pow( r, Power-1.0)*Power*dr + 1.0;
		
		// scale and rotate the point
		float zr = pow( r,Power);
		theta = theta*Power;
		phi = phi*Power;
		
		// convert back to cartesian coordinates
		z = zr*Vec3<float>(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z+=pos;
	}
	return 0.5*log(r)*r/dr;
}

void printProgress(float proggres) 
{
    std::cout << "\r" << "progress: " << proggres << "              ";
}

void generateImagePart(int startX, int startY, int endX, int endY, Color ** image, float * proggres)
{
    Vec3<float> cameraOrigin(1.0f, 1.0f, 1.0f);

    Vec3<float> cameraTarget(0.0f, 0.0f, 0.0f);

    Vec3<float> upDirection(0.0f, 1.0f, 0.0f);

    Vec3<float> cameraDir = (cameraTarget - cameraOrigin).normalize();
    Vec3<float> cameraRight = upDirection.cross(cameraOrigin).normalize();
    Vec3<float> cameraUp = cameraDir.cross(cameraRight);
    Vec3<float> lightDir = Vec3<float>(0, 0.5, 1.0).normalize();

    for(int i=startY; i<endY; i++){
        for(int j=startX; j<endX; j++) {
            Vec3<float> rayDir = (cameraRight * (((float)j / WIDTH * 2 - 1) * WIDTH / (float)HEIGHT) + cameraUp * ((float)i / HEIGHT * 2 - 1) + cameraDir).normalize();
            const int MAX_ITER = 50000; // 100 is a safe number to use, it won't produce too many artifacts and still be quite fast
            const float MAX_DIST = 500.0; // Make sure you change this if you have objects farther than 20 units away from the camera
            const float EPSILON = 0.001f; // At this distance we are close enough to the object that we have essentially hit it

            float totalDist = 0.0;
            Vec3<float> pos = cameraOrigin;
            float dist = EPSILON;

            for (int t = 0; t < MAX_ITER; t++)
            {
                // Either we've hit the object or hit nothing at all, either way we should break out of the loop
                if (dist < EPSILON || totalDist > MAX_DIST)
                    break; // If you use windows and the shader isn't working properly, change this to continue;

                dist = DE(pos); // Evalulate the distance at the current point
                totalDist += dist;
                pos += dist * rayDir; // Advance the point forwards in the ray direction by the distance
            }
            if (dist < EPSILON) {
                Vec3<float> x(EPSILON, 0, 0);
                Vec3<float> y(0, EPSILON, 0);
                Vec3<float> z(0, 0, EPSILON);

                Vec3<float> normal = Vec3<float>(
                    DE(pos + x) - DE(pos - x),
                    DE(pos + y) - DE(pos - y),
                    DE(pos + z) - DE(pos - z)).normalize();

                float light = fmax(0.1, lightDir.dot(normal));
                image[i][j].color[2] = (unsigned char)((double)light * 255); ///red
                image[i][j].color[1] = (unsigned char)((double)light * 255); ///green
                image[i][j].color[0] = (unsigned char)((double)light * 255); ///blue
            }
        }
        *proggres = (float)(i + 1 - startY) / (float)(endY - startY);
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
    float progresses[maxThreads];
    for (int i = 0; i < maxThreads; i++)
    {
        std::cout << ((float)i / maxThreads) * HEIGHT << " " << WIDTH << " " << ((i + 1.0f) / maxThreads) * HEIGHT << " " << "\n";
        progresses[i] = 0.0f;
        threads.push_back(std::thread(&generateImagePart, 0, ((float)i / maxThreads) * HEIGHT, WIDTH, ((i + 1.0f) / maxThreads) * HEIGHT, image, &progresses[i]));
    }
    float proggres = 0;
    while (proggres != 1) {
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
