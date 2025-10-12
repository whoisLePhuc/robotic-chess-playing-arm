#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;

// Struc to save chessboard square information
struct square_info{
    int row;
    int col;
    double diff_score;
    double avg_intensity_prev;
    double avg_intensity_curr;
};

// Function to calculate differences between two chessboard states
vector<square_info> calculate_square_differences(const Mat& diff_image, const Mat& prev_gray, const Mat& curr_gray, int square_size){
    vector<square_info> squares;
    for(int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++){
            // Calculate ROI for each square
            int x = col * square_size;
            int y = row * square_size;
            Rect roi(x, y, square_size, square_size);
            // Extract square region from diff image
            Mat square_diff = diff_image(roi);
            Mat square_prev = prev_gray(roi);
            Mat square_curr = curr_gray(roi);
            // Calculate average intensity in the square region
            double diff_score = (double)countNonZero(square_diff) / (square_size * square_size);
            // Calculate average intensity in previous and current frames
            Scalar avg_prev = mean(square_prev);
            Scalar avg_curr = mean(square_curr);
            // Save square information
            square_info info;
            info.row = row;
            info.col = col;
            info.diff_score = diff_score;
            info.avg_intensity_prev = avg_prev[0];
            info.avg_intensity_curr = avg_curr[0];
            squares.push_back(info);
        }
    return squares;
}

bool detect_chess_move(const string& prev_image_path, const string& curr_image_path, const string& output_image_path){
    // Read input images
    Mat prev_image = imread(prev_image_path);
    Mat curr_image = imread(curr_image_path);
    if (prev_image.empty() || curr_image.empty()){
        cerr << "Error: Could not open or find the images!" << endl;
        return false;
    }
    // Resize images to standard size (400x400)
    const int TARGET_SIZE = 400;
    Mat prev_resized, curr_resized;
    resize(prev_image, prev_resized, Size(TARGET_SIZE, TARGET_SIZE));
    resize(curr_image, curr_resized, Size(TARGET_SIZE, TARGET_SIZE));
    // Convert images to grayscale
    Mat prev_gray, curr_gray;
    cvtColor(prev_resized, prev_gray, COLOR_BGR2GRAY);
    cvtColor(curr_resized, curr_gray, COLOR_BGR2GRAY);
    // Apply Gaussian blur to reduce noise and improve difference detection
    GaussianBlur(prev_gray, prev_gray, Size(5, 5), 0);
    GaussianBlur(curr_gray, curr_gray, Size(5, 5), 0);
    // Compute absolute difference between the two images
    Mat diff_image;
    absdiff(prev_gray, curr_gray, diff_image);  
    // Threshold the difference image to get binary image
    Mat thresh_image;
    threshold(diff_image, thresh_image, 25, 255, THRESH_BINARY);
    // Calculate square differences
    const int SQUARE_SIZE = TARGET_SIZE / 8;
    vector<square_info> squares = calculate_square_differences(thresh_image, prev_gray, curr_gray, SQUARE_SIZE);
    // Sort squares by difference score in descending order
    sort(squares.begin(), squares.end(), [](const square_info& a, const square_info& b) {
        return a.diff_score > b.diff_score;
    });
    // Take top 2 squares as moved squares
    if (squares.size() < 2 || squares[0].diff_score < 0.1) {
        cerr << "Warning: Can not detect move" << endl;
        return false;
    }
    square_info square1 = squares[0];
    square_info square2 = squares[1];
    // Determine which square is the source and which is the destination
    square_info from_square, to_square;
    double intensity_diff1 = square1.avg_intensity_curr - square1.avg_intensity_prev;
    double intensity_diff2 = square2.avg_intensity_curr - square2.avg_intensity_prev;
    if(intensity_diff1 < intensity_diff2){
        from_square = square1; // Tối đi -> From
        to_square = square2; // Sáng lên -> To
    } else {
        from_square = square2;
        to_square = square1;
    }   
    // Draw result on current image
    Mat output_image = curr_resized.clone();
    // Calculate center points of the squares
    Point from_center(from_square.col * SQUARE_SIZE + SQUARE_SIZE / 2, from_square.row * SQUARE_SIZE + SQUARE_SIZE / 2);
    Point to_center(to_square.col * SQUARE_SIZE + SQUARE_SIZE / 2, to_square.row * SQUARE_SIZE + SQUARE_SIZE / 2);
    // Assign 1 for FROM square and 2 for TO square
    putText(output_image, "1", from_center, FONT_HERSHEY_BOLD, 2.0, Scalar(0, 0, 255), 4);
    putText(output_image, "2", to_center, FONT_HERSHEY_BOLD, 2.0, Scalar(0, 255, 0), 4); 
    // Draw rectangles around the squares
    rectangle(output_image, Rect(from_square.col * SQUARE_SIZE, from_square.row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE), Scalar(0, 0, 255), 2);
    rectangle(output_image, Rect(to_square.col * SQUARE_SIZE, to_square.row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE), Scalar(0, 255, 0), 2);
    // Save output image
    bool success = imwrite(output_image_path, output_image);
    if (success) {
        cout << "Phát hiện nước đi thành công!" << endl;
        cout << "FROM: Hàng " << from_square.row << ", Cột " << from_square.col << endl;
        cout << "TO: Hàng " << to_square.row << ", Cột " << to_square.col << endl;
        cout << "Ảnh kết quả đã lưu tại: " << output_image_path << endl;
    }
    
    return success;
}