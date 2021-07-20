#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <Camera.h>
#include <GLLib.h>
#include <Utils.h>
#include <Config.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>
#include <tuple>
#include <filesystem>
#include <cstdio>
#include <string>
#include <string_view>
struct ImageData {
	using  Pixels = std::shared_ptr<unsigned char[]>;
	int    width;
	int    height;
	Pixels pixels;
public:
	auto Clone() const -> ImageData{
		ImageData image;
		image.width  = width;
		image.height = height;
		image.pixels = std::shared_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
		return image;
	}
};
int main() {
	std::string path = CV_FINAL_DATA_DIR"/jelly_beans/rectified/";
	auto imageSCount = 0;
	auto imageTCount = 0;
	for (const std::filesystem::directory_entry& dir : std::filesystem::directory_iterator(path))
	{
		auto filepath = dir.path().string();
		auto filename = dir.path().filename().string();
		int x, y;
		float camX, camY;
		sscanf(filename.c_str(), "out_%d_%d_%f_%f_.png", &x, &y, &camX, &camY);
		imageSCount = std::max(imageSCount, x);
		imageTCount = std::max(imageTCount, y);
	}
	imageSCount    += 1;
	imageTCount    += 1;
	auto imageDatas = std::vector<ImageData>(imageSCount*imageTCount);
	for (const std::filesystem::directory_entry& dir : std::filesystem::directory_iterator(path))
	{
		auto filepath = dir.path().string();
		auto filename = dir.path().filename().string();
		int x, y, width, height, comp;
		float camX, camY;
		sscanf(filename.c_str(), "out_%d_%d_%f_%f_.png", &x, &y, &camX, &camY);
		std::cout << "\r" << x << "," << y << "," << camX << "," << camY << std::flush;
		auto* pixels = stbi_load(filepath.c_str(), &width, &height, &comp, 4);
		imageDatas[imageSCount * y + x].width  = width;
		imageDatas[imageTCount * y + x].height = height;
		imageDatas[imageTCount * y + x].pixels = std::shared_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
		std::memcpy(imageDatas[imageTCount * y + x].pixels.get(), pixels, sizeof(unsigned char) * width * height * 4);
	}
	auto& imageData0   = imageDatas[8];
	auto& imageData1   = imageDatas[8+imageSCount];
	auto  imageData2   = ImageData{};
	{
		imageData2.width  = imageData0.width;
		imageData2.height = imageData0.height;
		imageData2.pixels = std::shared_ptr<unsigned char[]>(new unsigned char[4 * imageData2.width * imageData2.height]);
		for (int i = 0; i < 4 * imageData2.width * imageData2.height; ++i) {
			imageData2.pixels[i] = 255;
		}
	}

	int   patchSizeX   = 10;
	int   patchSizeY   = 10;
	int   searchRangeX = 50;
	int   searchRangeY = 50;
	{
		for (int j1 = 0; j1 < imageData0.height - patchSizeY; j1 += patchSizeY) {
			std::cout << "\r" << j1 << std::flush;
			for (int i1 = 0; i1 < imageData0.width - patchSizeX; i1 += patchSizeX) {
				int    bestX = -1;
				int    bestY = -1;
				double bestVal = FLT_MAX;
				double prevVal = 0.0f;
				for (int j2 = std::max(j1 - searchRangeY, 0); j2 < std::min(j1 + searchRangeY, imageData1.height - patchSizeY); ++j2)
				//int j2 = j1;
				{
					//for (int i2 = std::max(i1 - searchRangeX, 0); i2 < std::min(i1 + searchRangeX, imageData1.width - patchSizeX); ++i2)
					int i2 = i1;
					{
						double val = 0.0f;
						for (int t = 0; t < patchSizeY; ++t) {
							for (int s = 0; s < patchSizeX; ++s) {
								int  index1 = imageData0.width * (t + j1) + s + i1;
								int  index2 = imageData1.width * (t + j2) + s + i2;
								unsigned char p1[3] = {
									imageData0.pixels[4 * index1 + 0],
									imageData0.pixels[4 * index1 + 1],
									imageData0.pixels[4 * index1 + 2],
								};
								unsigned char p2[3] = {
									imageData1.pixels[4 * index2 + 0],
									imageData1.pixels[4 * index2 + 1],
									imageData1.pixels[4 * index2 + 2],
								};
								double l1 = 0.299 * double(p1[0]) + 0.587 * double(p1[1]) + 0.114 * double(p1[2]);
								double l2 = 0.299 * double(p2[0]) + 0.587 * double(p2[1]) + 0.114 * double(p2[2]);
								val += std::pow(l1 - l2, 2.0f);
							}
						}
						val /= static_cast<double>(patchSizeX * patchSizeY);
						if (bestVal > val) {
							bestX = i2;
							bestY = j2;
							bestVal = val;
						}
					}
				}
				for (int t = 0; t < patchSizeY; ++t) {
					for (int s = 0; s < patchSizeX; ++s) {
						float depthX = std::pow(1.0f/(1.0f + (static_cast<float>(abs(bestX- i1))*static_cast<float>(imageSCount)/ static_cast<float>(imageData0.width ))), 2.2f);
						float depthY = std::pow(1.0f/(1.0f + (static_cast<float>(abs(bestY- j1))*static_cast<float>(imageTCount)/ static_cast<float>(imageData0.height))), 2.2f);
						imageData2.pixels[4 * (imageData0.width * (j1 + t) + i1 + s) + 0] = 255.99f * depthY;
						imageData2.pixels[4 * (imageData0.width * (j1 + t) + i1 + s) + 1] = 255.99f * depthY;
						imageData2.pixels[4 * (imageData0.width * (j1 + t) + i1 + s) + 2] = 255.99f * depthY;
						imageData2.pixels[4 * (imageData0.width * (j1 + t) + i1 + s) + 3] = 255;
					}
				}
				//std::cout << "(" << i1 << "," << j1 << ")->(" << bestX << "," << bestY << "," << bestVal << ")\n";
			}
		}
	}
	
	stbi_write_png("tekitou11.png", imageData2.width, imageData2.height, 4, imageData2.pixels.get(), 4 * imageData2.width);
}