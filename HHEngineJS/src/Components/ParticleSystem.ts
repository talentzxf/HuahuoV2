// @ts-nocheck

import {huahuoEngine} from "../EngineAPI";

class ParticleSystem{
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

export {ParticleSystem}