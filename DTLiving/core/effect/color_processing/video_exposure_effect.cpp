//
//  video_exposure_effect.cpp
//  DTLiving
//
//  Created by Dan Jiang on 2020/4/7.
//  Copyright © 2020 Dan Thought Studio. All rights reserved.
//

#include "constants.h"
#include "video_exposure_effect.h"

namespace dtliving {
namespace effect {
namespace color_processing {

VideoExposureEffect::VideoExposureEffect(std::string name)
: VideoEffect(name) {
}

void VideoExposureEffect::BeforeDrawArrays(GLsizei width, GLsizei height, int program_index) {
    GLint location = program_->UniformLocation(kVideoExposureEffectExposure);
    auto uniform = uniforms_[std::string(kVideoExposureEffectExposure)];
    glUniform1fv(location, 1, uniform.u_float.data());
}

}
}
}
