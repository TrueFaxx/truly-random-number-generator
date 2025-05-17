#include <opencv2/opencv.hpp>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>

int main() {
    cv::VideoCapture cap(0, cv::CAP_DSHOW);
    if (!cap.isOpened()) {
        std::cerr << "webcam initialization failure\n";
        return -1;
    }

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "empty frame\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }
        std::vector<uchar> buffer;
        cv::imencode(".bmp", frame, buffer);
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(buffer.data(), buffer.size(), hash);
        BIGNUM* bn = BN_bin2bn(hash, SHA256_DIGEST_LENGTH, nullptr);
        if (!bn) {
            std::cerr << "failed\n";
            break;
        }
        char* dec_str = BN_bn2dec(bn);
        if (!dec_str) {
            std::cerr << "failed\n";
            BN_free(bn);
            break;
        }
        std::cout << "\nhash:\n" << dec_str << "\n";

        unsigned char bn_bin[SHA256_DIGEST_LENGTH];
        int bn_len = BN_bn2bin(bn, bn_bin);
        uint64_t seed = 0;
        int offset = (bn_len > 8 ? bn_len - 8 : 0);
        for (int i = offset; i < bn_len; ++i) {
            seed = (seed << 8) | bn_bin[i];
        }
        std::cout << "seed:" << seed << "\n";
        std::mt19937_64 rng(seed);
        std::cout << "Random number:" << rng() << "\n";
        OPENSSL_free(dec_str);
        BN_free(bn);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
