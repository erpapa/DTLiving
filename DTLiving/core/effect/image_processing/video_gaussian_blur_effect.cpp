//
//  video_gaussian_blur_effect.cpp
//  DTLiving
//
//  Created by Dan Jiang on 2020/4/17.
//  Copyright © 2020 Dan Thought Studio. All rights reserved.
//

#include "constants.h"
#include "video_gaussian_blur_effect.h"
#include <cmath>
#include <sstream>

namespace dtliving {
namespace effect {
namespace image_processing {

VideoGaussianBlurEffect::VideoGaussianBlurEffect(const char *name, const char *vertex_shader_file, const char *fragment_shader_file,
                                                 const char *vertex_shader_file2, const char *fragment_shader_file2)
: VideoTwoPassTextureSamplingEffect(name, vertex_shader_file, fragment_shader_file,
                                    vertex_shader_file2, fragment_shader_file2) {
    auto gaussian_blur_vertex = VertexShaderOptimized(1.0, 1.0);
    auto gaussian_blur_fragment = FragmentShaderOptimized(1.0, 1.0);
}

void VideoGaussianBlurEffect::Init() {
    Init();
}

void VideoGaussianBlurEffect::BeforeDrawArrays() {
}

std::string VideoGaussianBlurEffect::VertexShaderOptimized(int blur_radius, float sigma) {
    // First, generate the normal Gaussian weights for a given sigma
    GLfloat *standard_gaussian_weights = new GLfloat[blur_radius + 1];
    GLfloat sum_of_weights = 0.0;
    for (int i = 0; i < blur_radius + 1; i++) {
        standard_gaussian_weights[i] = (1.0 / std::sqrt(2.0 * 3.141592 * std::pow(sigma, 2.0))) * std::exp(-std::pow(i, 2.0) / (2.0 * std::pow(sigma, 2.0)));
        if (i == 0) {
            sum_of_weights += standard_gaussian_weights[i];
        } else {
            sum_of_weights += 2.0 * standard_gaussian_weights[i];
        }
    }

    // Next, normalize these weights to prevent the clipping of the Gaussian curve at the end of the discrete samples from reducing luminance
    for (int i = 0; i < blur_radius + 1; i++) {
        standard_gaussian_weights[i] = standard_gaussian_weights[i] / sum_of_weights;
    }
    
    // From these weights we calculate the offsets to read interpolated values from
    int number_of_optimized_offsets = std::min(blur_radius / 2 + (blur_radius % 2), 7);
    GLfloat *optimized_gaussian_offsets = new GLfloat[number_of_optimized_offsets];
    for (int i = 0; i < number_of_optimized_offsets; i++) {
        GLfloat first_weight = standard_gaussian_weights[i * 2 + 1];
        GLfloat second_weight = standard_gaussian_weights[i * 2 + 2];
        GLfloat optimized_weight = first_weight + second_weight;
        optimized_gaussian_offsets[i] = (first_weight * (i * 2 + 1) + second_weight * (i * 2 + 2)) / optimized_weight;
    }

    std::stringstream os;
    os << "attribute vec4 a_position;\n";
    os << "attribute vec4 a_texcoord;\n";
    os << "\n";
    os << "uniform float u_texelWidthOffset;\n";
    os << "uniform float u_texelHeightOffset;\n";
    os << "\n";
    os << "varying vec2 v_blurCoordinates[" << (1 + number_of_optimized_offsets * 2) << "];\n";
    os << "\n";
    os << "void main() {\n";
    os << "gl_Position = a_position;\n";
    os << "\n";
    os << "vec2 singleStepOffset = vec2(u_texelWidthOffset, u_texelHeightOffset);\n";
    os << "v_blurCoordinates[0] = a_texcoord.xy;\n";
    for (int i = 0; i < number_of_optimized_offsets; i++) {
        os << "v_blurCoordinates[" << i * 2 + 1 << "] = a_texcoord.xy + singleStepOffset * " << optimized_gaussian_offsets[i] << ";\n";
        os << "v_blurCoordinates[" << i * 2 + 2 << "] = a_texcoord.xy - singleStepOffset * " << optimized_gaussian_offsets[i] << ";\n";
    }
    os << "}\n";

    return os.str();
}

std::string VideoGaussianBlurEffect::FragmentShaderOptimized(int blur_radius, float sigma) {
    // First, generate the normal Gaussian weights for a given sigma
    GLfloat *standard_gaussian_weights = new GLfloat[blur_radius + 1];
    GLfloat sum_of_weights = 0.0;
    for (int i = 0; i < blur_radius + 1; i++) {
        standard_gaussian_weights[i] = (1.0 / std::sqrt(2.0 * 3.141592 * std::pow(sigma, 2.0))) * std::exp(-std::pow(i, 2.0) / (2.0 * std::pow(sigma, 2.0)));
        if (i == 0) {
            sum_of_weights += standard_gaussian_weights[i];
        } else {
            sum_of_weights += 2.0 * standard_gaussian_weights[i];
        }
    }

    // Next, normalize these weights to prevent the clipping of the Gaussian curve at the end of the discrete samples from reducing luminance
    for (int i = 0; i < blur_radius + 1; i++) {
        standard_gaussian_weights[i] = standard_gaussian_weights[i] / sum_of_weights;
    }
    
    // From these weights we calculate the offsets to read interpolated values from
    int number_of_optimized_offsets = std::min(blur_radius / 2 + (blur_radius % 2), 7);
    int true_number_of_optimized_offsets = blur_radius / 2 + (blur_radius % 2);

    std::stringstream os;
    os << "varying vec2 v_blurCoordinates[" << (1 + number_of_optimized_offsets * 2) << "];\n";
    os << "\n";
    os << "uniform sampler2D u_texture;\n";
    os << "uniform highp float u_texelWidthOffset;\n";
    os << "uniform highp float u_texelHeightOffset;\n";
    os << "\n";
    os << "void main() {\n";
    os << "lowp vec4 sum = vec4(0.0);\n";
    os << "sum += texture2D(u_texture, v_blurCoordinates[0]) * " << standard_gaussian_weights[0] << ";\n";
    for (int i = 0; i < number_of_optimized_offsets; i++) {
        GLfloat first_weight = standard_gaussian_weights[i * 2 + 1];
        GLfloat second_weight = standard_gaussian_weights[i * 2 + 2];
        GLfloat optimized_weight = first_weight + second_weight;
        os << "sum += texture2D(u_texture, v_blurCoordinates[" << i * 2 + 1 << "]) * " << optimized_weight << ";\n";
        os << "sum += texture2D(u_texture, v_blurCoordinates[" << i * 2 + 2 << "]) * " << optimized_weight << ";\n";
    }
    
    if (true_number_of_optimized_offsets > number_of_optimized_offsets) {
        os << "highp vec2 singleStepOffset = vec2(u_texelWidthOffset, u_texelHeightOffset);\n";
    }
    for (int i = number_of_optimized_offsets; i < true_number_of_optimized_offsets; i++) {
        GLfloat first_weight = standard_gaussian_weights[i * 2 + 1];
        GLfloat second_weight = standard_gaussian_weights[i * 2 + 2];
        GLfloat optimized_weight = first_weight + second_weight;
        GLfloat optimized_offset = (first_weight * (i * 2 + 1) + second_weight * (i * 2 + 2)) / optimized_weight;
        os << "sum += texture2D(u_texture, v_blurCoordinates[0] + singleStepOffset * " << optimized_offset << ") * " << optimized_weight << ";\n";
        os << "sum += texture2D(u_texture, v_blurCoordinates[0] - singleStepOffset * " << optimized_offset << ") * " << optimized_weight << ";\n";
    }
    os << "gl_FragColor = sum;\n";
    os << "}\n";

    return os.str();
}

}
}
}