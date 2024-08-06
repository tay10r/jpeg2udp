#include "camera.h"

#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <memory>
#include <vector>

#include <stdlib.h>

namespace {

std::unique_ptr<cv::VideoCapture> g_device;

} // namespace

int
camera_init()
{
    g_device.reset(new cv::VideoCapture(/*device=*/0, cv::CAP_V4L2));
    if (!g_device->isOpened()) {
        g_device.reset();
        return -1;
    }
    return 0;
}

void
camera_shutdown()
{
    g_device.reset();
}

unsigned char*
camera_read(const int header_size, int* size)
{
    cv::Mat frame;

    g_device->read(frame);
    if (frame.empty()) {
        return NULL;
    }

    cv::Mat gray_frame;

    cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
    
    // Set JPEG compression parameters
    const int quality = 25;
    std::vector<int> compression_params;
    //compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(quality);

    // Encode the image
    std::vector<unsigned char> buffer;
    const bool success = cv::imencode(".jpg", gray_frame, buffer, compression_params);
    if (!success) {
        return NULL;
    }

    unsigned char* result = static_cast<unsigned char*>(malloc(static_cast<size_t>(header_size) + buffer.size()));
    if (!result) {
        return NULL;
    }

    memcpy(result + header_size, buffer.data(), buffer.size());

    *size = static_cast<int>(buffer.size());

    return result;
}
