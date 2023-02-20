// @ts-nocheck

import {huahuoEngine} from "../EngineAPI";
import {AbstractComponent, Component} from "../Components/AbstractComponent";

// ParticleSystem is not compatible with any shape. It should be used in ParticleSystemRenderer only as a subcomponent.
@Component({compatibleShapes:[]})
class Particles extends AbstractComponent{
    _particleStatuses // 0 - inactive, 1 - active
    _particlePositions
    _particleVelocity

    maxNumbers = 10000 // Preload 10000 particles.

    constructor(rawObj?) {
        super(rawObj)

        this._particleVelocity = huahuoEngine.ti.Vector.field(3, ti.f32, [this.maxNumbers])
        this._particlePositions = huahuoEngine.ti.Vector.field(3, ti.f32, [this.maxNumbers])
        this._particleStatuses = huahuoEngine.ti.field(ti.f32, [this.maxNumbers])
    }
}

export {Particles}