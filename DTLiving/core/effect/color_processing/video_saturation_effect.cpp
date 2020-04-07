//
//  video_saturation_effect.cpp
//  DTLiving
//
//  Created by Dan Jiang on 2020/4/7.
//  Copyright © 2020 Dan Thought Studio. All rights reserved.
//

#include "constants.h"
#include "video_saturation_effect.h"

namespace dtliving {
namespace effect {
namespace color_processing {

VideoSaturationEffect::VideoSaturationEffect(const char *name, const char *vertex_shader_file, const char *fragment_shader_file)
: VideoEffect(name, vertex_shader_file, fragment_shader_file) {
}

void VideoSaturationEffect::BeforeDrawArrays() {
    GLint location = program_->UniformLocation(kVideoSaturationEffectSaturation);
    auto saturation = uniforms_[std::string(kVideoSaturationEffectSaturation)];
    glUniform1f(location, saturation.u_float);
}

}
}
}