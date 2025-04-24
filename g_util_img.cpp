/**
 * Copyright © 2025-2025 Gking,All Rights Reserved.
 * https://github.com/Gking-a/Gking_Milkvduo_Tools
 * Mozilla Public License Version 2.0
 */
#include "g_u_h.h"

ImageDetector::ImageDetector(){};
ImageDetector::~ImageDetector(){};
ImageDetectInfo* detectBinaryMiddle(Mat &img,void**) {
    Mat binary;
    threshold(img, binary, 55, 255, THRESH_BINARY);

    ImageDetectInfo* result = new ImageDetectInfo();
    result->detected = false;

    const int height = binary.rows;

    // 配置参数
    const int min_road_width = 15;    // 最小有效道路宽度
    const int min_valid_lines = 1;   // 最少有效扫描线数量
    //const int noise_max_width = 10;  // 最大噪声宽度阈值
    const int max_merge_gap = 8;
    std::vector<int> valid_x;

    // 生成扫描行（同原逻辑）
    const int center_y = height / 2;
    std::vector<int> scan_rows{center_y};
    for (int i = 1; i <= 15; ++i) {
        scan_rows.push_back(center_y - i);
        // scan_rows.push_back(center_y + 2*i);
    }

    // 逐行处理
    for (const int y : scan_rows) {
        if (y < 0 || y >= height) continue;

        const uchar* row = binary.ptr<uchar>(y);
        const int width = binary.cols;
        
        // 连续黑色区域检测
        std::vector<std::pair<int, int>> black_zones;  // 存储<起始x, 结束x>
        bool in_zone = false;
        int start_x = -1;

        // 全行扫描
        for (int x = 0; x < width; ++x) {
            if (row[x] == 0) {  // 黑色像素
                if (!in_zone) {
                    start_x = x;    // 区域开始
                    in_zone = true;
                }
            } else {
                if (in_zone) {
                    black_zones.emplace_back(start_x, x-1); // 记录区域
                    in_zone = false;
                }
            }
        }
        
        // 处理行尾区域
        if (in_zone) {
            black_zones.emplace_back(start_x, width-1);
        }
        if (!black_zones.empty()) {
            std::vector<std::pair<int, int>> merged;
            auto current = black_zones.front();
            
            for (size_t i = 1; i < black_zones.size(); ++i) {
                const auto& next = black_zones[i];
                
                // 间隙计算：next.start - current.end
                if (next.first - current.second <= max_merge_gap + 1) {
                    current.second = next.second;  // 合并区域
                } else {
                    merged.push_back(current);
                    current = next;
                }
            }
            merged.push_back(current);
            black_zones = merged;  // 更新为合并后区域
        }
        // 寻找最长有效区域
        int max_length = 0;
        std::pair<int, int> best_zone(-1, -1);

        for (const auto& zone : black_zones) {
            const int zone_length = zone.second - zone.first + 1;
            
            // 验证：长度足够
            if (zone_length >= min_road_width) {
                if (zone_length > max_length) {
                    max_length = zone_length;
                    best_zone = zone;
                }
            }
        }

        // 计算中线坐标
        if (best_zone.first != -1) {
            const double midline = (best_zone.first + best_zone.second) / 2.0;
            valid_x.push_back(midline);
        }
    }

    // 最终判定
    if (valid_x.size() >= min_valid_lines) {
        result->detected = true;
        result->referoffsetx = accumulate(valid_x.begin(), valid_x.end(), 0) / valid_x.size()-
                              (binary.cols / 2.0);
    } else {
        result->referoffsetx = binary.cols / 2.0;
    }

    return result;
}
/**
 * @param img HSV
 */
ImageDetectInfo* detectFollow(cv::Mat& hsv, void ** a) {
    detectFollowStruct* arg=(detectFollowStruct*)*a;
    ImageDetectInfo* result = new ImageDetectInfo();
    // 颜色阈值处理
    cv::Mat mask;
    cv::inRange(hsv,            // 输入hsv图像
        arg->lower,      // 阈值下限 (H,S,V)
        arg->upper,      // 阈值上限 (H,S,V)
        mask);          // 输出二值掩码
    // 形态学优化
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, { 3,3 }));
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (!contours.empty()) {
        auto max_contour = *std::max_element(contours.begin(), contours.end(),
            [](const auto& a, const auto& b) { return cv::contourArea(a) < cv::contourArea(b); });
        cv::Moments m = cv::moments(max_contour);
        if (m.m00 == 0) {
            result->detected = false;
            result->referoffsetx = 0;
            result->referoffsety = 0;
        }
        else {
            result->referoffsetx = static_cast<int>(m.m10 / m.m00-hsv.cols/2);
            result->referoffsety = static_cast<int>(-m.m01 / m.m00+hsv.rows/2);
            result->detected = true;
        }
    }
    return result;
}