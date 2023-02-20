// @ts-nocheck

import {huahuoEngine} from "../EngineAPI";
import {Component} from "../Components/AbstractComponent";

// ParticleSystem is not compatible with any shape. It should be used in ParticleSystemRenderer only as a subcomponent.
@Component({compatibleShapes:[]})
class Particles {
    _particleStatuses // 0 - inactive, 1 - active
    _particlePositions
    _particleVelocity

    maxNumbers = 10000 // Preload 10000 particles.

    constructor() {
        this._particleVelocity = huahuoEngine.ti.Vector.field(3, ti.f32, [this.maxNumbers])
        this._particlePositions = huahuoEngine.ti.Vector.field(3, ti.f32, [this.maxNumbers])
        this._particleStatuses = huahuoEngine.ti.field(ti.f32, [this.maxNumbers])
    }
}

export {Particles}