#ifndef CHESS_MOVE_DETECT_H
#define CHESS_MOVE_DETECT_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// Cấu trúc lưu thông tin một ô cờ
struct square_info {
    int row;
    int col;
    double diff_score;
    double avg_intensity_prev;
    double avg_intensity_curr;
};

// Hàm tính toán sự khác biệt giữa hai trạng thái bàn cờ
std::vector<square_info> calculate_square_differences(
    const cv::Mat& diff_image,
    const cv::Mat& prev_gray,
    const cv::Mat& curr_gray,
    int square_size
);

// Hàm phát hiện nước đi cờ vua dựa trên hai ảnh đầu vào
bool detect_chess_move(
    const std::string& prev_image_path,
    const std::string& curr_image_path,
    const std::string& output_image_path
);

#endif // CHESS_MOVE_DETECT_H
