#include <thread>
#include <iostream>
#include <opencv2/opencv.hpp> 
#include "Buffer.hpp"
#include "Image.hpp"
#include <dirent.h>
#include <atomic>
#include "Time.cpp"

#define NUM_PRODUCERS 5
#define NUM_CONSUMERS 5
#define PRODUCER_DELAY_TO_PRODUCE 1
#define CONSUMER_DELAY_TO_CONSUME 3

std::atomic<int> num_producers_working(0);

void consumeImage(Buffer<Image> &buffer);
void produceImage(Buffer<Image> &buffer, std::vector<Image> &images, int start, int end);
std::vector<Image> getImagesList(const char * initialDir, const char * resultDir);

void logMessage(const std::string &message, const std::string &logFile = "log.txt") {
    std::ofstream log(logFile, std::ios_base::app);
    if (log.is_open()) {
        log << message << std::endl;
    }
}

int main(int argc, const char * argv[]) {
    LogDuration ld("1");

    // Check the number of arguments
    if (argc != 3) { 
        std::cerr << "Error: Incorrect number of arguments.\n";
        std::cerr << "Usage: buffer.out <Initial_Directory_Images_Path> <Result_Directory_Images_Path>\n";
        return -1; 
    }

    // Verify the existence of directories
    DIR *initialDir = opendir(argv[1]);
    DIR *resultDir = opendir(argv[2]);
    if (initialDir == NULL) {
        perror("Error: Invalid initial directory path");
        return -1;
    }
    if (resultDir == NULL) {
        perror("Error: Invalid result directory path");
        closedir(initialDir);
        return -1;
    }
    closedir(initialDir);
    closedir(resultDir);

    std::cout << "Executing code in main...\n";
    logMessage("Starting the main execution.");

    srand(time(NULL));
    Buffer<Image> buffer;

    // Check if the input directory contains images
    std::vector<Image> images = getImagesList(argv[1], argv[2]);
    if (images.empty()) {
        std::cerr << "Error: No valid images found in the input directory.\n";
        logMessage("No valid images found in the input directory.");
        return -1;
    }

    // Ensure there are enough images for processing
    if (images.size() < NUM_PRODUCERS) {
        std::cerr << "Error: Number of images is less than the number of producers.\n";
        logMessage("Not enough images for the number of producers.");
        return -1;
    }

    // Set up threads
    std::vector<std::thread> producers_and_consumers;
    int step = (images.size() / NUM_PRODUCERS);

    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers_and_consumers.push_back(std::thread(produceImage, 
          std::ref(buffer), std::ref(images), step * i, step * (i + 1)));
    }

    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        producers_and_consumers.push_back(std::thread(consumeImage, std::ref(buffer)));
    }

    // Wait for all threads to finish
    for (auto& t : producers_and_consumers) {
        t.join();
    }

    std::cout << "Done!\n";
    logMessage("Execution finished successfully.");
}

void consumeImage(Buffer<Image> &buffer) {
    while(num_producers_working == 0) std::this_thread::yield();

    while(num_producers_working != 0 || buffer.getSize() > 0) {
        Image img = buffer.consume(1);
        cv::Mat newImage;
        try {
            if (img.geImage().empty()) {
                logMessage("Skipping empty image.");
                continue;
            }
            bitwise_not(img.geImage(), newImage);
            cv::imwrite(img.getPathToNewImage(), newImage);
        } catch (const std::exception &e) {
            logMessage(std::string("Error during consumption: ") + e.what());
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(CONSUMER_DELAY_TO_CONSUME));
    }
}

void produceImage(Buffer<Image> &buffer, std::vector<Image> &images, int start, int end) {
    ++num_producers_working;
    for (int i = start; i < end; i++) {
        try {
            images[i].setImage(cv::imread(images[i].getPathToImage(), 1));
            if (images[i].geImage().empty()) {
                logMessage("Skipping invalid image: " + images[i].getPathToImage());
                continue;
            }
            buffer.produce(i, images[i]);
        } catch (const std::exception &e) {
            logMessage(std::string("Error during production: ") + e.what());
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(PRODUCER_DELAY_TO_PRODUCE));
    }
    --num_producers_working;
}

std::vector<Image> getImagesList(const char * initialDirName, const char * resultDirName) {
    DIR *initialDir = opendir(initialDirName);
    DIR *resultDir = opendir(resultDirName);
    std::vector<Image> images;
    if(initialDir == NULL || resultDir == NULL) {
        perror("Invalid path to directories");
        logMessage("Invalid directories passed to getImagesList.");
        return images;
    }

    struct dirent *ent;
    while ((ent = readdir (initialDir)) != NULL) {
        std::string pathToImage = "", pathToNewImage = "";
        pathToImage.append(std::string(initialDirName).append("/").append(std::string(ent->d_name)));
        pathToNewImage.append(std::string(resultDirName).append("/").append(std::string(ent->d_name)));
        std::string extension = pathToImage.substr(pathToImage.find_last_of(".") + 1);
        if(extension == "png" || extension == "jpg") {
            images.push_back(Image(pathToImage, pathToNewImage));
        }
    }
    closedir(initialDir);
    closedir(resultDir);
    logMessage("Number of images found: " + std::to_string(images.size()));
    return images;
}
