#include "utilities/image.h"

float distFunc(Vec3<float> pos)
{
    return pos.length() - 0.5f;
}

int main(){
    int height = 2160;
    int width = 3840;
    float aspect = (float)width / height;
    // unsigned char image[height][width][bytesPerPixel];
    Color ** image = new Color*[height];
    for (int i = 0; i < height; i++) {
        image[i] = new Color[width * bytesPerPixel];
    }
    char imageFileName[] = "bitmapImage.bmp";
    int i, j;

    // raymarch part here
    Vec3<float> cameraOrigin(2.0f, 2.0f, 2.0f);

    Vec3<float> cameraTarget(0.0f, 0.0f, 0.0f);

    Vec3<float> upDirection(0.0f, 1.0f, 0.0f);

    Vec3<float> cameraDir = (cameraTarget - cameraOrigin).normalize();
    Vec3<float> cameraRight = upDirection.cross(cameraOrigin).normalize();
    Vec3<float> cameraUp = cameraDir.cross(cameraRight);
    Vec3<float> lightDir = Vec3<float>(0, 0.5, 1.0).normalize();

    for(i=0; i<height; i++){
        for(j=0; j<width; j++) {
            Vec3<float> rayDir = (cameraRight * (((float)j / width * 2 - 1) * aspect) + cameraUp * ((float)i / height * 2 - 1) + cameraDir).normalize();
            const int MAX_ITER = 100; // 100 is a safe number to use, it won't produce too many artifacts and still be quite fast
            const float MAX_DIST = 20.0; // Make sure you change this if you have objects farther than 20 units away from the camera
            const float EPSILON = 0.001; // At this distance we are close enough to the object that we have essentially hit it

            float totalDist = 0.0;
            Vec3<float> pos = cameraOrigin;
            float dist = EPSILON;

            for (int t = 0; t < MAX_ITER; t++)
            {
                // Either we've hit the object or hit nothing at all, either way we should break out of the loop
                if (dist < EPSILON || totalDist > MAX_DIST)
                    break; // If you use windows and the shader isn't working properly, change this to continue;

                dist = distFunc(pos); // Evalulate the distance at the current point
                totalDist += dist;
                pos += dist * rayDir; // Advance the point forwards in the ray direction by the distance
            }
            if (dist < EPSILON) {
                Vec3<float> x(EPSILON, 0, 0);
                Vec3<float> y(0, EPSILON, 0);
                Vec3<float> z(0, 0, EPSILON);

                Vec3<float> normal = Vec3<float>(
                    distFunc(pos + x) - distFunc(pos - x),
                    distFunc(pos + y) - distFunc(pos - y),
                    distFunc(pos + z) - distFunc(pos - z)).normalize();

                float light = fmax(0, lightDir.dot(normal));
                image[i][j].color[2] = (unsigned char)((double)light * 255); ///red
                image[i][j].color[1] = (unsigned char)((double)light * 255); ///green
                image[i][j].color[0] = (unsigned char)((double)light * 255); ///blue
            }
        }
    }

    generateBitmapImage(image, height, width, imageFileName);
}
