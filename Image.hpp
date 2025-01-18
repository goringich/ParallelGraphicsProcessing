#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <opencv2/opencv.hpp>
#include <string>
#include <stdexcept>

class Image {
  cv::Mat image;
  const std::string pathToImage;
  const std::string pathToNewImage;

public:
  // Default constructor
  Image() : image(), pathToImage(""), pathToNewImage("") {}

  // Constructor with paths
  Image(const std::string& pathToImage_, const std::string& pathToNewImage_)
      : image(), pathToImage(pathToImage_), pathToNewImage(pathToNewImage_) {}

  // Getters
  std::string getPathToImage() const { return pathToImage; }
  std::string getPathToNewImage() const { return pathToNewImage; }
  cv::Mat getImage() const {
    if (image.empty()) {
      throw std::runtime_error("Image is empty!");
    }
    return image;
  }

  // Setters
  void setImage(const cv::Mat& image_) {
    if (image_.empty()) {
      throw std::invalid_argument("Input image is empty!");
    }
    image = image_.clone(); // Clone to ensure ownership
  }

  // Deleting copy constructor and copy assignment
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
};

#endif
