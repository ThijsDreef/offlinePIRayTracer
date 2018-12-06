#include "utilities/image.h"


int main(){
    int height = 2160;
    int width = 3840;
    // unsigned char image[height][width][bytesPerPixel];
    Color ** image = new Color*[height];
    for (int i = 0; i < height; i++) {
        image[i] = new Color[width * bytesPerPixel];
    }
    char imageFileName[] = "bitmapImage.bmp";
    int i, j;
    for(i=0; i<height; i++){
        for(j=0; j<width; j++) {
            image[i][j].color[2] = (unsigned char)((double)i / height * 255); ///red
            image[i][j].color[1] = (unsigned char)((double)j / width * 255); ///green
            image[i][j].color[0] = (unsigned char)((double)i / height * 255); ///blue
        }
    }

    generateBitmapImage(image, height, width, imageFileName);
}
